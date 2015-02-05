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
	S3FS_Store &getStore();

signals:
	void ready();

public slots:
	void fuse_lookup(QtFuseRequest *req, fuse_ino_t ino, const QByteArray &path);
	void fuse_setattr(QtFuseRequest *req, fuse_ino_t node, struct_stat *attr, int to_set, struct fuse_file_info *fi);
	void fuse_getattr(QtFuseRequest *req, fuse_ino_t node, struct fuse_file_info *fi);
	void fuse_mkdir(QtFuseRequest *req, fuse_ino_t parent, const QByteArray &name, int mode);
	void fuse_opendir(QtFuseRequest *req, fuse_ino_t ino, struct fuse_file_info *fi);
	void fuse_readdir(QtFuseRequest *req, fuse_ino_t ino, off_t off, struct fuse_file_info *fi);
	void fuse_releasedir(QtFuseRequest *req, fuse_ino_t ino, struct fuse_file_info *fi);
	void storeIsReady();

protected:
	quint64 makeInode();

private:
	S3Fuse fuse;
	S3FS_Store store;
	QByteArray bucket;
	bool is_ready;
	quint64 last_inode;
};

