#include "S3Fuse.hpp"
#include "S3FS.hpp"
#include "QtFuseRequest.hpp"

S3Fuse::S3Fuse(const QByteArray &bucket, const QByteArray &path, S3FS *_parent): QtFuse(path, bucket) {
	parent = _parent;
}

void S3Fuse::fuse_init(struct fuse_conn_info *ci) {
	ci->async_read = 1;
	ci->max_write = 65536;
	ci->max_readahead = 65536;
	ci->want = FUSE_CAP_ASYNC_READ | FUSE_CAP_ATOMIC_O_TRUNC | FUSE_CAP_EXPORT_SUPPORT | FUSE_CAP_BIG_WRITES | FUSE_CAP_IOCTL_DIR;
	ci->max_background = 16;
	ci->congestion_threshold = 32;
}

void S3Fuse::fuse_lookup(QtFuseRequest *req, fuse_ino_t ino, const QByteArray &path) {
	parent->fuse_lookup(req, ino, path);
}

void S3Fuse::fuse_setattr(QtFuseRequest *req, fuse_ino_t node, struct stat *attr, int to_set, struct fuse_file_info *fi) {
	parent->fuse_setattr(req, node, attr, to_set, fi);
}

void S3Fuse::fuse_getattr(QtFuseRequest *req, fuse_ino_t ino, struct fuse_file_info *fi) {
	parent->fuse_getattr(req, ino, fi);
}

void S3Fuse::fuse_opendir(QtFuseRequest *req, fuse_ino_t ino, struct fuse_file_info *fi) {
	parent->fuse_opendir(req, ino, fi);
}

void S3Fuse::fuse_releasedir(QtFuseRequest *req, fuse_ino_t ino, struct fuse_file_info *fi) {
	parent->fuse_releasedir(req, ino, fi);
}

