#include "S3FS_Store.hpp"
#include "S3FS_Obj.hpp"
#include "S3FS_Aws_S3.hpp"
#include "S3FS_Store_MetaIterator.hpp"
#include <QDir>
#include <QUuid>
#include <QTimer>
#include <QDataStream>

#define INT_TO_BYTES(_x) QByteArray _x ## _b; { QDataStream s_tmp(&_x ## _b, QIODevice::WriteOnly); s_tmp << _x; }

S3FS_Store::S3FS_Store(const QByteArray &_bucket, QObject *parent): QObject(parent) {
	bucket = _bucket;
	algo = QCryptographicHash::Sha3_256; // default value
	// generate filename
	kv_location = QDir::temp().filePath(QString("s3clfs-")+QUuid::createUuid().toRfc4122().toHex());
	qDebug("S3FS: Keyval location: %s", qPrintable(kv_location));

	// initialize AWS substore
	s3 = new S3FS_Aws_S3(bucket, this);

	QTimer::singleShot(1000, this, SLOT(test_setready()));

	if (!kv.create(kv_location)) {
		qFatal("S3FS_Store: Failed to open cache");
	}
}

const QVariantMap &S3FS_Store::getConfig() {
	return config;
}

bool S3FS_Store::readConfig() {
	QVariant c;
	QDataStream kv_val(kv.value(QByteArray("\x01", 1))); kv_val >> c;
	if (!c.isValid()) return false;
	if (c.type() != QVariant::Map) return false;
	config = c.toMap();
	return true;
}

bool S3FS_Store::setConfig(const QVariantMap&c) {
	QByteArray buf;
	QDataStream buf_stream(&buf, QIODevice::WriteOnly); buf_stream << (QVariant)c;
	if (!kv.insert(QByteArray("\x01", 1), buf)) {
		return false;
	}
	config = c;
	return true;
}

void S3FS_Store::test_setready() {
	qDebug("S3FS_Store: Ready!");
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

	QByteArray key = QByteArray("\x01", 1) + ino_b; // where we should be storing cache info about this inode
	return kv.contains(key);
}

bool S3FS_Store::storeInode(const S3FS_Obj&o) {
	quint64 ino = o.getInode();
	INT_TO_BYTES(ino);

	QByteArray key = QByteArray("\x01", 1) + ino_b;
	return kv.insert(key, o.encode());
}

S3FS_Obj S3FS_Store::getInode(quint64 ino) {
	INT_TO_BYTES(ino);
	QByteArray key = QByteArray("\x01", 1) + ino_b;

	return S3FS_Obj(kv.value(key));
}

bool S3FS_Store::hasInodeLocally(quint64 ino) {
	INT_TO_BYTES(ino);
	QByteArray key = QByteArray("\x01", 1) + ino_b;
	return (kv.value(key).length() > 0); // if length == 0, means we don't have this locally
}

void S3FS_Store::callbackOnInodeCached(quint64, Callback*) {
	qFatal("Not expected to reach this");
	// TODO
}

QByteArray S3FS_Store::writeBlock(const QByteArray &buf) {
	if (buf.isEmpty()) return QByteArray();

	// compute hash
	QByteArray hash = QCryptographicHash::hash(buf, algo);

	if (hasBlockLocally(hash)) return hash;

	if (!kv.insert(QByteArray("\x02")+hash, buf))
		return QByteArray();

	return hash;
}

QByteArray S3FS_Store::readBlock(const QByteArray &hash) {
	return kv.value(QByteArray("\x02")+hash);
}

bool S3FS_Store::hasBlockLocally(const QByteArray &hash) {
	return kv.contains(QByteArray("\x02")+hash);
}

void S3FS_Store::callbackOnBlockCached(const QByteArray&, Callback*) {
	qFatal("Not expected to reach this");
	// TODO
}


bool S3FS_Store::hasInodeMeta(quint64 ino, const QByteArray &key_sub) {
	INT_TO_BYTES(ino);
	QByteArray key = QByteArray("\x01", 1) + ino_b;
	return kv.contains(key+key_sub);
}

QByteArray S3FS_Store::getInodeMeta(quint64 ino, const QByteArray &key_sub) {
	INT_TO_BYTES(ino);
	QByteArray key = QByteArray("\x01", 1) + ino_b;
	return kv.value(key+key_sub);
}

bool S3FS_Store::setInodeMeta(quint64 ino, const QByteArray &key_sub, const QByteArray &value) {
	INT_TO_BYTES(ino);
	QByteArray key = QByteArray("\x01", 1) + ino_b;
	return kv.insert(key+key_sub, value);
}

S3FS_Store_MetaIterator *S3FS_Store::getInodeMetaIterator(quint64 ino) {
	INT_TO_BYTES(ino);
	QByteArray key = QByteArray("\x01", 1) + ino_b;
	return new S3FS_Store_MetaIterator(&kv, key);
}

bool S3FS_Store::removeInodeMeta(quint64 ino, const QByteArray &key_sub) {
	INT_TO_BYTES(ino);
	QByteArray key = QByteArray("\x01", 1) + ino_b;
	return kv.remove(key+key_sub);
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
	return true;
}

