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
#include <signal.h>
#include <stdlib.h>
#include <QCoreApplication>
#if QT_VERSION < 0x050300
#include <contrib/QByteArrayList.hpp>
#endif

#define QTFUSE_OBJ_FROM_REQ() Q_CHECK_PTR(req); QtFuse *c = (QtFuse*)fuse_req_userdata(req); Q_CHECK_PTR(c)
#define QTFUSE_REQ() (new QtFuseRequest(req, *c))
#define QTFUSE_REQ_FI() (new QtFuseRequest(req, *c, fi))
#define QTFUSE_NOT_IMPL(e) qDebug("fuse: %s not implemented, returning " #e, __FUNCTION__); req->error(e)

void QtFuse::priv_qtfuse_init(void *userdata, struct fuse_conn_info *conn) {
	QtFuse *c = (QtFuse*) userdata;
	c->fuse_init(conn);
}

void QtFuse::fuse_init(struct fuse_conn_info *) {
	qDebug("QtFuse: init");
}

void QtFuse::priv_qtfuse_destroy(void *userdata) {
	QtFuse *c = (QtFuse*) userdata;
	c->fuse_destroy();
}

void QtFuse::fuse_destroy() {
	qDebug("QtFuse: destroy");
}

void QtFuse::priv_qtfuse_lookup(fuse_req_t req, fuse_ino_t parent, const char *name) {
	QTFUSE_OBJ_FROM_REQ();
	auto _req = QTFUSE_REQ();
	_req->fuse_ino = parent;
	_req->fuse_name = QByteArray(name); // null-terminated
	c->fuse_lookup(_req);
}

void QtFuse::fuse_lookup(QtFuseRequest *req) {
	QTFUSE_NOT_IMPL(ENOENT);
}

void QtFuse::priv_qtfuse_forget(fuse_req_t req, fuse_ino_t ino, unsigned long nlookup) {
	QTFUSE_OBJ_FROM_REQ();
	fuse_reply_none(req);
	c->fuse_forget(ino, nlookup);
}

void QtFuse::fuse_forget(fuse_ino_t node, unsigned long nlookup) {
	qDebug("TODO forget(%ld, %ld)", node, nlookup);
}

void QtFuse::priv_qtfuse_getattr(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi) {
	QTFUSE_OBJ_FROM_REQ();
	auto _req = QTFUSE_REQ_FI();
	_req->fuse_ino = ino;
	c->fuse_getattr(_req);
}

void QtFuse::fuse_getattr(QtFuseRequest *req) {
	QTFUSE_NOT_IMPL(ENOSYS);
}

void QtFuse::priv_qtfuse_setattr(fuse_req_t req, fuse_ino_t ino, struct stat *attr, int to_set, struct fuse_file_info *fi) {
	QTFUSE_OBJ_FROM_REQ();
	auto _req = QTFUSE_REQ_FI();
	_req->setAttr(attr);
	_req->fuse_ino = ino;
	_req->fuse_int = to_set;
	c->fuse_setattr(_req);
}

void QtFuse::fuse_setattr(QtFuseRequest *req) {
	QTFUSE_NOT_IMPL(ENOSYS);
}

void QtFuse::priv_qtfuse_readlink(fuse_req_t req, fuse_ino_t ino) {
	QTFUSE_OBJ_FROM_REQ();
	auto _req = QTFUSE_REQ();
	_req->fuse_ino = ino;
	c->fuse_readlink(_req);
}

void QtFuse::fuse_readlink(QtFuseRequest *req) {
	QTFUSE_NOT_IMPL(ENOSYS);
}

void QtFuse::priv_qtfuse_mknod(fuse_req_t req, fuse_ino_t parent, const char *name, mode_t mode, dev_t rdev) {
	QTFUSE_OBJ_FROM_REQ();
	auto _req = QTFUSE_REQ();
	_req->fuse_ino = parent;
	_req->fuse_name = QByteArray(name);
	_req->fuse_int = mode;
	_req->fuse_newino = rdev; // 64bits int, shouldn't be used for that but well...
	c->fuse_mknod(_req);
}

void QtFuse::fuse_mknod(QtFuseRequest *req) {
	QTFUSE_NOT_IMPL(EPERM);
}

void QtFuse::priv_qtfuse_mkdir(fuse_req_t req, fuse_ino_t parent, const char *name, mode_t mode) {
	QTFUSE_OBJ_FROM_REQ();
	auto _req = QTFUSE_REQ();
	_req->fuse_ino = parent;
	_req->fuse_name = QByteArray(name);
	_req->fuse_int = mode;
	c->fuse_mkdir(_req);
}

void QtFuse::fuse_mkdir(QtFuseRequest *req) {
	QTFUSE_NOT_IMPL(ENOSYS);
}

void QtFuse::priv_qtfuse_unlink(fuse_req_t req, fuse_ino_t parent, const char *name) {
	QTFUSE_OBJ_FROM_REQ();
	auto _req = QTFUSE_REQ();
	_req->fuse_ino = parent;
	_req->fuse_name = name;
	c->fuse_unlink(_req);
}

void QtFuse::fuse_unlink(QtFuseRequest *req) {
	QTFUSE_NOT_IMPL(ENOSYS);
}

void QtFuse::priv_qtfuse_rmdir(fuse_req_t req, fuse_ino_t parent, const char *name) {
	QTFUSE_OBJ_FROM_REQ();
	auto _req = QTFUSE_REQ();
	_req->fuse_ino = parent;
	_req->fuse_name = name;
	c->fuse_rmdir(_req);
}

void QtFuse::fuse_rmdir(QtFuseRequest *req) {
	QTFUSE_NOT_IMPL(ENOSYS);
}

void QtFuse::priv_qtfuse_symlink(fuse_req_t req, const char *link, fuse_ino_t parent, const char *name) {
	QTFUSE_OBJ_FROM_REQ();
	auto _req = QTFUSE_REQ();
	_req->fuse_ino = parent;
	_req->fuse_name = QByteArray(name);
	_req->fuse_value = QByteArray(link);
	c->fuse_symlink(_req);
}

void QtFuse::fuse_symlink(QtFuseRequest *req) {
	QTFUSE_NOT_IMPL(ENOSYS);
}

void QtFuse::priv_qtfuse_rename(fuse_req_t req, fuse_ino_t parent, const char *name, fuse_ino_t newparent, const char *newname) {
	QTFUSE_OBJ_FROM_REQ();
	auto _req = QTFUSE_REQ();
	_req->fuse_ino = parent;
	_req->fuse_newino = newparent;
	_req->fuse_name = QByteArray(name);
	_req->fuse_value = QByteArray(newname);
	c->fuse_rename(_req);
}

void QtFuse::fuse_rename(QtFuseRequest *req) {
	QTFUSE_NOT_IMPL(ENOSYS);
}

void QtFuse::priv_qtfuse_link(fuse_req_t req, fuse_ino_t ino, fuse_ino_t newparent, const char *newname) {
	QTFUSE_OBJ_FROM_REQ();
	auto _req = QTFUSE_REQ();
	_req->fuse_ino = ino;
	_req->fuse_newino = newparent;
	_req->fuse_value = QByteArray(newname);
	c->fuse_link(_req);
}

void QtFuse::fuse_link(QtFuseRequest *req) {
	QTFUSE_NOT_IMPL(EPERM); // "The file system containing oldpath and newpath does not support the creation of hard links."
}

void QtFuse::priv_qtfuse_open(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi) {
	QTFUSE_OBJ_FROM_REQ();
	auto _req = QTFUSE_REQ_FI();
	_req->fuse_ino = ino;
	c->fuse_open(_req);
}

void QtFuse::fuse_open(QtFuseRequest *req) {
	QTFUSE_NOT_IMPL(ENOSYS);
}

void QtFuse::priv_qtfuse_read(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off, struct fuse_file_info *fi) {
	QTFUSE_OBJ_FROM_REQ();
	auto _req = QTFUSE_REQ_FI();
	_req->fuse_ino = ino;
	_req->fuse_size = size;
	_req->fuse_offset = off;
	c->fuse_read(_req);
}

void QtFuse::fuse_read(QtFuseRequest *req) {
	QTFUSE_NOT_IMPL(ENOSYS);
}

void QtFuse::priv_qtfuse_write(fuse_req_t req, fuse_ino_t ino, const char *buf, size_t size, off_t off, struct fuse_file_info *fi) {
	QTFUSE_OBJ_FROM_REQ();
	auto _req = QTFUSE_REQ_FI();
	_req->fuse_ino = ino;
	_req->fuse_value = QByteArray(buf, size);
	_req->fuse_offset = off;
	c->fuse_write(_req);
}

void QtFuse::fuse_write(QtFuseRequest *req) {
	QTFUSE_NOT_IMPL(ENOSYS);
}

void QtFuse::priv_qtfuse_flush(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi) {
	QTFUSE_OBJ_FROM_REQ();
	auto _req = QTFUSE_REQ_FI();
	_req->fuse_ino = ino;
	c->fuse_flush(_req);
}

void QtFuse::fuse_flush(QtFuseRequest *req) {
	QTFUSE_NOT_IMPL(ENOSYS);
}

void QtFuse::priv_qtfuse_release(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi) {
	QTFUSE_OBJ_FROM_REQ();
	auto _req = QTFUSE_REQ_FI();
	_req->fuse_ino = ino;
	c->fuse_release(_req);
}

void QtFuse::fuse_release(QtFuseRequest *req) {
	QTFUSE_NOT_IMPL(ENOSYS);
}

void QtFuse::priv_qtfuse_fsync(fuse_req_t req, fuse_ino_t ino, int datasync, struct fuse_file_info *fi) {
	QTFUSE_OBJ_FROM_REQ();
	auto _req = QTFUSE_REQ_FI();
	_req->fuse_ino = ino;
	_req->fuse_int = datasync;
	c->fuse_fsync(_req);
}

void QtFuse::fuse_fsync(QtFuseRequest *req) {
	QTFUSE_NOT_IMPL(ENOSYS);
}

void QtFuse::priv_qtfuse_opendir(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi) {
	QTFUSE_OBJ_FROM_REQ();
	auto _req = QTFUSE_REQ_FI();
	_req->fuse_ino = ino;
	c->fuse_opendir(_req);
}

void QtFuse::fuse_opendir(QtFuseRequest *req) {
	QTFUSE_NOT_IMPL(ENOSYS);
}

void QtFuse::priv_qtfuse_readdir(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off, struct fuse_file_info *fi) {
	QTFUSE_OBJ_FROM_REQ();
	QtFuseRequest *_req = QTFUSE_REQ_FI();
	_req->prepareBuffer(size);
	_req->fuse_ino = ino;
	_req->fuse_offset = off;
	c->fuse_readdir(_req);
}

void QtFuse::fuse_readdir(QtFuseRequest *req) {
	QTFUSE_NOT_IMPL(ENOSYS);
}

void QtFuse::priv_qtfuse_releasedir(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi) {
	QTFUSE_OBJ_FROM_REQ();
	auto _req = QTFUSE_REQ_FI();
	_req->fuse_ino = ino;
	c->fuse_releasedir(_req);
}

void QtFuse::fuse_releasedir(QtFuseRequest *req) {
	QTFUSE_NOT_IMPL(ENOSYS);
}

void QtFuse::priv_qtfuse_fsyncdir(fuse_req_t req, fuse_ino_t ino, int datasync, struct fuse_file_info *fi) {
	QTFUSE_OBJ_FROM_REQ();
	auto _req = QTFUSE_REQ_FI();
	_req->fuse_ino = ino;
	_req->fuse_int = datasync;
	c->fuse_fsyncdir(_req);
}

void QtFuse::fuse_fsyncdir(QtFuseRequest *req) {
	QTFUSE_NOT_IMPL(ENOSYS);
}

void QtFuse::priv_qtfuse_statfs(fuse_req_t req, fuse_ino_t ino) {
	QTFUSE_OBJ_FROM_REQ();
	auto _req = QTFUSE_REQ();
	_req->fuse_ino = ino;
	c->fuse_statfs(_req);
}

void QtFuse::fuse_statfs(QtFuseRequest *req) {
	QTFUSE_NOT_IMPL(ENOSYS);
}

void QtFuse::priv_qtfuse_setxattr(fuse_req_t req, fuse_ino_t ino, const char *name, const char *value, size_t size, int flags) {
	QTFUSE_OBJ_FROM_REQ();
	auto _req = QTFUSE_REQ();
	_req->fuse_ino = ino;
	_req->fuse_name = QByteArray(name);
	_req->fuse_value = QByteArray(value);
	_req->fuse_size = size;
	_req->fuse_int = flags;
	c->fuse_setxattr(_req);
}

void QtFuse::fuse_setxattr(QtFuseRequest *req) {
	QTFUSE_NOT_IMPL(ENOTSUP);
}

void QtFuse::priv_qtfuse_getxattr(fuse_req_t req, fuse_ino_t ino, const char *name, size_t size) {
	QTFUSE_OBJ_FROM_REQ();
	auto _req = QTFUSE_REQ();
	_req->fuse_ino = ino;
	_req->fuse_name = QByteArray(name);
	_req->fuse_size = size;
	c->fuse_getxattr(_req);
}

void QtFuse::fuse_getxattr(QtFuseRequest *req) {
	QTFUSE_NOT_IMPL(ENOTSUP);
}

void QtFuse::priv_qtfuse_listxattr(fuse_req_t req, fuse_ino_t ino, size_t size) {
	QTFUSE_OBJ_FROM_REQ();
	auto _req = QTFUSE_REQ();
	_req->fuse_ino = ino;
	_req->fuse_size = size;
	c->fuse_listxattr(_req);
}

void QtFuse::fuse_listxattr(QtFuseRequest *req) {
	QTFUSE_NOT_IMPL(ENOTSUP);
}

void QtFuse::priv_qtfuse_removexattr(fuse_req_t req, fuse_ino_t ino, const char *name) {
	QTFUSE_OBJ_FROM_REQ();
	auto _req = QTFUSE_REQ();
	_req->fuse_ino = ino;
	_req->fuse_name = QByteArray(name);
	c->fuse_removexattr(_req);
}

void QtFuse::fuse_removexattr(QtFuseRequest *req) {
	QTFUSE_NOT_IMPL(ENOTSUP);
}

void QtFuse::priv_qtfuse_access(fuse_req_t req, fuse_ino_t ino, int mask) {
	QTFUSE_OBJ_FROM_REQ();
	auto _req = QTFUSE_REQ();
	_req->fuse_ino = ino;
	_req->fuse_int = mask;
	c->fuse_access(_req);
}

void QtFuse::fuse_access(QtFuseRequest *req) {
	QTFUSE_NOT_IMPL(ENOSYS);
}

void QtFuse::priv_qtfuse_create(fuse_req_t req, fuse_ino_t parent, const char *name, mode_t mode, struct fuse_file_info *fi) {
	QTFUSE_OBJ_FROM_REQ();
	auto _req = QTFUSE_REQ_FI();
	_req->fuse_ino = parent;
	_req->fuse_name = QByteArray(name);
	_req->fuse_int = mode;
	c->fuse_create(_req);
}

void QtFuse::fuse_create(QtFuseRequest *req) {
	QTFUSE_NOT_IMPL(ENOSYS);
}

void QtFuse::priv_qtfuse_getlk(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi, struct flock *lock) {
	QTFUSE_OBJ_FROM_REQ();
	c->fuse_getlk(QTFUSE_REQ_FI(), ino, lock);
}

void QtFuse::fuse_getlk(QtFuseRequest *req, fuse_ino_t, struct flock*) {
	QTFUSE_NOT_IMPL(ENOSYS);
}

void QtFuse::priv_qtfuse_setlk(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi, struct flock *lock, int sleep) {
	QTFUSE_OBJ_FROM_REQ();
	c->fuse_setlk(QTFUSE_REQ_FI(), ino, lock, sleep);
}

void QtFuse::fuse_setlk(QtFuseRequest *req, fuse_ino_t, struct flock*, int) {
	QTFUSE_NOT_IMPL(ENOSYS);
}

void QtFuse::priv_qtfuse_bmap(fuse_req_t req, fuse_ino_t ino, size_t blocksize, uint64_t idx) {
	QTFUSE_OBJ_FROM_REQ();
	c->fuse_bmap(QTFUSE_REQ(), ino, blocksize, idx);
}

void QtFuse::fuse_bmap(QtFuseRequest *req, fuse_ino_t, size_t, uint64_t) {
	QTFUSE_NOT_IMPL(ENOSYS);
}

void QtFuse::priv_qtfuse_ioctl(fuse_req_t req, fuse_ino_t ino, int cmd, void *arg, struct fuse_file_info *fi, unsigned flags, const void *in_buf, size_t in_bufsz, size_t out_bufsz) {
	QTFUSE_OBJ_FROM_REQ();
	c->fuse_ioctl(QTFUSE_REQ_FI(), ino, cmd, arg, flags, in_buf, in_bufsz, out_bufsz);
}

void QtFuse::fuse_ioctl(QtFuseRequest *req, fuse_ino_t, int, void *, unsigned, const void *, size_t, size_t) {
	QTFUSE_NOT_IMPL(ENOSYS);
}

void QtFuse::priv_qtfuse_poll(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi, struct fuse_pollhandle *ph) {
	QTFUSE_OBJ_FROM_REQ();
	c->fuse_poll(QTFUSE_REQ_FI(), ino, ph);
}

void QtFuse::fuse_poll(QtFuseRequest *req, fuse_ino_t, struct fuse_pollhandle*) {
	QTFUSE_NOT_IMPL(ENOSYS);
}

void QtFuse::priv_qtfuse_write_buf(fuse_req_t req, fuse_ino_t ino, struct fuse_bufvec *bufv, off_t off, struct fuse_file_info *fi) {
	QTFUSE_OBJ_FROM_REQ();
	auto _req = QTFUSE_REQ_FI();

	struct fuse_bufvec dst = FUSE_BUFVEC_INIT(fuse_buf_size(bufv));
	QByteArray dst_buf;
	dst_buf.resize(fuse_buf_size(bufv));
	dst.buf[0].mem = dst_buf.data();
	ssize_t res = fuse_buf_copy(&dst, bufv, FUSE_BUF_NO_SPLICE);

	if (res < 0) {
		_req->error(-res);
		return;
	}

	dst_buf.resize(res);

	_req->fuse_ino = ino;
	_req->fuse_value = dst_buf;
	_req->fuse_offset = off;
	c->fuse_write(_req);
}

void QtFuse::priv_qtfuse_retrieve_reply(fuse_req_t req, void *cookie, fuse_ino_t ino, off_t offset, struct fuse_bufvec *bufv) {
	QTFUSE_OBJ_FROM_REQ();
	c->fuse_retrieve_reply(QTFUSE_REQ(), cookie, ino, offset, bufv);
}

void QtFuse::fuse_retrieve_reply(QtFuseRequest *req, void*, fuse_ino_t, off_t, struct fuse_bufvec *) {
	req->none();
}

void QtFuse::priv_qtfuse_forget_multi(fuse_req_t req, size_t count, struct fuse_forget_data *forgets) {
	QTFUSE_OBJ_FROM_REQ();
	fuse_reply_none(req);
	c->fuse_forget_multi(count, forgets);
}

void QtFuse::fuse_forget_multi(size_t count, struct fuse_forget_data *forgets) {
	for(size_t i = 0; i < count; i++)
		fuse_forget(forgets[i].ino, forgets[i].nlookup);
}

void QtFuse::priv_qtfuse_flock(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi, int op) {
	QTFUSE_OBJ_FROM_REQ();
	c->fuse_flock(QTFUSE_REQ_FI(), ino, op);
}

void QtFuse::fuse_flock(QtFuseRequest *req, fuse_ino_t, int) {
	QTFUSE_NOT_IMPL(ENOSYS);
}

void QtFuse::priv_qtfuse_fallocate(fuse_req_t req, fuse_ino_t ino, int mode, off_t offset, off_t length, struct fuse_file_info *fi) {
	QTFUSE_OBJ_FROM_REQ();
	c->fuse_fallocate(QTFUSE_REQ_FI(), ino, mode, offset, length);
}

void QtFuse::fuse_fallocate(QtFuseRequest *req, fuse_ino_t, int, off_t, off_t) {
	QTFUSE_NOT_IMPL(ENOSYS);
}

static bool signals_set = false;
struct fuse_lowlevel_ops QtFuse::qtfuse_op = {
	priv_qtfuse_init,
	priv_qtfuse_destroy,
	priv_qtfuse_lookup,
	priv_qtfuse_forget,
	priv_qtfuse_getattr,
	priv_qtfuse_setattr,
	priv_qtfuse_readlink,
	priv_qtfuse_mknod,
	priv_qtfuse_mkdir,
	priv_qtfuse_unlink,
	priv_qtfuse_rmdir,
	priv_qtfuse_symlink,
	priv_qtfuse_rename,
	priv_qtfuse_link,
	priv_qtfuse_open,
	priv_qtfuse_read,
	priv_qtfuse_write,
	priv_qtfuse_flush,
	priv_qtfuse_release,
	priv_qtfuse_fsync,
	priv_qtfuse_opendir,
	priv_qtfuse_readdir,
	priv_qtfuse_releasedir,
	priv_qtfuse_fsyncdir,
	priv_qtfuse_statfs,
	priv_qtfuse_setxattr,
	priv_qtfuse_getxattr,
	priv_qtfuse_listxattr,
	priv_qtfuse_removexattr,
	priv_qtfuse_access,
	priv_qtfuse_create,
	priv_qtfuse_getlk,
	priv_qtfuse_setlk,
	priv_qtfuse_bmap,
	priv_qtfuse_ioctl,
	priv_qtfuse_poll,
	priv_qtfuse_write_buf,
	priv_qtfuse_retrieve_reply,
	priv_qtfuse_forget_multi,
	priv_qtfuse_flock,
	priv_qtfuse_fallocate,
};

static void exit_handler(int) {
	QCoreApplication::quit();
}

static int set_one_signal_handler(int sig, void (*handler)(int)) {
	struct sigaction sa;

	memset(&sa, 0, sizeof(struct sigaction));
	sa.sa_handler = handler;
	sigemptyset(&(sa.sa_mask));
	sa.sa_flags = 0;

	return sigaction(sig, &sa, NULL);
}

void QtFuse::run() {
	fuse_cleaned = false;

	QByteArrayList o = opts.split(',');
	o.append("default_permissions");
	char *argv[] = { strdup("fuse"), strdup("-f"), strdup(mp.data()), strdup("-o"), strdup(o.join(',').data()) }; // hard_remove ?
	int argc = 5;

	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
	int foreground;
	int multithreaded;
	int res;

	res = fuse_parse_cmdline(&args, &mountpoint, &multithreaded, &foreground);
	if (res == -1) {
		fuse_opt_free_args(&args);
		QCoreApplication::exit(1);
		return;
	}

	chan = fuse_mount(mountpoint, &args);
	Q_CHECK_PTR(chan);

	fuse = fuse_lowlevel_new(&args, &qtfuse_op, sizeof(qtfuse_op), this);
	Q_CHECK_PTR(fuse);
	fuse_opt_free_args(&args);

	fuse_session_add_chan(fuse, chan);

	struct fuse_buf fuse_buf;
	memset(&fuse_buf, 0, sizeof(fuse_buf));
	size_t bufsize = fuse_chan_bufsize(chan);
	char *fuse_buf_mem = (char *) malloc(bufsize);
	Q_CHECK_PTR(fuse_buf_mem);

	ready();

	while (!fuse_session_exited(fuse)) {
		tmpch = chan;
		fuse_buf.size = bufsize;
		fuse_buf.mem = fuse_buf_mem;
		fuse_buf.flags = (enum fuse_buf_flags)0;

		res = fuse_session_receive_buf(fuse, &fuse_buf, &tmpch);
//		res = fuse_chan_recv_buf(&tmpch, &fuse_buf, bufsize);
		if (res == -EINTR)
			continue;
		if (res <= 0)
			break;
		//fuse_session_process(fuse, fuse_buf, fuse_buf_len, tmpch);
		fuse_session_process_buf(fuse, &fuse_buf, tmpch);
	}

//	free(fuse_buf);
	fuse_session_reset(fuse);

	fuse_unmount(mountpoint, chan);
	fuse_session_destroy(fuse);

	fuse_cleaned = true;

	for(int i = 0; i < argc; i++) free(argv[i]);
}

void QtFuse::prepare() {
	// Register various types for safety
	qRegisterMetaType<QtFuse*>("QtFuse*");
	qRegisterMetaType<QtFuseRequest*>("QtFuseRequest*");
	qRegisterMetaType<fuse_ino_t>("fuse_ino_t");
	qRegisterMetaType<mode_t>("mode_t");
	qRegisterMetaType<size_t>("size_t");
	qRegisterMetaType<off_t>("off_t");
	qRegisterMetaType<struct fuse_file_info*>("struct fuse_file_info*");
	qRegisterMetaType<struct flock *>("struct flock*");
	qRegisterMetaType<struct stat *>("struct stat*");
	qRegisterMetaType<struct fuse_bufvec*>("struct fuse_bufvec*");
}

QtFuse::QtFuse(const QByteArray &_mp, const QByteArray &_src, const QByteArray &_opts, QObject *parent): QThread(parent) {
	// required in some cases by Qt
	prepare();

	mp = _mp;
	src = _src;
	opts = _opts;
	// so we can catch ^C and killed processes, make those signal call QCoreApplication::quit()
	if (!signals_set) {
		signals_set = true;
		set_one_signal_handler(SIGHUP, exit_handler);
		set_one_signal_handler(SIGINT, exit_handler);
		set_one_signal_handler(SIGTERM, exit_handler);
	}
	connect(QCoreApplication::instance(), SIGNAL(aboutToQuit()), this, SLOT(quit()));
}

void QtFuse::quit() {
	if (fuse_cleaned) return;
	// attempt to stop fuse thread (we might die here)
	terminate();
	// teardown fuse
	fuse_session_reset(fuse);
	fuse_unmount(mountpoint, chan);
	fuse_session_destroy(fuse);
	fuse_cleaned = true;
}

QtFuse::~QtFuse() {
	quit();
}

void QtFuse::start() {
	connect(this, &QThread::finished, QCoreApplication::instance(), &QCoreApplication::quit);
	QThread::start();
}

