#include "S3Fuse.hpp"

S3Fuse::S3Fuse(QString _bucket, QString path): QtFuse(path) {
	bucket = _bucket;
}

QtFuseNode *S3Fuse::fuse_make_node(struct stat *attr, QString name, QtFuseNode *parent, fuse_ino_t ino) {
	qDebug("S3Fuse::fuse_make_node(%p,%s,%p,%ld)", attr, qPrintable(name), parent, ino);
	return NULL;
}

