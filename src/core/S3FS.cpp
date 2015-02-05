#include "S3FS.hpp"
#include "S3FS_Obj.hpp"
#include "QtFuseRequest.hpp"
#include "Callback.hpp"

#define WAIT_READY(...) if (!is_ready) { Callback *cb = new Callback(this, __func__, __VA_ARGS__); connect(this, SIGNAL(ready()), cb, SLOT(trigger())); return; }
#define GET_INODE(ino, ...) if (!store.hasInodeLocally(ino)) { Callback *cb = new Callback(this, __func__, __VA_ARGS__); store.callbackOnInodeCached(ino, cb); } S3FS_Obj ino ## _o = store.getInode(ino)

S3FS::S3FS(const QByteArray &_bucket, const QByteArray &path): fuse(path, this), store(_bucket) {
	bucket = _bucket;
	is_ready = false;

	connect(&store, SIGNAL(ready()), this, SLOT(storeIsReady()));

	fuse.init();
}

bool S3FS::isReady() const {
	return is_ready;
}

void S3FS::storeIsReady() {
	if (!store.hasInode(1)) {
		qDebug("S3FS: Filesystem has no root inode, creating one now");
		format();
	}
	qDebug("S3FS: ready!");
	is_ready = true;
	ready();
}

void S3FS::format() {
	// create empty directory inode 1, increments generation
	S3FS_Obj root;
	root.makeRoot();

	qDebug("S3FS: Created root block");

	store.storeInode(root);
}

void S3FS::fuse_getattr(QtFuseRequest *req, fuse_ino_t ino, struct fuse_file_info *fi) {
	WAIT_READY(Q_ARG(QtFuseRequest*, req), Q_ARG(fuse_ino_t, ino), Q_ARG(struct fuse_file_info *, fi));

	if (!store.hasInode(ino)) {
		req->error(ENOENT);
		return;
	}

	GET_INODE(ino, Q_ARG(QtFuseRequest*, req), Q_ARG(fuse_ino_t, ino), Q_ARG(struct fuse_file_info *, fi));

	if (!ino_o.isValid()) {
		req->error(ENOENT);
		return;
	}

	req->attr(&(ino_o.constAttr()));
}

