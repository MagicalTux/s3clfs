#include "QtFuse.hpp"

#pragma once

class S3FS;

class S3Fuse: public QtFuse {
	Q_OBJECT
public:
	S3Fuse(const QByteArray &path, S3FS*parent);

protected:
	virtual QtFuseNode *fuse_make_root_node(struct stat *attr);
	virtual void fuse_lookup(QtFuseRequest *req, QtFuseNode *parent, const QByteArray &name);

private:
	S3FS *parent;
};
