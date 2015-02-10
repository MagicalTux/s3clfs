#include "S3Fuse.hpp"
#include "S3FS.hpp"
#include "QtFuseRequest.hpp"

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

S3Fuse::S3Fuse(const QByteArray &bucket, const QByteArray &path, const QByteArray &opts, S3FS *_parent): QtFuse(path, bucket, opts) {
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
	parent->fuse_lookup(req, ino, path);
}

void S3Fuse::fuse_forget(QtFuseRequest *req, fuse_ino_t ino, unsigned long nlookup) {
	parent->fuse_forget(req, ino, nlookup);
}

void S3Fuse::fuse_setattr(QtFuseRequest *req, fuse_ino_t node, struct stat *attr, int to_set, struct fuse_file_info *fi) {
	parent->fuse_setattr(req, node, attr, to_set, fi);
}

void S3Fuse::fuse_getattr(QtFuseRequest *req, fuse_ino_t ino, struct fuse_file_info *fi) {
	parent->fuse_getattr(req, ino, fi);
}

void S3Fuse::fuse_unlink(QtFuseRequest *req, fuse_ino_t parent_ino, const QByteArray &name) {
	parent->fuse_unlink(req, parent_ino, name);
}

void S3Fuse::fuse_readlink(QtFuseRequest *req, fuse_ino_t node) {
	parent->fuse_readlink(req, node);
}

void S3Fuse::fuse_mkdir(QtFuseRequest *req, fuse_ino_t parent_ino, const QByteArray &name, int mode) {
	parent->fuse_mkdir(req, parent_ino, name, mode);
}

void S3Fuse::fuse_rmdir(QtFuseRequest *req, fuse_ino_t parent_ino, const QByteArray &name) {
	parent->fuse_rmdir(req, parent_ino, name);
}

void S3Fuse::fuse_symlink(QtFuseRequest *req, const QByteArray &link, fuse_ino_t parent_ino, const QByteArray &name) {
	parent->fuse_symlink(req, link, parent_ino, name);
}

void S3Fuse::fuse_rename(QtFuseRequest *req, fuse_ino_t parent_ino, const QByteArray &name, fuse_ino_t newparent, const QByteArray &newname) {
	parent->fuse_rename(req, parent_ino, name, newparent, newname);
}

void S3Fuse::fuse_link(QtFuseRequest *req, fuse_ino_t ino, fuse_ino_t newparent, const QByteArray &newname) {
	parent->fuse_link(req, ino, newparent, newname);
}

void S3Fuse::fuse_flush(QtFuseRequest *req, fuse_ino_t ino, struct fuse_file_info *fi) {
	parent->fuse_flush(req, ino, fi);
}

void S3Fuse::fuse_release(QtFuseRequest *req, fuse_ino_t ino, struct fuse_file_info *fi) {
	parent->fuse_release(req, ino, fi);
}

void S3Fuse::fuse_open(QtFuseRequest *req, fuse_ino_t ino, struct fuse_file_info *fi) {
	parent->fuse_open(req, ino, fi);
}

void S3Fuse::fuse_opendir(QtFuseRequest *req, fuse_ino_t ino, struct fuse_file_info *fi) {
	parent->fuse_opendir(req, ino, fi);
}

void S3Fuse::fuse_readdir(QtFuseRequest *req, fuse_ino_t ino, off_t off, struct fuse_file_info *fi) {
	parent->fuse_readdir(req, ino, off, fi);
}

void S3Fuse::fuse_releasedir(QtFuseRequest *req, fuse_ino_t ino, struct fuse_file_info *fi) {
	parent->fuse_releasedir(req, ino, fi);
}

void S3Fuse::fuse_create(QtFuseRequest *req, fuse_ino_t parent_ino, const char *name, mode_t mode, struct fuse_file_info *fi) {
	parent->fuse_create(req, parent_ino, name, mode, fi);
}

void S3Fuse::fuse_read(QtFuseRequest *req, fuse_ino_t ino, size_t size, off_t offset, struct fuse_file_info *fi) {
	parent->fuse_read(req, ino, size, offset, fi);
}

void S3Fuse::fuse_write(QtFuseRequest *req, fuse_ino_t ino, const QByteArray &buf, off_t offset, struct fuse_file_info *fi) {
	parent->fuse_write(req, ino, buf, offset, fi);
}

void S3Fuse::fuse_write_buf(QtFuseRequest *req, fuse_ino_t ino, struct fuse_bufvec *bufv, off_t off, struct fuse_file_info *fi) {
	parent->fuse_write_buf(req, ino, bufv, off, fi);
}

