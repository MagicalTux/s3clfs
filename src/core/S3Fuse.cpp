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
#include "S3Fuse.hpp"
#include "S3FS.hpp"
#include "S3FS_Config.hpp"
#include "QtFuseRequest.hpp"

S3Fuse::S3Fuse(S3FS_Config *cfg, S3FS *_parent): QtFuse(cfg->mountPath(), cfg->bucket(), cfg->mountOptions(), _parent) {
	parent = _parent;
}

void S3Fuse::fuse_init(struct fuse_conn_info *ci) {
	ci->async_read = 1;
	ci->max_write = S3FUSE_BLOCK_SIZE;
	ci->max_readahead = S3FUSE_BLOCK_SIZE * 32;
	ci->capable &= ~FUSE_CAP_SPLICE_READ;
	ci->want = FUSE_CAP_ASYNC_READ | FUSE_CAP_ATOMIC_O_TRUNC | FUSE_CAP_EXPORT_SUPPORT | FUSE_CAP_BIG_WRITES | FUSE_CAP_IOCTL_DIR;
	ci->max_background = 16;
	ci->congestion_threshold = 32;
}

void S3Fuse::fuse_lookup(QtFuseRequest *req, fuse_ino_t ino, const QByteArray &path) {
	QMetaObject::invokeMethod(parent, "fuse_lookup", Q_ARG(QtFuseRequest*,req), Q_ARG(fuse_ino_t,ino), Q_ARG(const QByteArray&,path));
}

void S3Fuse::fuse_forget(fuse_ino_t ino, unsigned long nlookup) {
	QMetaObject::invokeMethod(parent, "fuse_forget", Q_ARG(fuse_ino_t,ino), Q_ARG(unsigned long,nlookup));
}

void S3Fuse::fuse_setattr(QtFuseRequest *req, fuse_ino_t node, int to_set) {
	QMetaObject::invokeMethod(parent, "fuse_setattr", Q_ARG(QtFuseRequest*,req), Q_ARG(fuse_ino_t,node), Q_ARG(int,to_set));
}

void S3Fuse::fuse_getattr(QtFuseRequest *req, fuse_ino_t ino) {
	QMetaObject::invokeMethod(parent, "fuse_getattr", Q_ARG(QtFuseRequest*,req), Q_ARG(fuse_ino_t,ino));
}

void S3Fuse::fuse_unlink(QtFuseRequest *req, fuse_ino_t parent_ino, const QByteArray &name) {
	QMetaObject::invokeMethod(parent, "fuse_unlink", Q_ARG(QtFuseRequest*,req), Q_ARG(fuse_ino_t,parent_ino), Q_ARG(const QByteArray&,name));
}

void S3Fuse::fuse_readlink(QtFuseRequest *req, fuse_ino_t node) {
	QMetaObject::invokeMethod(parent, "fuse_readlink", Q_ARG(QtFuseRequest*,req), Q_ARG(fuse_ino_t,node));
}

void S3Fuse::fuse_mkdir(QtFuseRequest *req, fuse_ino_t parent_ino, const QByteArray &name, int mode) {
	QMetaObject::invokeMethod(parent, "fuse_mkdir", Q_ARG(QtFuseRequest*,req), Q_ARG(fuse_ino_t,parent_ino), Q_ARG(const QByteArray&,name), Q_ARG(int,mode));
}

void S3Fuse::fuse_rmdir(QtFuseRequest *req, fuse_ino_t parent_ino, const QByteArray &name) {
	QMetaObject::invokeMethod(parent, "fuse_rmdir", Q_ARG(QtFuseRequest*,req), Q_ARG(fuse_ino_t,parent_ino), Q_ARG(const QByteArray&,name));
}

void S3Fuse::fuse_symlink(QtFuseRequest *req, const QByteArray &link, fuse_ino_t parent_ino, const QByteArray &name) {
	QMetaObject::invokeMethod(parent, "fuse_symlink", Q_ARG(QtFuseRequest*,req), Q_ARG(const QByteArray&,link), Q_ARG(fuse_ino_t,parent_ino), Q_ARG(const QByteArray&,name));
}

void S3Fuse::fuse_rename(QtFuseRequest *req, fuse_ino_t parent_ino, const QByteArray &name, fuse_ino_t newparent, const QByteArray &newname) {
	QMetaObject::invokeMethod(parent, "fuse_rename", Q_ARG(QtFuseRequest*,req), Q_ARG(fuse_ino_t,parent_ino), Q_ARG(const QByteArray&,name), Q_ARG(fuse_ino_t,newparent), Q_ARG(const QByteArray&,newname));
}

void S3Fuse::fuse_link(QtFuseRequest *req, fuse_ino_t ino, fuse_ino_t newparent, const QByteArray &newname) {
	QMetaObject::invokeMethod(parent, "fuse_link", Q_ARG(QtFuseRequest*,req), Q_ARG(fuse_ino_t,ino), Q_ARG(fuse_ino_t,newparent), Q_ARG(const QByteArray&,newname));
}

void S3Fuse::fuse_flush(QtFuseRequest *req, fuse_ino_t ino) {
	QMetaObject::invokeMethod(parent, "fuse_flush", Q_ARG(QtFuseRequest*,req), Q_ARG(fuse_ino_t,ino));
}

void S3Fuse::fuse_release(QtFuseRequest *req, fuse_ino_t ino) {
	QMetaObject::invokeMethod(parent, "fuse_release", Q_ARG(QtFuseRequest*,req), Q_ARG(fuse_ino_t,ino));
}

void S3Fuse::fuse_open(QtFuseRequest *req, fuse_ino_t ino) {
	QMetaObject::invokeMethod(parent, "fuse_open", Q_ARG(QtFuseRequest*,req), Q_ARG(fuse_ino_t,ino));
}

void S3Fuse::fuse_opendir(QtFuseRequest *req, fuse_ino_t ino) {
	QMetaObject::invokeMethod(parent, "fuse_opendir", Q_ARG(QtFuseRequest*,req), Q_ARG(fuse_ino_t,ino));
}

void S3Fuse::fuse_readdir(QtFuseRequest *req, fuse_ino_t ino, off_t off) {
	QMetaObject::invokeMethod(parent, "fuse_readdir", Q_ARG(QtFuseRequest*,req), Q_ARG(fuse_ino_t,ino), Q_ARG(off_t,off));
}

void S3Fuse::fuse_releasedir(QtFuseRequest *req, fuse_ino_t ino) {
	QMetaObject::invokeMethod(parent, "fuse_releasedir", Q_ARG(QtFuseRequest*,req), Q_ARG(fuse_ino_t,ino));
}

void S3Fuse::fuse_create(QtFuseRequest *req, fuse_ino_t parent_ino, const char *name, mode_t mode) {
	QMetaObject::invokeMethod(parent, "fuse_create", Q_ARG(QtFuseRequest*,req), Q_ARG(fuse_ino_t,parent_ino), Q_ARG(const QByteArray&,name), Q_ARG(mode_t,mode));
}

void S3Fuse::fuse_read(QtFuseRequest *req, fuse_ino_t ino, size_t size, off_t offset) {
	QMetaObject::invokeMethod(parent, "fuse_read", Q_ARG(QtFuseRequest*,req), Q_ARG(fuse_ino_t,ino), Q_ARG(size_t,size), Q_ARG(off_t,offset));
}

void S3Fuse::fuse_write(QtFuseRequest *req, fuse_ino_t ino, const QByteArray &buf, off_t offset) {
	QMetaObject::invokeMethod(parent, "fuse_write", Q_ARG(QtFuseRequest*,req), Q_ARG(fuse_ino_t,ino), Q_ARG(const QByteArray&,buf), Q_ARG(off_t,offset));
}

void S3Fuse::fuse_write_buf(QtFuseRequest *req, fuse_ino_t ino, struct fuse_bufvec *bufv, off_t off) {
	struct fuse_bufvec dst = FUSE_BUFVEC_INIT(fuse_buf_size(bufv));
	QByteArray dst_buf;
	dst_buf.resize(fuse_buf_size(bufv));
	dst.buf[0].mem = dst_buf.data();
	ssize_t res = fuse_buf_copy(&dst, bufv, FUSE_BUF_NO_SPLICE);

	if (res < 0) {
		req->error(-res);
		return;
	}

	dst_buf.resize(res);
	fuse_write(req, ino, dst_buf, off);
}

void S3Fuse::fuse_getxattr(QtFuseRequest *req, fuse_ino_t, const QByteArray &, size_t) {
	req->error(ENOTSUP); // just return ENOSYS here to avoid log full of "getxattr not impl"
}

