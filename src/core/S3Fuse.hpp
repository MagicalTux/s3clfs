#include "QtFuse.hpp"

#pragma once

class S3FS;

class S3Fuse: public QtFuse {
	Q_OBJECT
public:
	S3Fuse(const QByteArray &path, S3FS*parent);

protected:
	virtual void fuse_getattr(QtFuseRequest *req, fuse_ino_t node, struct fuse_file_info *fi);
	virtual void fuse_lookup(QtFuseRequest *req, fuse_ino_t parent, const QByteArray &name);

private:
	S3FS *parent;
};
