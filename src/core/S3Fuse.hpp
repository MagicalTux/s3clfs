#include "QtFuse.hpp"

#pragma once

class S3Fuse: public QtFuse {
	Q_OBJECT
public:
	S3Fuse(const QByteArray &path);

protected:
	virtual QtFuseNode *fuse_make_node(struct stat *attr, QString name, QtFuseNode *parent, fuse_ino_t ino=0);
};
