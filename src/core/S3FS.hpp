#include <QObject>
#include "S3Fuse.hpp"
#include "S3FS_Store.hpp"
#include "Keyval.hpp"

#pragma once

class S3FS: public QObject {
	Q_OBJECT

public:
	S3FS(const QByteArray &bucket, const QByteArray &path);
	void format();
	bool isReady() const;

signals:
	void ready();

public slots:
	void fuse_getattr(QtFuseRequest *req, fuse_ino_t node, struct fuse_file_info *fi);
	void test_setready();

private:
	S3Fuse fuse;
	S3FS_Store store;
	QByteArray bucket;
	bool is_ready;
};

