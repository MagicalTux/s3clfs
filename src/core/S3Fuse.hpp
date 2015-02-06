#include "QtFuse.hpp"

#pragma once

class S3FS;

class S3Fuse: public QtFuse {
	Q_OBJECT
public:
	S3Fuse(const QByteArray &bucket, const QByteArray &path, S3FS*parent);

protected:
	virtual void fuse_init(struct fuse_conn_info *);
	virtual void fuse_getattr(QtFuseRequest *req, fuse_ino_t node, struct fuse_file_info *fi);
	virtual void fuse_lookup(QtFuseRequest *req, fuse_ino_t parent, const QByteArray &name);
	virtual void fuse_setattr(QtFuseRequest *req, fuse_ino_t node, struct stat *attr, int to_set, struct fuse_file_info *fi);
	virtual void fuse_unlink(QtFuseRequest *req, fuse_ino_t parent, const QByteArray &name);
	virtual void fuse_mkdir(QtFuseRequest *req, fuse_ino_t parent, const QByteArray &name, int mode);
	virtual void fuse_flush(QtFuseRequest *req, fuse_ino_t ino, struct fuse_file_info *fi);
	virtual void fuse_release(QtFuseRequest *req, fuse_ino_t ino, struct fuse_file_info *fi);
	virtual void fuse_opendir(QtFuseRequest *req, fuse_ino_t ino, struct fuse_file_info *fi);
	virtual void fuse_readdir(QtFuseRequest *req, fuse_ino_t ino, off_t off, struct fuse_file_info *fi);
	virtual void fuse_releasedir(QtFuseRequest *req, fuse_ino_t ino, struct fuse_file_info *fi);
	virtual void fuse_create(QtFuseRequest *req, fuse_ino_t parent, const char *name, mode_t mode, struct fuse_file_info *fi);

private:
	S3FS *parent;
};
