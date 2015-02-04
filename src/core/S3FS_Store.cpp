#include "S3FS_Store.hpp"
#include <QDir>
#include <QUuid>

S3FS_Store::S3FS_Store(const QByteArray &_bucket, QObject *parent): QObject(parent) {
	bucket = _bucket;
	algo = QCryptographicHash::Sha3_256; // default value
	// generate filename
	kv_location = QDir::temp().filePath(QString("s3clfs-")+QUuid::createUuid().toRfc4122().toHex());
	qDebug("S3FS: Keyval location: %s", qPrintable(kv_location));

	if (!kv.create(kv_location)) {
		qFatal("S3FS: Failed to open cache");
	}
}

S3FS_Store::~S3FS_Store() {
	if (kv.isValid()) {
		kv.close();
		Keyval::destroy(kv_location);
	}
}

