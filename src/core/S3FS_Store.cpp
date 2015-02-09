#include "S3FS_Store.hpp"
#include "S3FS_Obj.hpp"
#include "S3FS_Aws_S3.hpp"
#include "S3FS_Aws_SQS.hpp"
#include "S3FS_Store_MetaIterator.hpp"
#include "Callback.hpp"
#include <QDir>
#include <QUuid>
#include <QDataStream>

#define INT_TO_BYTES(_x) QByteArray _x ## _b; { QDataStream s_tmp(&_x ## _b, QIODevice::WriteOnly); s_tmp << _x; }

S3FS_Store::S3FS_Store(const QByteArray &_bucket, const QByteArray &queue, QObject *parent): QObject(parent) {
	bucket = _bucket;
	aws_list_ready = false;
	aws_format_ready = false;
	last_inode_rev = 0;
	file_match = QRegExp("metadata/[0-9a-f]/[0-9a-f]{2}/([0-9a-f]{16})/([0-9a-f]{16})\\.dat");
	algo = QCryptographicHash::Sha3_256; // default value
	// generate filename
	kv_location = QDir::temp().filePath(QString("s3clfs-")+QUuid::createUuid().toRfc4122().toHex());
	qDebug("S3FS: Keyval location: %s", qPrintable(kv_location));

	if (!kv.create(kv_location)) {
		qFatal("S3FS_Store: Failed to open cache");
	}

	// initialize AWS
	aws = new S3FS_Aws(this);

	if (!aws->isValid()) {
		QTimer::singleShot(1000, this, SLOT(readyStateWithoutAws()));
		return;
	}

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

	cache_updater.start(300000); // 5min
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
	auto i = getInodeMetaIterator(ino);
	if (!i->isValid()) return;
	do {
		kv.remove(i->fullKey());
	} while(i->next());
	delete i;
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

S3FS_Store::~S3FS_Store() {
	if (kv.isValid()) {
		kv.close();
		Keyval::destroy(kv_location);
	}
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
	if (inodes_to_update_1.contains(ino))
		inodes_to_update_1.remove(ino);
	inodes_to_update_2.insert(ino);
}

void S3FS_Store::updateInodes() {
	quint64 ino;
	foreach(ino, inodes_to_update_1)
		sendInodeToAws(ino);
	
	inodes_to_update_1 = inodes_to_update_2;
	inodes_to_update_2.clear();
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
	QByteArray data = r->body();
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
	return kv.value(QByteArrayLiteral("\x02")+hash);
}

bool S3FS_Store::hasBlockLocally(const QByteArray &hash) {
	return kv.contains(QByteArrayLiteral("\x02")+hash);
}

void S3FS_Store::callbackOnBlockCached(const QByteArray &block, Callback *cb) {
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

	// call callbacks
	QList<Callback*> list = block_download_callback.take(block);
	Callback *cb;
	foreach(cb, list)
		cb->trigger();
}


bool S3FS_Store::hasInodeMeta(quint64 ino, const QByteArray &key_sub) {
	INT_TO_BYTES(ino);
	QByteArray key = QByteArrayLiteral("\x01") + ino_b;
	return kv.contains(key+key_sub);
}

QByteArray S3FS_Store::getInodeMeta(quint64 ino, const QByteArray &key_sub) {
	INT_TO_BYTES(ino);
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
	quint64 new_inode_rev = QDateTime::currentMSecsSinceEpoch()*1000;

	if (new_inode_rev <= last_inode_rev) { // ensure we are incremental
		new_inode_rev = last_inode_rev+100;
	}
	last_inode_rev = new_inode_rev;
	return new_inode_rev;
}

