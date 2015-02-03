#include "QtFuseNode.hpp"

QtFuseNode::QtFuseNode(QtFuse &_fuse, struct stat *_attr, QString _name, QtFuseNode *_parent): QObject(&_fuse), fuse(_fuse), attr(*_attr), name(_name) {
	ino = _attr->st_ino;
	parent = _parent;
}

fuse_ino_t QtFuseNode::getIno() const {
	return ino;
}

const struct stat *QtFuseNode::getAttr() const {
	return &attr;
}

bool QtFuseNode::isRoot() const {
	return parent==NULL;
}

