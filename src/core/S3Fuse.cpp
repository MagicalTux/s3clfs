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

void S3Fuse::fuse_getattr(QtFuseRequest *req, fuse_ino_t ino, struct fuse_file_info *) {
	qDebug("S3Fuse::getattr for inode %ld", ino);
	if (ino != 1) {
		req->error(ENOSYS);
		return;
	}

	struct stat attr;
	attr.st_dev = 0x1337;
	attr.st_ino = 1;
	attr.st_mode = 0755 | S_IFDIR;
	attr.st_nlink = 1;
	attr.st_uid = 0;
	attr.st_gid = 0;
	attr.st_rdev = 0;
	attr.st_size = 0;
	attr.st_blksize = 512;
	attr.st_blocks = 0;
	attr.st_atime = 0;
	attr.st_mtime = 0;
	attr.st_ctime = 0;

	req->attr(&attr);
}

