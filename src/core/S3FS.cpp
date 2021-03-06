/*  S3ClFS - AWS S3 backed cluster filesystem
 *  Copyright (C) 2015 Mark Karpeles
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "S3FS.hpp"
#include "S3FS_Obj.hpp"
#include "S3FS_Config.hpp"
#include "S3FS_Control.hpp"
#include "S3Fuse.hpp"
#include "QtFuseRequest.hpp"
#include "S3FS_Store_MetaIterator.hpp"
#include <QDateTime>
#include <QDataStream>
#include <sys/time.h>

#define WAIT_READY() if (!is_ready) { connect(this, SIGNAL(ready()), req, SLOT(trigger())); return; } if (is_overloaded) { connect(this, SIGNAL(loadReduced()), req, SLOT(trigger())); return; }
#define GET_INODE(ino) \
	if (!store.hasInode(ino)) { req->error(ENOENT); return; } \
	if (!store.hasInodeLocally(ino)) { store.callbackOnInodeCached(ino, req); return; } S3FS_Obj &ino ## _o = *store.getInode(ino);

S3FS::S3FS(S3FS_Config *_cfg): store(_cfg) {
	cfg = _cfg;
	is_ready = false;
	is_overloaded = false;
	cluster_node_id = cfg->clusterId();

	connect(&store, SIGNAL(ready()), this, SLOT(storeIsReady()));
	connect(&store, SIGNAL(overloadStatus(bool)), this, SLOT(setOverload(bool)));

	new S3FS_Control(this, cfg);
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
	// force caching root & lost+found immediately
	store.callbackOnInodeCached(1, NULL);
}

void S3FS::format() {
	qDebug("S3FS: Formatting...");

	// create config
	QVariantMap cfg;
	cfg.insert("block_size", S3FUSE_BLOCK_SIZE);
	cfg.insert("hash_algo", "SHA3_256");

	store.setConfig(cfg);

	// create empty directory inode 1, increments generation
	S3FS_Obj root;
	root.makeRoot();
	store.storeInode(root);

	// add . and ..
	QByteArray dir_entry;
	QDataStream(&dir_entry, QIODevice::WriteOnly) << root.getInode() << root.getFiletype();
	store.setInodeMeta(1, ".", dir_entry);
	store.setInodeMeta(1, "..", dir_entry);
}

void S3FS::fuse_lookup(QtFuseRequest *req) {
	WAIT_READY();
	quint64 ino = req->inode();
	GET_INODE(ino);

//	qDebug("S3FS: lookup(%s) from inode %lu", qPrintable(path), ino);

	if (!ino_o.isDir()) {
		req->error(ENOTDIR);
		return;
	}

	if (!store.hasInodeMeta(ino, req->name())) {
		// file not found
		req->error(ENOENT);
		return;
	}

	QByteArray res_ino_bin = store.getInodeMeta(ino, req->name());
	quint64 res_ino;
	QDataStream(res_ino_bin) >> res_ino;
	if (res_ino <= 1) {
		qDebug("S3FS: Corrupted filesystem, directory contains entry pointing to inode %llu", res_ino);
		req->error(ENOENT);
		return;
	}
	if (!store.hasInode(res_ino)) {
		// corrupted inode?
		qDebug("S3FS: Entry %s of inode %llu is corrupted, removed from tree", req->name().data(), ino);
		store.removeInodeMeta(ino, req->name()); // remove reference to inode
		req->error(ENOENT);
		return;
	}

	GET_INODE(res_ino); // need to load it too

	req->entry(&res_ino_o.constAttr());
}

void S3FS::fuse_forget(fuse_ino_t, unsigned long) {
	// TODO? 
}

void S3FS::fuse_setattr(QtFuseRequest *req) {
	WAIT_READY();
	quint64 ino = req->inode();
	GET_INODE(ino);
	auto attr = req->attr();
	int to_set = req->fuseInt();

//	qDebug("S3FS: about to setattr on inode %lu", ino);

	struct stat s = ino_o.constAttr();

	if (to_set & FUSE_SET_ATTR_MODE) s.st_mode = (attr->st_mode & 07777) | (s.st_mode & S_IFMT);
	if (to_set & FUSE_SET_ATTR_UID) s.st_uid = attr->st_uid;
	if (to_set & FUSE_SET_ATTR_GID) s.st_gid = attr->st_gid;
	if (ino_o.isFile()) { // do not allow setting size on anything else than a file
		if (to_set & FUSE_SET_ATTR_SIZE) s.st_size = attr->st_size; // TODO drop data that shouldn't be there anymore
	}
	if ((to_set & FUSE_SET_ATTR_ATIME_NOW) || (to_set & FUSE_SET_ATTR_MTIME_NOW)) {
		struct timeval tmp;
		gettimeofday(&tmp, NULL);
		struct timespec t;
		t.tv_sec = tmp.tv_sec;
		t.tv_nsec = tmp.tv_usec * 1000;

		if (to_set | FUSE_SET_ATTR_ATIME_NOW) s.st_atim = t;
		if (to_set | FUSE_SET_ATTR_MTIME_NOW) s.st_mtim = t;
	}
	if (to_set & FUSE_SET_ATTR_ATIME) s.st_atim = attr->st_atim;
	if (to_set & FUSE_SET_ATTR_MTIME) s.st_mtim = attr->st_mtim;

	ino_o.setAttr(s);
	ino_o.touch(false); // update ctime
	store.storeInode(ino_o);
	req->attr(&ino_o.constAttr());
}

void S3FS::fuse_getattr(QtFuseRequest *req) {
	WAIT_READY();
	quint64 ino = req->inode();
	GET_INODE(ino);

	req->attr(&(ino_o.constAttr()));
}

void S3FS::fuse_unlink(QtFuseRequest *req) {
	WAIT_READY();
	quint64 parent = req->inode();
	GET_INODE(parent);

	if (!store.hasInodeMeta(parent, req->name())) {
		req->error(ENOENT);
		return;
	}

	quint64 ino_n;
	quint32 type_n;
	QDataStream(store.getInodeMeta(parent, req->name())) >> ino_n >> type_n;
	if (ino_n <= 1) {
		qDebug("S3FS: filesystem seems corrupted, please run fsck");
		req->error(ENOENT);
		return;
	}

	if ((type_n & S_IFMT) == S_IFDIR) {
		req->error(EISDIR);
		return;
	}

	if (!store.removeInodeMeta(parent, req->name())) {
		req->error(EIO);
		return;
	}

	parent_o.touch(true);
	store.storeInode(parent_o);

	req->error(0);
}

void S3FS::fuse_readlink(QtFuseRequest *req) {
	WAIT_READY();
	quint64 node = req->inode();
	GET_INODE(node);

	if (!node_o.isSymlink()) {
		req->error(EINVAL);
		return;
	}

	req->readlink(store.getInodeMeta(node, QByteArrayLiteral("\x00")));
}

void S3FS::fuse_mkdir(QtFuseRequest *req) {
	WAIT_READY();
	quint64 parent = req->inode();
	GET_INODE(parent);
	int mode = req->fuseInt();

	if (store.hasInodeMeta(parent, req->name())) {
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
	parent_o.touch(true);
	store.storeInode(parent_o);
	store.setInodeMeta(parent, req->name(), dir_entry);

	// add ..
	dir_entry.clear();
	QDataStream(&dir_entry, QIODevice::WriteOnly) << parent_o.getInode() << parent_o.getFiletype();
	store.setInodeMeta(new_dir.getInode(), "..", dir_entry);

	req->entry(&new_dir.constAttr());
}

void S3FS::fuse_rmdir(QtFuseRequest *req) {
	WAIT_READY();
	quint64 parent = req->inode();
	GET_INODE(parent);

	if (!store.hasInodeMeta(parent, req->name())) {
		req->error(ENOENT);
		return;
	}

	if ((req->name() == ".") || (req->name() == "..")) {
		req->error(EINVAL);
		return;
	}

	quint64 ino_n;
	quint32 type_n;
	QDataStream(store.getInodeMeta(parent, req->name())) >> ino_n >> type_n;
	if (ino_n <= 1) {
		qDebug("S3FS: filesystem seems corrupted, please run fsck");
		req->error(ENOENT);
		return;
	}

	if ((type_n & S_IFMT) != S_IFDIR) {
		req->error(ENOTDIR);
		return;
	}

	// check if not empty
	GET_INODE(ino_n);
	Q_UNUSED(ino_n_o);
	auto i = store.getInodeMetaIterator(ino_n);
	bool is_empty = true;
	do {
		if ((i->key() != "") && (i->key() != ".") && (i->key() != "..")) {
			is_empty = false;
			break;
		}
	} while(i->next());
	delete i;

	if (!is_empty) {
		req->error(ENOTEMPTY);
		return;
	}

	if (!store.removeInodeMeta(parent, req->name())) {
		req->error(EIO);
		return;
	}
	store.clearInodeMeta(ino_n); // remove . and ..

	parent_o.touch(true);
	store.storeInode(parent_o);

	req->error(0);
}

void S3FS::fuse_symlink(QtFuseRequest *req) {
	WAIT_READY();
	quint64 parent = req->inode();
	GET_INODE(parent);

	if (store.hasInodeMeta(parent, req->name())) {
		req->error(EEXIST);
		return;
	}

	S3FS_Obj symlink;
	symlink.makeEntry(makeInode(), S_IFLNK, 0777, req->context()->uid, req->context()->gid);

	store.storeInode(symlink);
	store.setInodeMeta(symlink.getInode(), QByteArrayLiteral("\x00"), req->value());

	// store dir entry
	QByteArray dir_entry;
	QDataStream(&dir_entry, QIODevice::WriteOnly) << symlink.getInode() << symlink.getFiletype();
	store.setInodeMeta(parent, req->name(), dir_entry);
	parent_o.touch(true);
	store.storeInode(parent_o);

	req->entry(&symlink.constAttr());
}

void S3FS::fuse_rename(QtFuseRequest *req) {
	WAIT_READY();
	quint64 parent = req->inode();
	quint64 newparent = req->newInode();
	GET_INODE(parent);
	GET_INODE(newparent);

	if (!newparent_o.isDir()) {
		req->error(ENOTDIR);
		return;
	}

	if (!store.hasInodeMeta(parent, req->name())) {
		req->error(ENOENT);
		return;
	}

	QByteArray file_ino_type = store.getInodeMeta(parent, req->name());

	if (!store.hasInodeMeta(newparent, req->value())) {
		// most simple, rename target doesn't exist
		if (!store.setInodeMeta(newparent, req->value(), file_ino_type)) {
			req->error(EIO);
			return;
		}
		store.removeInodeMeta(parent, req->name());
		req->error(0);
		return;
	}

	quint64 file_ino, file_type;
	QDataStream(file_ino_type) >> file_ino >> file_type;

	quint64 newfile_ino, newfile_type;
	QByteArray newfile_ino_type = store.getInodeMeta(newparent, req->value());
	QDataStream(newfile_ino_type) >> newfile_ino >> newfile_type;

	if (file_ino == newfile_ino) {
		// actually the same file, just remove old name & be done with it
		store.removeInodeMeta(parent, req->name());
		req->error(0);
		return;
	}

	if (((file_type & S_IFMT) != S_IFDIR) && (newfile_type & S_IFMT) == S_IFDIR) {
		// trying to overwrite a directory with something else than a directory? nope.
		req->error(EISDIR);
		return;
	}
	if (((file_type & S_IFMT) == S_IFDIR) && (newfile_type & S_IFMT) != S_IFDIR) {
		// trying to overwrite something else than a directory with a directory? nope.
		req->error(ENOTDIR);
		return;
	}

	// TODO: if both are directories, check that target dir is empty
	if (!store.setInodeMeta(newparent, req->value(), file_ino_type)) {
		req->error(EIO);
		return;
	}
	store.removeInodeMeta(parent, req->name());

	parent_o.touch(true);
	store.storeInode(parent_o);
	newparent_o.touch(true);
	store.storeInode(newparent_o);

	req->error(0);
}

void S3FS::fuse_link(QtFuseRequest *req) {
	WAIT_READY();
	quint64 ino = req->inode();
	quint64 newparent = req->newInode();
	GET_INODE(ino);
	GET_INODE(newparent);

	if (!newparent_o.isDir()) {
		req->error(ENOTDIR);
		return;
	}

	if (!ino_o.isFile()) {
		req->error(EINVAL);
		return;
	}

	if (store.hasInodeMeta(newparent, req->value())) {
		req->error(EEXIST);
		return;
	}

	// store dir entry
	QByteArray dir_entry;
	QDataStream(&dir_entry, QIODevice::WriteOnly) << ino_o.getInode() << ino_o.getFiletype();
	store.setInodeMeta(newparent, req->value(), dir_entry);

	newparent_o.touch(true);
	store.storeInode(newparent_o);

	req->entry(&ino_o.constAttr());
}

void S3FS::fuse_flush(QtFuseRequest *req) {
	// nothing here for now
	req->error(0);
}

void S3FS::fuse_release(QtFuseRequest *req) {
	// nothing here for now
	req->error(0);
}

void S3FS::fuse_open(QtFuseRequest *req) {
	WAIT_READY();
	quint64 ino = req->inode();
	GET_INODE(ino);
	auto fi = req->fi();

	if (ino_o.isDir()) {
		req->error(EISDIR);
		return;
	}

	if (!ino_o.isFile()) {
		req->error(ENODEV); // TODO is this the best errno for this case?
		return;
	}

	// TODO check fi->flags
	if (fi->flags & O_TRUNC) {
		// need to truncate whole file
		store.clearInodeMeta(ino);
		ino_o.setSize(0);
		store.storeInode(ino_o);
	}

	req->open(fi);
}

void S3FS::fuse_opendir(QtFuseRequest *req) {
	WAIT_READY();
	quint64 ino = req->inode();
	GET_INODE(ino);
	auto fi = req->fi();

	if (!ino_o.isDir()) {
		req->error(ENOTDIR);
		return;
	}

	fi->fh = (uintptr_t)store.getInodeMetaIterator(ino); // next entry to read

	req->open(fi);
}

void S3FS::fuse_readdir(QtFuseRequest *req) {
	WAIT_READY();
	quint64 ino = req->inode();
	GET_INODE(ino);
	auto fi = req->fi();

	if (!ino_o.isDir()) {
		req->error(ENOTDIR);
		return;
	}

	S3FS_Store_MetaIterator *fh = (S3FS_Store_MetaIterator*)fi->fh;
	if (!fh) {
		req->error(EBADF);
		return;
	}

	off_t off = req->offset();

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

void S3FS::fuse_releasedir(QtFuseRequest *req) {
	WAIT_READY();
	quint64 ino = req->inode();
	GET_INODE(ino);
	Q_UNUSED(ino_o);

	S3FS_Store_MetaIterator *fh = (S3FS_Store_MetaIterator*)req->fi()->fh;
	if (fh)
		delete fh;

	req->error(0); // success
}

void S3FS::fuse_create(QtFuseRequest *req) {
	WAIT_READY();
	quint64 parent = req->inode();
	GET_INODE(parent);
	auto fi = req->fi();
	mode_t mode = req->fuseInt();

	// check if file exists
	if (store.hasInodeMeta(parent, req->name())) {
		// TODO handle fi->flags (open file)
		quint64 child_ino;
		QDataStream(store.getInodeMeta(parent, req->name())) >> child_ino;
		if (child_ino > 1) {
			GET_INODE(child_ino);
			// should we truncate file?
			if (fi->flags & O_TRUNC) {
				// need to truncate whole file
				store.clearInodeMeta(child_ino);
				child_ino_o.setSize(0);
				store.storeInode(child_ino_o);
			}
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
	store.setInodeMeta(parent, req->name(), dir_entry);
	parent_o.touch(true);
	store.storeInode(parent_o);

	req->create(&new_file.constAttr(), fi);
}

void S3FS::fuse_read(QtFuseRequest *req) {
	WAIT_READY();
	quint64 ino = req->inode();
	GET_INODE(ino);

	QByteArray buf;

	size_t size = req->size();
	off_t offset = req->offset();

	if ((quint64)offset >= ino_o.size()) {
		req->buf(QByteArray()); // no data
		return;
	}

	if (size + offset > ino_o.size()) {
		size = ino_o.size() - offset;
	}

	// check for block(s)
	quint64 pos = offset;
	quint64 final_pos = offset+size;

	// read while we need to read more
	while(pos < final_pos) {
		qint64 offset_block = pos - (pos % S3FUSE_BLOCK_SIZE);

		QByteArray offset_block_b;
		QDataStream(&offset_block_b, QIODevice::WriteOnly) << offset_block;
		QByteArray tmp_buf;

		if (store.hasInodeMeta(ino, offset_block_b)) {
			QByteArray block_id = store.getInodeMeta(ino, offset_block_b);

			if (!store.hasBlockLocally(block_id)) {
				// need to get block & retry
				store.callbackOnBlockCached(block_id, req);
				return;
			}
			quint64 block_pos = pos % S3FUSE_BLOCK_SIZE;
			tmp_buf = store.readBlock(block_id).mid(block_pos, S3FUSE_BLOCK_SIZE - block_pos);

			quint64 need_add = S3FUSE_BLOCK_SIZE - block_pos - tmp_buf.length();
			if (need_add) tmp_buf.append(QByteArray(need_add, '\0'));
		} else {
			quint64 block_pos = pos % S3FUSE_BLOCK_SIZE;
			tmp_buf = QByteArray(S3FUSE_BLOCK_SIZE - block_pos, '\0');
		}
		if ((quint64)tmp_buf.length() > final_pos-pos)
			tmp_buf.resize(final_pos-pos);

		if (tmp_buf.length() == 0) abort();

		buf += tmp_buf;
		pos += tmp_buf.length();
	}

	req->buf(buf);
}

void S3FS::fuse_write(QtFuseRequest *req) {
	WAIT_READY();
	quint64 ino = req->inode();
	GET_INODE(ino);
	bool need_wait = false;

	const QByteArray &buf = req->value();
	off_t offset = req->offset();

	// write data
	if (buf.isEmpty()) {
		req->error(EINVAL);
		return;
	}

	// OK, now cut buffer into pieces that fit into S3FUSE_BLOCK_SIZE
	size_t pos = 0;
	
	if (offset % S3FUSE_BLOCK_SIZE) {
		qint64 maxlen = S3FUSE_BLOCK_SIZE - (offset % S3FUSE_BLOCK_SIZE);
		if (buf.length() < maxlen) {
			if (!real_write(ino_o, buf, offset, req, need_wait)) {
				ino_o.touch(true);
				store.storeInode(ino_o);
				if (need_wait) return;
				req->error(EIO);
				return;
			}
			ino_o.touch(true);
			store.storeInode(ino_o);
			req->write(buf.length());
			return;
		}
		// too bad, buf doesn't fit
		if (!real_write(ino_o, buf.left(maxlen), offset, req, need_wait)) {
			store.storeInode(ino_o);
			if (need_wait) return;
			req->error(EIO);
			return;
		}
		pos = maxlen;
	}

	size_t len = buf.length();

	while(pos < len) {
		if (pos + S3FUSE_BLOCK_SIZE > len) {
			// final block
			if (!real_write(ino_o, buf.mid(pos), offset+pos, req, need_wait)) {
				ino_o.touch(true);
				store.storeInode(ino_o);
				if (need_wait) return;
				req->error(EIO);
				return;
			}
			ino_o.touch(true);
			store.storeInode(ino_o);
			req->write(len);
			return;
		}
		// middle write
		if (!real_write(ino_o, buf.mid(pos, S3FUSE_BLOCK_SIZE), offset+pos, req, need_wait)) {
			ino_o.touch(true);
			store.storeInode(ino_o);
			if (need_wait) return;
			req->error(EIO);
			return;
		}
		pos += S3FUSE_BLOCK_SIZE;
	}
	ino_o.touch(true);
	store.storeInode(ino_o);
	req->write(pos); // if len was a multiple of S3FUSE_BLOCK_SIZE
}

// hing this as inline for optimization
inline bool S3FS::real_write(S3FS_Obj &ino, const QByteArray &buf, off_t offset, QtFuseRequest *req, bool &need_wait) {
	qint64 offset_block = offset - (offset % S3FUSE_BLOCK_SIZE);
	qint64 offset_in_block = offset - offset_block;

	QByteArray offset_block_b;
	QDataStream(&offset_block_b, QIODevice::WriteOnly) << offset_block;

	if ((offset == offset_block) && (buf.length() == S3FUSE_BLOCK_SIZE)) {
		// ok, that's easy
		QByteArray block_id = store.writeBlock(buf);
		store.setInodeMeta(ino.getInode(), offset_block_b, block_id);
		// update size if needed
		if (((quint64)offset + S3FUSE_BLOCK_SIZE) > ino.size())
			ino.setSize(offset + S3FUSE_BLOCK_SIZE);
		return true;
	}

	if ((offset == offset_block) && ((quint64)(buf.length() + offset) >= ino.size())) {
		// writing a block that will be at the end of this file, easy
		QByteArray block_id = store.writeBlock(buf);
		store.setInodeMeta(ino.getInode(), offset_block_b, block_id);
		ino.setSize(offset + buf.length());
		return true;
	}

	if (store.hasInodeMeta(ino.getInode(), offset_block_b)) {
		// need to get that block, update it, and write it again
		QByteArray old_block_id = store.getInodeMeta(ino.getInode(), offset_block_b);
		if (!store.hasBlockLocally(old_block_id)) {
			// need to get block & retry
			store.callbackOnBlockCached(old_block_id, req);
			need_wait = true;
			return false;
		}
		QByteArray block_data = store.readBlock(old_block_id);

		if (block_data.length() <= offset_in_block) {
			// add zeroes (note, we could have used block_data.resize() but then empty data would be undefined, we want it to be zeroes)
			block_data.append(QByteArray(offset_in_block-block_data.length(), '\0'));
			// add actual data
			block_data.append(buf);
		} else if (block_data.length() > (offset_in_block+buf.length())) {
			// need to insert buf in there
			block_data = block_data.mid(0, offset_in_block) + buf + block_data.mid(offset_in_block+buf.length());
		} else if (block_data.length() > offset_in_block) {
			// need to replace existing data with new data
			block_data = block_data.mid(0, offset_in_block);
			// add actual data
			block_data.append(buf);
		}

		// store new block
		QByteArray block_id = store.writeBlock(block_data);
		store.setInodeMeta(ino.getInode(), offset_block_b, block_id);
		// it is quite likely we caused file size to change
		if ((quint64)(offset + buf.length()) > ino.size())
			ino.setSize(offset + buf.length());
		return true;
	}

	// there was no data here, create a block to hold the data we received
	// possibly prefix zeroes because offset is not at block start
	QByteArray block_data(offset-offset_block, '\0');
	block_data.append(buf);
	QByteArray block_id = store.writeBlock(block_data);
	store.setInodeMeta(ino.getInode(), offset_block_b, block_id);
	if ((quint64)(offset + buf.length()) > ino.size())
		ino.setSize(offset + buf.length());
	return true;
}

quint64 S3FS::makeInode() {
	quint64 new_inode = QDateTime::currentMSecsSinceEpoch()*1000 + cluster_node_id;

	if (new_inode <= last_inode) { // ensure we are incremental
		new_inode = last_inode+100;
	}
	last_inode = new_inode;
	return new_inode;
}

void S3FS::setOverload(bool status) {
	if (is_overloaded == status) return;
	is_overloaded = status;
	if (!status) {
		qDebug("S3FS: network load reduced, resuming operations");
		loadReduced();
	} else {
		qDebug("S3FS: network load too high, waiting for cool down");
	}
}

