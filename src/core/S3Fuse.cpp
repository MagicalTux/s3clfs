#include "S3Fuse.hpp"
#include "S3FuseNode.hpp"

S3Fuse::S3Fuse(QString _bucket, QString path): QtFuse(path) {
	bucket = _bucket;
	init();
}

QtFuseNode *S3Fuse::fuse_make_node(struct stat *attr, QString name, QtFuseNode *parent, fuse_ino_t ino) {
	qDebug("S3Fuse::fuse_make_node(%p,%s,%p,%ld)", attr, qPrintable(name), parent, ino);

	if ((ino == 0) || (inode_map.contains(ino))) {
		// need to allocate inode number
		for(; inode_map.contains(inode_map_idx); inode_map_idx++);
		ino = inode_map_idx;
	}
	attr->st_ino = ino;
	inode_map_generation = 1;
	inode_map_idx = 2;
	QtFuseNode *node = new S3FuseNode(*this, attr, name, parent);
	inode_map.insert(ino, node);
	return node;
}

