#include "S3FS.hpp"
#include <QDir>
#include <QUuid>

S3FS::S3FS(const QByteArray &_bucket, const QByteArray &path): fuse(path, this) {
	bucket = _bucket;

	// generate filename
	kv_location = QDir::temp().filePath(QString("s3clfs-")+QUuid::createUuid().toRfc4122().toHex());
	qDebug("Keyval location: %s", qPrintable(kv_location));

	if (!kv.create(kv_location)) {
		qFatal("Failed to open cache");
	}

	fuse.init();
}

S3FS::~S3FS() {
	if (kv.isValid()) {
		kv.close();
		Keyval::destroy(kv_location);
	}
}

