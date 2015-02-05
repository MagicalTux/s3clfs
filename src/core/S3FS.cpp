#include "S3FS.hpp"
#include "S3FS_Obj.hpp"
#include "QtFuseRequest.hpp"
#include "Callback.hpp"

#define METHOD_ARGS(...) QList<QGenericArgument> func_args = {__VA_ARGS__}
#define WAIT_READY() if (!is_ready) { Callback *cb = new Callback(this, __func__, func_args); connect(this, SIGNAL(ready()), cb, SLOT(trigger())); return; }
#define GET_INODE(ino) \
	if (!store.hasInode(ino)) { req->error(ENOENT); return; } \
	if (!store.hasInodeLocally(ino)) { Callback *cb = new Callback(this, __func__, func_args); store.callbackOnInodeCached(ino, cb); } S3FS_Obj ino ## _o = store.getInode(ino); \
	if (!ino ## _o.isValid()) { req->error(ENOENT); return; }

S3FS::S3FS(const QByteArray &_bucket, const QByteArray &path): fuse(_bucket, path, this), store(_bucket) {
	bucket = _bucket;
	is_ready = false;

	connect(&store, SIGNAL(ready()), this, SLOT(storeIsReady()));

	fuse.init();
}

S3FS_Store &S3FS::getStore() {
	return store;
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

void S3FS::fuse_lookup(QtFuseRequest *req, fuse_ino_t ino, const QByteArray &path) {
	METHOD_ARGS(Q_ARG(QtFuseRequest*, req), Q_ARG(fuse_ino_t, ino), Q_ARG(const QByteArray&, path));
	WAIT_READY();
	GET_INODE(ino);

	if (!ino_o.isDir()) {
		req->error(ENOTDIR);
		return;
	}

	if (!store.hasInodeMeta(ino, path)) {
		// file not found
		req->error(ENOENT);
		return;
	}

	QByteArray res_ino_bin = store.getInodeMeta(ino, path);
	quint64 res_ino;
	QDataStream(res_ino_bin) >> res_ino;
	if (res_ino <= 1) {
		qDebug("S3FS: Corrupted filesystem, directory contains entry pointing to inode %llu", res_ino);
		req->error(ENOENT);
		return;
	}

	GET_INODE(res_ino); // need to load it too

	req->entry(&res_ino_o.constAttr());
}

void S3FS::fuse_setattr(QtFuseRequest *req, fuse_ino_t ino, struct stat *attr, int to_set, struct fuse_file_info *fi) {
	METHOD_ARGS(Q_ARG(QtFuseRequest*, req), Q_ARG(fuse_ino_t, ino), Q_ARG(struct stat*, attr), Q_ARG(int, to_set), Q_ARG(struct fuse_file_info *, fi));
	WAIT_READY();
	GET_INODE(ino);

	qDebug("S3FS: about to setattr on inode %lu", ino);

	struct stat s = ino_o.constAttr();

	if (to_set & FUSE_SET_ATTR_MODE) s.st_mode = (attr->st_mode & 07777) | (s.st_mode & S_IFMT);
	if (to_set & FUSE_SET_ATTR_UID) s.st_uid = attr->st_uid;
	if (to_set & FUSE_SET_ATTR_GID) s.st_gid = attr->st_gid;
	if (ino_o.isFile()) { // do not allow setting size on anything else than a file
		if (to_set & FUSE_SET_ATTR_SIZE) s.st_size = attr->st_size; // TODO drop data that shouldn't be there anymore
	}
	if (to_set & FUSE_SET_ATTR_ATIME) s.st_atime = attr->st_atime;
	if (to_set & FUSE_SET_ATTR_MTIME) s.st_mtime = attr->st_mtime;
	if ((to_set & FUSE_SET_ATTR_ATIME_NOW) || (to_set & FUSE_SET_ATTR_MTIME_NOW)) {
		time_t t = time(NULL);
		if (to_set | FUSE_SET_ATTR_ATIME_NOW) s.st_atime = t;
		if (to_set | FUSE_SET_ATTR_MTIME_NOW) s.st_mtime = t;
	}

	ino_o.setAttr(s);
	store.storeInode(ino_o);
	req->attr(&ino_o.constAttr());
}

void S3FS::fuse_getattr(QtFuseRequest *req, fuse_ino_t ino, struct fuse_file_info *fi) {
	METHOD_ARGS(Q_ARG(QtFuseRequest*, req), Q_ARG(fuse_ino_t, ino), Q_ARG(struct fuse_file_info *, fi));
	WAIT_READY();
	GET_INODE(ino);

	req->attr(&(ino_o.constAttr()));
}

void S3FS::fuse_opendir(QtFuseRequest *req, fuse_ino_t ino, struct fuse_file_info *fi) {
	METHOD_ARGS(Q_ARG(QtFuseRequest*, req), Q_ARG(fuse_ino_t, ino), Q_ARG(struct fuse_file_info *, fi));
	WAIT_READY();
	GET_INODE(ino);

	if (!ino_o.isDir()) {
		req->error(ENOTDIR);
		return;
	}

	req->open(fi);
}

void S3FS::fuse_releasedir(QtFuseRequest *req, fuse_ino_t ino, struct fuse_file_info *fi) {
	METHOD_ARGS(Q_ARG(QtFuseRequest*, req), Q_ARG(fuse_ino_t, ino), Q_ARG(struct fuse_file_info *, fi));
	WAIT_READY();
	GET_INODE(ino);

	req->error(0); // success
}

