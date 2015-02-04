#include "S3Fuse.hpp"
#include "S3FS.hpp"
#include "QtFuseRequest.hpp"

S3Fuse::S3Fuse(const QByteArray &path, S3FS *_parent): QtFuse(path) {
	parent = _parent;
}

void S3Fuse::fuse_lookup(QtFuseRequest *req, fuse_ino_t ino, const QByteArray &path) {
	// valid response can be "entry" or "error"
	qDebug("Request for lookup of %s from inode %ld", path.data(), ino);
	req->error(ENOENT);
}

void S3Fuse::fuse_getattr(QtFuseRequest *req, fuse_ino_t ino, struct fuse_file_info *fi) {
	parent->fuse_getattr(req, ino, fi);
}

