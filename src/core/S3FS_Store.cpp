/*  S3ClFS - AWS S3 backed cluster filesystem
 *  Copyright (C) 2015 Mark Karpeles
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "S3FS_Store.hpp"
#include "S3FS_Obj.hpp"
#include "S3FS_Config.hpp"
#include "S3FS_Aws_S3.hpp"
#include "S3FS_Aws_SQS.hpp"
#include "S3FS_Store_MetaIterator.hpp"
#include "Callback.hpp"
#include <QDir>
#include <QUuid>
#include <QDataStream>


#define INT_TO_BYTES(_x) QByteArray _x ## _b; { QDataStream s_tmp(&_x ## _b, QIODevice::WriteOnly); s_tmp << _x; }

S3FS_Store::S3FS_Store(S3FS_Config *_cfg, QObject *parent): QObject(parent) {
	cfg = _cfg;
	bucket = cfg->bucket();
	aws_list_ready = false;
	aws_format_ready = false;
	last_inode_rev = 0;
	file_match = QRegExp("metadata/[0-9a-f]/[0-9a-f]{2}/([0-9a-f]{16})/([0-9a-f]{16})\\.dat");
	algo = QCryptographicHash::Sha3_256; // default value
	cluster_node_id = cfg->clusterId();

	// location of leveldb store
	QString cache_path = cfg->cachePath();
	if (cache_path.isEmpty()) {
		kv_location = QDir::temp().filePath(QString("s3clfs-")+bucket);
	} else {
		kv_location = cache_path;
	}
	qDebug("S3FS: Keyval location: %s", qPrintable(kv_location));

	if (!kv.open(kv_location)) {
		qFatal("S3FS_Store: Failed to open cache");
	}

	// initialize AWS
	aws = new S3FS_Aws(this);

	if (!aws->isValid()) {
		QTimer::singleShot(1000, this, SLOT(readyStateWithoutAws()));
		return;
	}

	QByteArray queue = cfg->queue();
	if (!queue.isEmpty()) {
		aws_sqs = new S3FS_Aws_SQS(queue, aws);
		connect(aws_sqs, SIGNAL(newFile(const QString&,const QString&)), this, SLOT(gotNewFile(const QString&,const QString&)));
	}

	// test
	connect(S3FS_Aws_S3::listFiles(bucket, "metadata/", aws), SIGNAL(finished(S3FS_Aws_S3*)), this, SLOT(receivedInodeList(S3FS_Aws_S3*)));
	connect(S3FS_Aws_S3::getFile(bucket, "metadata/format.dat", aws), SIGNAL(finished(S3FS_Aws_S3*)), this, SLOT(receivedFormatFile(S3FS_Aws_S3*)));

	connect(&inodes_updater, SIGNAL(timeout()), this, SLOT(updateInodes()));
	inodes_updater.setSingleShot(false);
	inodes_updater.start(1000);

	connect(&cache_updater, SIGNAL(timeout()), this, SLOT(getInodesList()));
	cache_updater.setSingleShot(true);

	connect(&lastaccess_updater, SIGNAL(timeout()), this, SLOT(lastaccess_update()));
	lastaccess_updater.setSingleShot(false);
	lastaccess_updater.start(5000);

	connect(&lastaccess_cleaner, SIGNAL(timeout()), this, SLOT(lastaccess_clean()));
	lastaccess_cleaner.setSingleShot(false);
	lastaccess_cleaner.start(1800000); // 30min
}

void S3FS_Store::gotNewFile(const QString &_bucket, const QString &file) {
	if (bucket != _bucket) return;
//	qDebug("GOT NEW FILES %s", qPrintable(file));
	learnFile(file, false);
}

void S3FS_Store::getInodesList() {
	connect(S3FS_Aws_S3::listFiles(bucket, "metadata/", aws), SIGNAL(finished(S3FS_Aws_S3*)), this, SLOT(receivedInodeList(S3FS_Aws_S3*)));
}

void S3FS_Store::receivedInodeList(S3FS_Aws_S3 *r) {
	bool need_more;
	QStringList list = r->parseListFiles(need_more);
//	qDebug("S3FS_Store: scanning inodes, got %d entries", list.size());
	QString name;
	foreach(name, list) {
		learnFile(name, true);
	}
	if (need_more) {
		connect(r->listMoreFiles("metadata/", list), SIGNAL(finished(S3FS_Aws_S3*)), this, SLOT(receivedInodeList(S3FS_Aws_S3*)));
		return;
	}

	cache_updater.start(900000); // 15min
	if (aws_list_ready) return;
	aws_list_ready = true;
	if (aws_format_ready && aws_list_ready) ready();
}

void S3FS_Store::learnFile(const QString &name, bool in_list) {
	// metadata/1/01/0000000000000001/00050e9d900df750.dat
	// metadata/0/70/00050e9d94651c70/00050e9d947721b8.dat
	if (!file_match.exactMatch(name)) {
		if (name == QStringLiteral("metadata/format.dat")) return; // do not delete that file
		if (name.left(9) != QStringLiteral("metadata/")) return; // avoid deleting stuff outside of metadata
		qDebug("S3FS_Store: deleting unknown file %s found on S3", qPrintable(name));
		S3FS_Aws_S3::deleteFile(bucket, name.toUtf8(), aws);
		return;
	}
	QByteArray fn = QByteArray::fromHex(file_match.cap(1).toLatin1());
	QByteArray newrev = QByteArray::fromHex(file_match.cap(2).toLatin1());

	if (kv.contains(QByteArrayLiteral("\x03")+fn)) {
		// remove old file
		QByteArray rev = kv.value(QByteArrayLiteral("\x03")+fn);
		if (rev == newrev) return; // no change
		if (rev < newrev) {
			// our version is older, update our value and do not delete from S3
			qDebug("S3FS_Store: inode %s update - our version %s, s3 has %s", fn.toHex().data(), rev.toHex().data(), newrev.toHex().data());
			kv.insert(QByteArrayLiteral("\x03")+fn, newrev);
			if (kv.contains(QByteArrayLiteral("\x01")+fn)) {
				// clear this inode from cache
				qDebug("S3FS_Store: Inode %s has changed, invalidating cache", fn.toHex().data());
				auto i = new KeyvalIterator(&kv);
				i->find(QByteArrayLiteral("\x01")+fn);
				do {
					if (i->key().left(fn.length()+1) == QByteArrayLiteral("\x01")+fn) {
						kv.remove(i->key());
					} else {
						break;
					}
				} while(i->next());
				delete i;
			}
			return;
		}
		// our version is newer, delete old stuff
		if (in_list) {
			QByteArray fn_hex = fn.toHex();
			QByteArray rev_hex = rev.toHex();
			QByteArray old_path = "metadata/"+fn_hex.right(1)+"/"+fn_hex.right(2)+"/"+fn_hex+"/"+newrev.toHex()+".dat";
			S3FS_Aws_S3::deleteFile(bucket, old_path, aws);
		}
		return;
	}
	kv.insert(QByteArrayLiteral("\x03")+fn, newrev);
}

void S3FS_Store::removeInodeFromCache(quint64 ino) {
	INT_TO_BYTES(ino);
	auto i = getInodeMetaIterator(ino);

	// lastaccess
	lastaccess_inode.remove(ino);
	kv.remove(QByteArrayLiteral("\x11")+ino_b);

	if (!i->isValid()) return;
	do {
		kv.remove(i->fullKey());
	} while(i->next());
	delete i;
}

void S3FS_Store::brokenInode(quint64 ino) {
	INT_TO_BYTES(ino);
	// that inode is borked, for now let's just forget about it
	removeInodeFromCache(ino);
	kv.remove(QByteArrayLiteral("\x03")+ino_b); // next call checking if inode exists will fail
}

void S3FS_Store::receivedFormatFile(S3FS_Aws_S3 *r) {
	QVariant c;
	QDataStream kv_val(r->body()); kv_val >> c;
	if ((!c.isValid()) || (c.type() != QVariant::Map)) {
		aws_format_ready = true;
		if (aws_format_ready && aws_list_ready) ready();
		return;
	}
	config = c.toMap();

	qDebug("S3FS_Store: got config from AWS");
	aws_format_ready = true;
	if (aws_format_ready && aws_list_ready) ready();
}

const QVariantMap &S3FS_Store::getConfig() {
	return config;
}

bool S3FS_Store::readConfig() {
	QVariant c;
	QDataStream kv_val(kv.value(QByteArrayLiteral("\x01"))); kv_val >> c;
	if (!c.isValid()) return false;
	if (c.type() != QVariant::Map) return false;
	config = c.toMap();
	return true;
}

bool S3FS_Store::setConfig(const QVariantMap&c) {
	QByteArray buf;
	QDataStream buf_stream(&buf, QIODevice::WriteOnly); buf_stream << (QVariant)c;
	if (!kv.insert(QByteArrayLiteral("\x01"), buf)) {
		return false;
	}
	S3FS_Aws_S3::putFile(bucket, "metadata/format.dat", buf, aws);
	config = c;
	return true;
}

void S3FS_Store::readyStateWithoutAws() {
	qDebug("S3FS_Store: Going ready without any actual backend storage!");
	ready();
}

bool S3FS_Store::hasInode(quint64 ino) {
	// transform quint64 to qbytearray (big endian)
	INT_TO_BYTES(ino);

	QByteArray key = QByteArrayLiteral("\x03") + ino_b; // where we should be storing cache info about this inode
	return kv.contains(key);
}

bool S3FS_Store::storeInode(const S3FS_Obj&o) {
	quint64 ino = o.getInode();
	INT_TO_BYTES(ino);

	QByteArray key = QByteArrayLiteral("\x01") + ino_b;
	if (!kv.insert(key, o.encode())) return false;
	kv.insert(QByteArrayLiteral("\x03") + ino_b, QByteArray(8, '\0')); // default to zero

	// send inode to aws
	inodeUpdated(ino);

	return true;
}

void S3FS_Store::inodeUpdated(quint64 ino) {
	if (!inodes_to_update.contains(ino))
		inodes_to_update.insert(ino);
	lastaccess_inode.insert(ino);
}

void S3FS_Store::updateInodes() {
	foreach(quint64 ino, inodes_to_update)
		sendInodeToAws(ino);
	
	inodes_to_update.clear();
}

void S3FS_Store::sendInodeToAws(quint64 ino) {
	INT_TO_BYTES(ino);
	// metadata/z/yz/xyz.dat

	QByteArray data;
	QDataStream data_stream(&data, QIODevice::WriteOnly);
	auto i = getInodeMetaIterator(ino);
	int count = 0;
	do {
		data_stream << i->key();
		data_stream << i->value();
		count++;
	} while(i->next());
	delete i;
	if (count == 0) data.clear();

	quint64 ino_rev = makeInodeRev();
	INT_TO_BYTES(ino_rev);

	QByteArray ino_hex = ino_b.toHex();
	S3FS_Aws_S3::putFile(bucket, "metadata/"+ino_hex.right(1)+"/"+ino_hex.right(2)+"/"+ino_hex+"/"+ino_rev_b.toHex()+".dat", data, aws);
	kv.insert(QByteArrayLiteral("\x03")+ino_b, ino_rev_b);
}

S3FS_Obj S3FS_Store::getInode(quint64 ino) {
	INT_TO_BYTES(ino);
	QByteArray key = QByteArrayLiteral("\x01") + ino_b;
	lastaccess_inode.insert(ino);

	return S3FS_Obj(kv.value(key));
}

bool S3FS_Store::hasInodeLocally(quint64 ino) {
	INT_TO_BYTES(ino);
	QByteArray key = QByteArrayLiteral("\x01") + ino_b;
	return kv.contains(key);
//	return (kv.value(key).length() > 0); // if length == 0, means we don't have this locally
}

void S3FS_Store::callbackOnInodeCached(quint64 ino, Callback *cb) {
	// we need to try to get that inode
	if (inode_download_callback.contains(ino)) {
		inode_download_callback[ino].append(cb);
		return;
	}

	// create wait queue
	inode_download_callback.insert(ino, QList<Callback*>() << cb);

	INT_TO_BYTES(ino);

	QByteArray ino_rev = kv.value(QByteArrayLiteral("\x03")+ino_b);
	if (ino_rev.isEmpty()) {
		qFatal("Could not fetch inode!");
	}

	// send request
	QByteArray ino_hex = ino_b.toHex();
	QByteArray ino_rev_hex = ino_rev.toHex();
	QByteArray path = QByteArrayLiteral("metadata/")+ino_hex.right(1)+QByteArrayLiteral("/")+ino_hex.right(2)+QByteArrayLiteral("/")+ino_hex+QByteArrayLiteral("/")+ino_rev_hex+QByteArrayLiteral(".dat");
	S3FS_Aws_S3 *req = S3FS_Aws_S3::getFile(bucket, path, aws);
	if (!req) {
		qFatal("Could not make request to fetch inode");
	}
	req->setProperty("_inode_num", ino);
	connect(req, SIGNAL(finished(S3FS_Aws_S3*)), this, SLOT(receivedInode(S3FS_Aws_S3*)));
}

void S3FS_Store::receivedInode(S3FS_Aws_S3*r) {
	quint64 ino = r->property("_inode_num").toULongLong();
	INT_TO_BYTES(ino);
	lastaccess_inode.insert(ino);
	QByteArray data = r->body();
	if (data.isEmpty()) {
		qDebug("Inode %llu missing!", ino);
		kv.remove(QByteArrayLiteral("\x03")+ino_b); // remove this inode from known inodes
		// call callbacks
		QList<Callback*> list = inode_download_callback.take(ino);
		foreach(auto cb, list)
			cb->trigger();
		return;
	}
	QDataStream s(data);
	qDebug("Received inode %llu", ino);

	while(!s.atEnd()) {
		QByteArray key, val;
		s >> key >> val;
		kv.insert(QByteArrayLiteral("\x01")+ino_b+key, val);
	}

	// call callbacks
	QList<Callback*> list = inode_download_callback.take(ino);
	Callback *cb;
	foreach(cb, list)
		cb->trigger();
}

QByteArray S3FS_Store::writeBlock(const QByteArray &buf) {
	if (buf.isEmpty()) return QByteArray();

	// compute hash
	QByteArray hash = QCryptographicHash::hash(buf, algo);
	lastaccess_data.insert(hash);

	if (hasBlockLocally(hash)) return hash;

	if (!kv.insert(QByteArrayLiteral("\x02")+hash, buf))
		return QByteArray();

	// storage
	QByteArray hash_hex = hash.toHex();
	QByteArray path = QByteArrayLiteral("data/")+hash_hex.right(1)+"/"+hash_hex.right(2)+"/"+hash_hex+".dat";

	S3FS_Aws_S3::putFile(bucket, path, buf, aws);

	return hash;
}

QByteArray S3FS_Store::readBlock(const QByteArray &hash) {
	lastaccess_data.insert(hash);
	return kv.value(QByteArrayLiteral("\x02")+hash);
}

bool S3FS_Store::hasBlockLocally(const QByteArray &hash) {
	return kv.contains(QByteArrayLiteral("\x02")+hash);
}

void S3FS_Store::callbackOnBlockCached(const QByteArray &block, Callback *cb) {
	lastaccess_data.insert(block);
	// we need to try to get that block
	if (block_download_callback.contains(block)) {
		block_download_callback[block].append(cb);
		return;
	}

	// create wait queue
	block_download_callback.insert(block, QList<Callback*>() << cb);

	// send request
	QByteArray block_hex = block.toHex();
	QByteArray path = QByteArrayLiteral("data/")+block_hex.right(1)+QByteArrayLiteral("/")+block_hex.right(2)+QByteArrayLiteral("/")+block_hex+QByteArrayLiteral(".dat");
	S3FS_Aws_S3 *req = S3FS_Aws_S3::getFile(bucket, path, aws);
	if (!req) {
		qFatal("Could not make request to fetch block");
	}
	req->setProperty("_block_id", block);
	connect(req, SIGNAL(finished(S3FS_Aws_S3*)), this, SLOT(receivedBlock(S3FS_Aws_S3*)));
}

void S3FS_Store::receivedBlock(S3FS_Aws_S3*r) {
	QByteArray block = r->property("_block_id").toByteArray();
	QByteArray data = r->body();
	kv.insert(QByteArrayLiteral("\x02")+block, data);
	lastaccess_data.insert(block);

	// call callbacks
	QList<Callback*> list = block_download_callback.take(block);
	Callback *cb;
	foreach(cb, list)
		cb->trigger();
}


bool S3FS_Store::hasInodeMeta(quint64 ino, const QByteArray &key_sub) {
	INT_TO_BYTES(ino);
	lastaccess_inode.insert(ino);
	QByteArray key = QByteArrayLiteral("\x01") + ino_b;
	return kv.contains(key+key_sub);
}

QByteArray S3FS_Store::getInodeMeta(quint64 ino, const QByteArray &key_sub) {
	INT_TO_BYTES(ino);
	lastaccess_inode.insert(ino);
	QByteArray key = QByteArrayLiteral("\x01") + ino_b;
	return kv.value(key+key_sub);
}

bool S3FS_Store::setInodeMeta(quint64 ino, const QByteArray &key_sub, const QByteArray &value) {
	INT_TO_BYTES(ino);
	QByteArray key = QByteArrayLiteral("\x01") + ino_b;
	if (!kv.insert(key+key_sub, value)) return false;
	inodeUpdated(ino);
	return true;
}

S3FS_Store_MetaIterator *S3FS_Store::getInodeMetaIterator(quint64 ino) {
	INT_TO_BYTES(ino);
	lastaccess_inode.insert(ino);
	QByteArray key = QByteArrayLiteral("\x01") + ino_b;
	return new S3FS_Store_MetaIterator(&kv, key);
}

bool S3FS_Store::removeInodeMeta(quint64 ino, const QByteArray &key_sub) {
	INT_TO_BYTES(ino);
	QByteArray key = QByteArrayLiteral("\x01") + ino_b;
	if (!kv.remove(key+key_sub)) return false;
	inodeUpdated(ino);
	return true;
}

bool S3FS_Store::clearInodeMeta(quint64 ino) {
	auto i = getInodeMetaIterator(ino);
	if (!i->isValid()) {
		delete i;
		return false;
	}
	do {
		if (i->key() == "") continue;
		if (!removeInodeMeta(ino, i->key())) {
			delete i;
			return false;
		}
	} while(i->next());
	delete i;
	inodeUpdated(ino);
	return true;
}

quint64 S3FS_Store::makeInodeRev() {
	quint64 new_inode_rev = QDateTime::currentMSecsSinceEpoch()*1000 + cluster_node_id;

	if (new_inode_rev <= last_inode_rev) { // ensure we are incremental
		new_inode_rev = last_inode_rev+100;
	}
	last_inode_rev = new_inode_rev;
	return new_inode_rev;
}

void S3FS_Store::lastaccess_update() {
	quint64 t = QDateTime::currentMSecsSinceEpoch();
	INT_TO_BYTES(t);
	foreach(auto ino, lastaccess_inode) {
		INT_TO_BYTES(ino);
		kv.insert(QByteArrayLiteral("\x11")+ino_b, t_b);
	}
	foreach(auto block, lastaccess_data) {
		kv.insert(QByteArrayLiteral("\x12")+block, t_b);
	}
	lastaccess_inode.clear();
	lastaccess_data.clear();
}

void S3FS_Store::lastaccess_clean() {
	lastaccess_update(); // start by making sure we have latest data
	// We want to remove any block of data that hasn't been used for 1 hour, or any inode unused for 24 hours
	auto i = new KeyvalIterator(&kv);
	auto j = new KeyvalIterator(&kv);
	i->find(QByteArrayLiteral("\x11")); // inodes

	quint64 timeout_inodes = QDateTime::currentMSecsSinceEpoch() - 86400000; // 24h
	quint64 timeout_blocks = QDateTime::currentMSecsSinceEpoch() - 3600000; // 1h
	INT_TO_BYTES(timeout_inodes);
	INT_TO_BYTES(timeout_blocks);

	while((i->isValid()) && (i->key().at(0) == '\x11')) {
		if (i->value() < timeout_inodes_b) {
			qDebug("S3FS_Store: inode %s not accessed for 24 hours, removing from cache", i->key().mid(1).toHex().data());
			QByteArray prefix = QByteArrayLiteral("\x01")+i->key().mid(1);
			j->find(prefix);
			while((j->isValid()) && (j->key().left(prefix.length()) == prefix)) {
				kv.remove(j->key());
				if (!j->next()) break;
			}
			kv.remove(i->key());
		}
		if (!i->next()) break;
	}
	delete j; // only for inodes
	while((i->isValid()) && (i->key().at(0) == '\x12')) {
		if (i->value() < timeout_blocks_b) {
			qDebug("S3FS_Store: block %s not accessed for 1 hour, removing from cache", i->key().mid(1).toHex().data());
			kv.remove(QByteArrayLiteral("\x02")+i->key().mid(1));
			kv.remove(i->key());
		}
		if (!i->next()) break;
	}
	delete i;
}

