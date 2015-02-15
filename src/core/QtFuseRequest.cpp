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
#include "QtFuse.hpp"
#include "QtFuseRequest.hpp"
#include <QCoreApplication>
#include <QTimer>

#define CHECK_ANSWER() if (answered) return; answered = true; QTimer::singleShot(0,this,SLOT(deleteLater()))

QtFuseRequest::QtFuseRequest(fuse_req_t _req, QtFuse &_parent, struct fuse_file_info *_fi): parent(_parent) {
	Q_CHECK_PTR(_req);
//	moveToThread(QCoreApplication::instance()->thread());
	req = _req;
	if (_fi) fuse_fi = *_fi;
	answered = false;
	data_buf = NULL;
	buf_size = 0;
	buf_pos = 0;
}

QtFuseRequest::~QtFuseRequest() {
	if (data_buf != NULL) delete data_buf;
}

const struct fuse_ctx *QtFuseRequest::context() const {
	return fuse_req_ctx(req);
}

void QtFuseRequest::prepareBuffer(size_t size) {
	if (buf_size != 0)
		delete data_buf;
	data_buf = new char[size];
	buf_size = size;
	buf_pos = 0;
}

void QtFuseRequest::error(int err) {
	CHECK_ANSWER();
	fuse_reply_err(req, err);
}

void QtFuseRequest::none() {
	CHECK_ANSWER();
	fuse_reply_none(req);
}

void QtFuseRequest::entry(const struct stat*attr, int generation) {
	CHECK_ANSWER();
	struct fuse_entry_param e;
	memset(&e, 0, sizeof(struct fuse_entry_param));

	e.ino = attr->st_ino;
	e.generation = generation;
	e.attr = *attr;
	e.attr_timeout = 1;
	e.entry_timeout = 1;

	fuse_reply_entry(req, &e);
}

void QtFuseRequest::create(const struct stat *attr, const struct fuse_file_info *fi, int generation) {
	CHECK_ANSWER();

	struct fuse_entry_param e;
	memset(&e, 0, sizeof(struct fuse_entry_param));

	e.ino = attr->st_ino;
	e.generation = generation;
	e.attr = *attr;

	fuse_reply_create(req, &e, fi);
}

void QtFuseRequest::attr(const struct stat *attr, double attr_timeout) {
	CHECK_ANSWER();

	fuse_reply_attr(req, attr, attr_timeout);
}

void QtFuseRequest::readlink(const QByteArray &link) {
	CHECK_ANSWER();

	fuse_reply_readlink(req, link.constData());
}

void QtFuseRequest::open(const struct fuse_file_info *fi) {
	CHECK_ANSWER();

	fuse_reply_open(req, fi);
}

void QtFuseRequest::write(size_t count) {
	CHECK_ANSWER();

	fuse_reply_write(req, count);
}

void QtFuseRequest::buf(const QByteArray &data) {
	CHECK_ANSWER();

	fuse_reply_buf(req, data.constData(), data.size());
}

void QtFuseRequest::iov(const struct iovec *iov, int count) {
	CHECK_ANSWER();

	fuse_reply_iov(req, iov, count);
}

void QtFuseRequest::statfs(const struct statvfs *stbuf) {
	CHECK_ANSWER();

	fuse_reply_statfs(req, stbuf);
}

void QtFuseRequest::xattr(size_t count) {
	CHECK_ANSWER();

	fuse_reply_xattr(req, count);
}

void QtFuseRequest::lock(struct flock *lock) {
	CHECK_ANSWER();

	fuse_reply_lock(req, lock);
}

void QtFuseRequest::bmap(uint64_t idx) {
	CHECK_ANSWER();

	fuse_reply_bmap(req, idx);
}

bool QtFuseRequest::dir_add(const QByteArray &name, const struct stat *stbuf, off_t next_offset) {
	if (data_buf == NULL) return false;

	size_t len = fuse_add_direntry(req, NULL, 0, name.constData(), stbuf, 0);
	if (len > buf_size-buf_pos) return false; // not enough room

	buf_pos += fuse_add_direntry(req, data_buf+buf_pos, buf_size-buf_pos, name.constData(), stbuf, next_offset);

	return true;
}

void QtFuseRequest::dir_send() {
	CHECK_ANSWER();

	fuse_reply_buf(req, data_buf, buf_pos);
	delete data_buf;
	data_buf = NULL;
	buf_size = 0;
}

struct fuse_file_info *QtFuseRequest::fi() {
	return &fuse_fi;
}

void QtFuseRequest::setAttr(struct stat	*a) {
	memcpy(&fuse_attr, a, sizeof(struct stat));
}

struct stat *QtFuseRequest::attr() {
	return &fuse_attr;
}

