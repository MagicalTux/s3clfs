#include "S3FS.hpp"
#include "S3FS_Obj.hpp"
#include "QtFuseRequest.hpp"
#include "Callback.hpp"
#include "S3FS_Store_MetaIterator.hpp"
#include <QDateTime>

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

	// add . and ..
	QByteArray dir_entry;
	QDataStream(&dir_entry, QIODevice::WriteOnly) << root.getInode() << root.getFiletype();
	store.setInodeMeta(1, ".", dir_entry);
	store.setInodeMeta(1, "..", dir_entry);
}

void S3FS::fuse_lookup(QtFuseRequest *req, fuse_ino_t ino, const QByteArray &path) {
	METHOD_ARGS(Q_ARG(QtFuseRequest*, req), Q_ARG(fuse_ino_t, ino), Q_ARG(const QByteArray&, path));
	WAIT_READY();
	GET_INODE(ino);

//	qDebug("S3FS: lookup(%s) from inode %lu", qPrintable(path), ino);

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

//	qDebug("S3FS: about to setattr on inode %lu", ino);

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

void S3FS::fuse_unlink(QtFuseRequest *req, fuse_ino_t parent, const QByteArray &name) {
	METHOD_ARGS(Q_ARG(QtFuseRequest*, req), Q_ARG(fuse_ino_t, parent), Q_ARG(const QByteArray &,name));
	WAIT_READY();
	GET_INODE(parent);

	if (!store.hasInodeMeta(parent, name)) {
		req->error(ENOENT);
		return;
	}

	quint64 ino_n;
	quint32 type_n;
	QDataStream(store.getInodeMeta(parent, name)) >> ino_n >> type_n;
	if (ino_n <= 1) {
		qDebug("S3FS: filesystem seems corrupted, please run fsck");
		req->error(ENOENT);
		return;
	}

	if ((type_n & S_IFMT) == S_IFDIR) {
		req->error(EISDIR);
		return;
	}

	if (!store.removeInodeMeta(parent, name)) {
		req->error(EIO);
		return;
	}

	req->error(0);
}

void S3FS::fuse_mkdir(QtFuseRequest *req, fuse_ino_t parent, const QByteArray &name, int mode) {
	METHOD_ARGS(Q_ARG(QtFuseRequest*, req), Q_ARG(fuse_ino_t, parent), Q_ARG(const QByteArray &,name), Q_ARG(int,mode));
	WAIT_READY();
	GET_INODE(parent);

	if (store.hasInodeMeta(parent, name)) {
		req->error(EEXIST);
		return;
	}

	// store inode
	S3FS_Obj new_dir;
	new_dir.makeDir(makeInode(), mode, req->context()->uid, req->context()->gid);
	store.storeInode(new_dir);

	// add .
	QByteArray dir_entry;
	QDataStream(&dir_entry, QIODevice::WriteOnly) << new_dir.getInode() << new_dir.getFiletype();
	store.setInodeMeta(new_dir.getInode(), ".", dir_entry);

	// store dir entry
	store.setInodeMeta(parent, name, dir_entry);

	// add ..
	dir_entry.clear();
	QDataStream(&dir_entry, QIODevice::WriteOnly) << parent_o.getInode() << parent_o.getFiletype();
	store.setInodeMeta(new_dir.getInode(), "..", dir_entry);

	req->entry(&new_dir.constAttr());
}

void S3FS::fuse_flush(QtFuseRequest *req, fuse_ino_t, struct fuse_file_info *) {
	// nothing here for now
	req->error(0);
}

void S3FS::fuse_release(QtFuseRequest *req, fuse_ino_t, struct fuse_file_info *) {
	// nothing here for now
	req->error(0);
}

void S3FS::fuse_opendir(QtFuseRequest *req, fuse_ino_t ino, struct fuse_file_info *fi) {
	METHOD_ARGS(Q_ARG(QtFuseRequest*, req), Q_ARG(fuse_ino_t, ino), Q_ARG(struct fuse_file_info *, fi));
	WAIT_READY();
	GET_INODE(ino);

	if (!ino_o.isDir()) {
		req->error(ENOTDIR);
		return;
	}

	fi->fh = (uintptr_t)store.getInodeMetaIterator(ino); // next entry to read

	req->open(fi);
}

void S3FS::fuse_readdir(QtFuseRequest *req, fuse_ino_t ino, off_t off, struct fuse_file_info *fi) {
	METHOD_ARGS(Q_ARG(QtFuseRequest*, req), Q_ARG(fuse_ino_t, ino), Q_ARG(off_t, off), Q_ARG(struct fuse_file_info *,fi));
	WAIT_READY();
	GET_INODE(ino);

	if (!ino_o.isDir()) {
		req->error(ENOTDIR);
		return;
	}

	S3FS_Store_MetaIterator *fh = (S3FS_Store_MetaIterator*)fi->fh;
	if (!fh) {
		req->error(EBADF);
		return;
	}

	while(true) {
		if (!fh->isValid()) {
			req->dir_send();
			return;
		}
		if (fh->key() == "") { // first entry
			if (!fh->next()) { // no next entry?
				req->dir_send(); // send as is
				return;
			}
		}
		struct stat s;
		quint64 ino_n; quint32 mode_n;
		QDataStream(fh->value()) >> ino_n >> mode_n;
		s.st_ino = ino_n;
		s.st_mode = mode_n;
		if (!req->dir_add(fh->key(), &s, ++off)) {
			// out of memory
			req->dir_send();
			return;
		}
		if (!fh->next()) {
			// end
			req->dir_send();
			return;
		}
	}
}

void S3FS::fuse_releasedir(QtFuseRequest *req, fuse_ino_t ino, struct fuse_file_info *fi) {
	METHOD_ARGS(Q_ARG(QtFuseRequest*, req), Q_ARG(fuse_ino_t, ino), Q_ARG(struct fuse_file_info *, fi));
	WAIT_READY();
	GET_INODE(ino);

	S3FS_Store_MetaIterator *fh = (S3FS_Store_MetaIterator*)fi->fh;
	if (fh)
		delete fh;

	req->error(0); // success
}

void S3FS::fuse_create(QtFuseRequest *req, fuse_ino_t parent, const char *name, mode_t mode, struct fuse_file_info *fi) {
	METHOD_ARGS(Q_ARG(QtFuseRequest*, req), Q_ARG(fuse_ino_t, parent), Q_ARG(const char*,name), Q_ARG(mode_t,mode), Q_ARG(struct fuse_file_info *,fi));
	WAIT_READY();
	GET_INODE(parent);

	// check if file exists
	if (store.hasInodeMeta(parent, name)) {
		// TODO handle fi->flags (open file)
		quint64 child_ino;
		QDataStream(store.getInodeMeta(parent, name)) >> child_ino;
		if (child_ino > 1) {
			GET_INODE(child_ino);
			req->create(&child_ino_o.constAttr(), fi);
			return;
		}
	}

	// store inode
	S3FS_Obj new_file;
	new_file.makeFile(makeInode(), mode, req->context()->uid, req->context()->gid);
	store.storeInode(new_file);

	// store dir entry
	QByteArray dir_entry;
	QDataStream(&dir_entry, QIODevice::WriteOnly) << new_file.getInode() << new_file.getFiletype();
	store.setInodeMeta(parent, name, dir_entry);

	req->create(&new_file.constAttr(), fi);
}

quint64 S3FS::makeInode() {
	quint64 new_inode = QDateTime::currentMSecsSinceEpoch()*1000;

	if (new_inode <= last_inode) { // ensure we are incremental
		new_inode = last_inode+100;
	}
	last_inode = new_inode;
	return new_inode;
}

