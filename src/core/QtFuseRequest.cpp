#include "QtFuse.hpp"
#include "QtFuseRequest.hpp"

QtFuseRequest::QtFuseRequest(fuse_req_t _req, QtFuse &_parent): QObject(&_parent), parent(_parent) {
	req = _req;
	answered = false;
	data_buf = NULL;
	buf_size = 0;
	buf_pos = 0;
}

QtFuseRequest::~QtFuseRequest() {
	if (data_buf != NULL) delete data_buf;
}

void QtFuseRequest::prepareBuffer(size_t size) {
	if (buf_size != 0)
		delete data_buf;
	data_buf = new char[size];
	buf_size = size;
	buf_pos = 0;
}

void QtFuseRequest::error(int err) {
	if (answered) return;
	answered = true;
	fuse_reply_err(req, err);
	deleteLater();
}

void QtFuseRequest::none() {
	if (answered) return;
	answered = true;

	fuse_reply_none(req);
	deleteLater();
}

void QtFuseRequest::entry(const struct stat*attr, int generation) {
	if (answered) return;
	answered = true;
	struct fuse_entry_param e;
	memset(&e, 0, sizeof(struct fuse_entry_param));

	e.ino = attr->st_ino;
	e.generation = generation;
	e.attr = *attr;

	fuse_reply_entry(req, &e);
	deleteLater();
}

void QtFuseRequest::create(const struct fuse_entry_param *e, const struct fuse_file_info *fi) {
	if (answered) return;
	answered = true;

	fuse_reply_create(req, e, fi);
	deleteLater();
}

void QtFuseRequest::attr(const struct stat *attr, double attr_timeout) {
	if (answered) return;
	answered = true;

	fuse_reply_attr(req, attr, attr_timeout);
	deleteLater();
}

void QtFuseRequest::readlink(const QString &link) {
	if (answered) return;
	answered = true;

	fuse_reply_readlink(req, link.toUtf8().constData());
	deleteLater();
}

void QtFuseRequest::open(const struct fuse_file_info *fi) {
	if (answered) return;
	answered = true;

	fuse_reply_open(req, fi);
	deleteLater();
}

void QtFuseRequest::write(size_t count) {
	if (answered) return;
	answered = true;

	fuse_reply_write(req, count);
	deleteLater();
}

void QtFuseRequest::buf(const QByteArray &data) {
	if (answered) return;
	answered = true;

	fuse_reply_buf(req, data.constData(), data.size());
	deleteLater();
}

void QtFuseRequest::iov(const struct iovec *iov, int count) {
	if (answered) return;
	answered = true;

	fuse_reply_iov(req, iov, count);
	deleteLater();
}

void QtFuseRequest::statfs(const struct statvfs *stbuf) {
	if (answered) return;
	answered = true;

	fuse_reply_statfs(req, stbuf);
	deleteLater();
}

void QtFuseRequest::xattr(size_t count) {
	if (answered) return;
	answered = true;

	fuse_reply_xattr(req, count);
	deleteLater();
}

void QtFuseRequest::lock(struct flock *lock) {
	if (answered) return;
	answered = true;

	fuse_reply_lock(req, lock);
	deleteLater();
}

void QtFuseRequest::bmap(uint64_t idx) {
	if (answered) return;
	answered = true;

	fuse_reply_bmap(req, idx);
	deleteLater();
}

bool QtFuseRequest::dir_add(const QString &name, const struct stat *stbuf, off_t next_offset) {
	if (data_buf == NULL) return false;

	size_t len = fuse_add_direntry(req, NULL, 0, name.toUtf8().constData(), stbuf, 0);
	if (len > buf_size-buf_pos) return false; // not enough room

	buf_pos += fuse_add_direntry(req, data_buf+buf_pos, buf_size-buf_pos, name.toUtf8().constData(), stbuf, next_offset);

	return true;
}

void QtFuseRequest::dir_send() {
	if (answered) return;
	answered = true;

	fuse_reply_buf(req, data_buf, buf_pos);
	delete data_buf;
	data_buf = NULL;
	buf_size = 0;
	deleteLater();
}

