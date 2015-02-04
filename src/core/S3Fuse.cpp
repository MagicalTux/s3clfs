#include "S3Fuse.hpp"
#include "S3FuseNode.hpp"
#include "S3FS.hpp"

S3Fuse::S3Fuse(const QByteArray &path, S3FS *_parent): QtFuse(path) {
	parent = _parent;
}

QtFuseNode *S3Fuse::fuse_make_root_node(struct stat *attr) {
	if (inode_map.contains(1)) return inode_map.value(1);
	attr->st_ino = 1;
	inode_map_generation = 1;
	inode_map_idx = 2;
	QtFuseNode *node = new S3FuseNode(*this, attr, "", NULL);
	inode_map.insert(1, node);
	return node;
}

void S3Fuse::fuse_lookup(QtFuseRequest *req, QtFuseNode *, const QByteArray &path) {
	// valid response can be "entry" or "error"
	qDebug("Request for lookup of %s", path.data());
	req->error(ENOENT);
}

