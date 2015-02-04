#include <QObject>
#include "S3Fuse.hpp"
#include "Keyval.hpp"

#pragma once

class S3FS: public QObject {
	Q_OBJECT

public:
	S3FS(const QByteArray &bucket, const QByteArray &path);
	~S3FS();

private:
	S3Fuse fuse;
	QByteArray bucket;
	QString kv_location;
	Keyval kv; // local cache
};

