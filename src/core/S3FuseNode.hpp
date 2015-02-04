#include "QtFuseNode.hpp"
#include "QtFuseRequest.hpp"

#pragma once

class S3FuseNode: public QtFuseNode {
	Q_OBJECT

public:
	S3FuseNode(QtFuse &fuse, struct stat *attr, QString name, QtFuseNode *_parent);
};

