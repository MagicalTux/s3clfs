#include "S3FS.hpp"
#include "QtFuseRequest.hpp"
#include <QDir>
#include <QUuid>
#include <QTimer>
#include "Callback.hpp"

S3FS::S3FS(const QByteArray &_bucket, const QByteArray &path): fuse(path, this) {
	bucket = _bucket;

	// generate filename
	kv_location = QDir::temp().filePath(QString("s3clfs-")+QUuid::createUuid().toRfc4122().toHex());
	qDebug("Keyval location: %s", qPrintable(kv_location));

	if (!kv.create(kv_location)) {
		qFatal("Failed to open cache");
	}

	fuse.init();
}

S3FS::~S3FS() {
	if (kv.isValid()) {
		kv.close();
		Keyval::destroy(kv_location);
	}
}

void S3FS::format() {
	// create empty directory inode 1, increments generation
}

void S3FS::fuse_getattr(QtFuseRequest *req, fuse_ino_t ino, struct fuse_file_info *fi) {
	// delay test
	Callback *cb = new Callback(this, "real_fuse_getattr", Q_ARG(QtFuseRequest*, req), Q_ARG(fuse_ino_t, ino), Q_ARG(struct fuse_file_info *, fi));

	QTimer::singleShot(1000, cb, SLOT(trigger()));
}

void S3FS::real_fuse_getattr(QtFuseRequest *req, fuse_ino_t ino, struct fuse_file_info *) {
	qDebug("S3FS::getattr for inode %ld", ino);
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

