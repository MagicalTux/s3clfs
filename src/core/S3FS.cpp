#include "S3FS.hpp"
#include "S3FS_Obj.hpp"
#include "QtFuseRequest.hpp"
#include <QDir>
#include <QUuid>
#include <QTimer>
#include "Callback.hpp"

#define WAIT_READY(...) if (!is_ready) { Callback *cb = new Callback(this, __func__, __VA_ARGS__); connect(this, SIGNAL(ready()), cb, SLOT(trigger())); return; }

S3FS::S3FS(const QByteArray &_bucket, const QByteArray &path): fuse(path, this) {
	bucket = _bucket;
	is_ready = false;

	// generate filename
	kv_location = QDir::temp().filePath(QString("s3clfs-")+QUuid::createUuid().toRfc4122().toHex());
	qDebug("S3FS: Keyval location: %s", qPrintable(kv_location));

	if (!kv.create(kv_location)) {
		qFatal("S3FS: Failed to open cache");
	}

	QTimer::singleShot(1000, this, SLOT(test_setready()));

	fuse.init();
}

S3FS::~S3FS() {
	if (kv.isValid()) {
		kv.close();
		Keyval::destroy(kv_location);
	}
}

bool S3FS::isReady() const {
	return is_ready;
}

void S3FS::test_setready() {
	format();
	qDebug("SET READY");
	is_ready = true;
	ready();
}

void S3FS::format() {
	// create empty directory inode 1, increments generation
	S3FS_Obj root;
	root.makeRoot();

	qDebug("Got root block: %s", root.encode().toHex().data());
}

void S3FS::fuse_getattr(QtFuseRequest *req, fuse_ino_t ino, struct fuse_file_info *fi) {
	WAIT_READY(Q_ARG(QtFuseRequest*, req), Q_ARG(fuse_ino_t, ino), Q_ARG(struct fuse_file_info *, fi));

	qDebug("S3FS::getattr for inode %ld req=%p", ino, req);
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

