#include <QObject>
#include "S3Fuse.hpp"
#include "Keyval.hpp"

#pragma once

class S3FS: public QObject {
	Q_OBJECT

public:
	S3FS(const QByteArray &bucket, const QByteArray &path);
	~S3FS();
	void format();
	bool isReady() const;

signals:
	void ready();

public slots:
	void fuse_getattr(QtFuseRequest *req, fuse_ino_t node, struct fuse_file_info *fi);
	void test_setready();

private:
	S3Fuse fuse;
	QByteArray bucket;
	QString kv_location;
	Keyval kv; // local cache
	bool is_ready;
};

