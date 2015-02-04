#include "S3FuseNode.hpp"

S3FuseNode::S3FuseNode(QtFuse &fuse, struct stat *attr, QString name, QtFuseNode *_parent): QtFuseNode(fuse, attr, name, _parent) {
	qDebug("S3FuseNode created");
}
