#include "S3FS_Store.hpp"
#include "S3FS_Obj.hpp"
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

	QTimer::singleShot(1000, this, SLOT(test_setready()));

	if (!kv.create(kv_location)) {
		qFatal("S3FS: Failed to open cache");
	}
}

void S3FS_Store::test_setready() {
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

bool S3FS_Store::hasInodeLocally(quint64) {
	return true; // TODO
}

void S3FS_Store::callbackOnInodeCached(quint64, Callback*) {
	qFatal("Not expected to reach this");
	// TODO
}

