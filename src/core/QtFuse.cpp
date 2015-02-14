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

#define QTFUSE_OBJ_FROM_REQ() QtFuse *c = (QtFuse*)fuse_req_userdata(req)
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
	c->fuse_lookup(QTFUSE_REQ(), parent, name);
}

void QtFuse::fuse_lookup(QtFuseRequest *req, fuse_ino_t, const QByteArray &) {
	QTFUSE_NOT_IMPL(ENOENT);
}

void QtFuse::priv_qtfuse_forget(fuse_req_t req, fuse_ino_t ino, unsigned long nlookup) {
	QTFUSE_OBJ_FROM_REQ();
	c->fuse_forget(QTFUSE_REQ(), ino, nlookup);
}

void QtFuse::fuse_forget(QtFuseRequest *req, fuse_ino_t node, unsigned long nlookup) {
	qDebug("TODO forget(%ld, %ld)", node, nlookup);
	req->none();
}

void QtFuse::priv_qtfuse_getattr(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi) {
	QTFUSE_OBJ_FROM_REQ();
	c->fuse_getattr(QTFUSE_REQ_FI(), ino);
}

void QtFuse::fuse_getattr(QtFuseRequest *req, fuse_ino_t) {
//	req->attr(node->getAttr());
	QTFUSE_NOT_IMPL(ENOSYS);
}

void QtFuse::priv_qtfuse_setattr(fuse_req_t req, fuse_ino_t ino, struct stat *attr, int to_set, struct fuse_file_info *fi) {
	QTFUSE_OBJ_FROM_REQ();
	auto _req = QTFUSE_REQ_FI();
	_req->setAttr(attr);
	c->fuse_setattr(_req, ino, to_set);
}

void QtFuse::fuse_setattr(QtFuseRequest *req, fuse_ino_t, int) {
	QTFUSE_NOT_IMPL(ENOSYS);
}

void QtFuse::priv_qtfuse_readlink(fuse_req_t req, fuse_ino_t ino) {
	QTFUSE_OBJ_FROM_REQ();
	c->fuse_readlink(QTFUSE_REQ(), ino);
}

void QtFuse::fuse_readlink(QtFuseRequest *req, fuse_ino_t ) {
	QTFUSE_NOT_IMPL(ENOSYS);
}

void QtFuse::priv_qtfuse_mknod(fuse_req_t req, fuse_ino_t parent, const char *name, mode_t mode, dev_t rdev) {
	QTFUSE_OBJ_FROM_REQ();
	c->fuse_mknod(QTFUSE_REQ(), parent, name, mode, rdev);
}

void QtFuse::fuse_mknod(QtFuseRequest *req, fuse_ino_t, const QByteArray&, mode_t, dev_t) {
	QTFUSE_NOT_IMPL(EPERM);
}

void QtFuse::priv_qtfuse_mkdir(fuse_req_t req, fuse_ino_t parent, const char *name, mode_t mode) {
	QTFUSE_OBJ_FROM_REQ();
	c->fuse_mkdir(QTFUSE_REQ(), parent, name, mode);
}

void QtFuse::fuse_mkdir(QtFuseRequest *req, fuse_ino_t, const QByteArray&, int) {
	QTFUSE_NOT_IMPL(ENOSYS);
}

void QtFuse::priv_qtfuse_unlink(fuse_req_t req, fuse_ino_t parent, const char *name) {
	QTFUSE_OBJ_FROM_REQ();
	c->fuse_unlink(QTFUSE_REQ(), parent, name);
}

void QtFuse::fuse_unlink(QtFuseRequest *req, fuse_ino_t, const QByteArray&) {
	QTFUSE_NOT_IMPL(ENOSYS);
}

void QtFuse::priv_qtfuse_rmdir(fuse_req_t req, fuse_ino_t parent, const char *name) {
	QTFUSE_OBJ_FROM_REQ();
	c->fuse_rmdir(QTFUSE_REQ(), parent, name);
}

void QtFuse::fuse_rmdir(QtFuseRequest *req, fuse_ino_t, const QByteArray &) {
	QTFUSE_NOT_IMPL(ENOSYS);
}

void QtFuse::priv_qtfuse_symlink(fuse_req_t req, const char *link, fuse_ino_t parent, const char *name) {
	QTFUSE_OBJ_FROM_REQ();
	c->fuse_symlink(QTFUSE_REQ(), link, parent, name);
}

void QtFuse::fuse_symlink(QtFuseRequest *req, const QByteArray&, fuse_ino_t, const QByteArray&) {
	QTFUSE_NOT_IMPL(ENOSYS);
}

void QtFuse::priv_qtfuse_rename(fuse_req_t req, fuse_ino_t parent, const char *name, fuse_ino_t newparent, const char *newname) {
	QTFUSE_OBJ_FROM_REQ();
	c->fuse_rename(QTFUSE_REQ(), parent, name, newparent, newname);
}

void QtFuse::fuse_rename(QtFuseRequest *req, fuse_ino_t, const QByteArray&, fuse_ino_t, const QByteArray&) {
	QTFUSE_NOT_IMPL(ENOSYS);
}

void QtFuse::priv_qtfuse_link(fuse_req_t req, fuse_ino_t ino, fuse_ino_t newparent, const char *newname) {
	QTFUSE_OBJ_FROM_REQ();
//	if (S_ISDIR(ino->getAttr()->st_mode)) {
//		fuse_reply_err(req, EPERM); // "oldpath is a directory."
//		return;
//	}
	c->fuse_link(QTFUSE_REQ(), ino, newparent, newname);
}

void QtFuse::fuse_link(QtFuseRequest *req, fuse_ino_t, fuse_ino_t, const QByteArray&) {
	QTFUSE_NOT_IMPL(EPERM); // "The file system containing oldpath and newpath does not support the creation of hard links."
}

void QtFuse::priv_qtfuse_open(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi) {
	QTFUSE_OBJ_FROM_REQ();
	c->fuse_open(QTFUSE_REQ_FI(), ino);
}

void QtFuse::fuse_open(QtFuseRequest *req, fuse_ino_t) {
	QTFUSE_NOT_IMPL(ENOSYS);
}

void QtFuse::priv_qtfuse_read(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off, struct fuse_file_info *fi) {
	QTFUSE_OBJ_FROM_REQ();
	c->fuse_read(QTFUSE_REQ_FI(), ino, size, off);
}

void QtFuse::fuse_read(QtFuseRequest *req, fuse_ino_t, size_t, off_t) {
	QTFUSE_NOT_IMPL(ENOSYS);
}

void QtFuse::priv_qtfuse_write(fuse_req_t req, fuse_ino_t ino, const char *buf, size_t size, off_t off, struct fuse_file_info *fi) {
	QTFUSE_OBJ_FROM_REQ();
	c->fuse_write(QTFUSE_REQ_FI(), ino, QByteArray(buf, size), off);
}

void QtFuse::fuse_write(QtFuseRequest *req, fuse_ino_t, const QByteArray&, off_t) {
	QTFUSE_NOT_IMPL(ENOSYS);
}

void QtFuse::priv_qtfuse_flush(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi) {
	QTFUSE_OBJ_FROM_REQ();
	c->fuse_flush(QTFUSE_REQ_FI(), ino);
}

void QtFuse::fuse_flush(QtFuseRequest *req, fuse_ino_t) {
	QTFUSE_NOT_IMPL(ENOSYS);
}

void QtFuse::priv_qtfuse_release(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi) {
	QTFUSE_OBJ_FROM_REQ();
	c->fuse_release(QTFUSE_REQ_FI(), ino);
}

void QtFuse::fuse_release(QtFuseRequest *req, fuse_ino_t) {
	QTFUSE_NOT_IMPL(ENOSYS);
}

void QtFuse::priv_qtfuse_fsync(fuse_req_t req, fuse_ino_t ino, int datasync, struct fuse_file_info *fi) {
	QTFUSE_OBJ_FROM_REQ();
	c->fuse_fsync(QTFUSE_REQ_FI(), ino, datasync);
}

void QtFuse::fuse_fsync(QtFuseRequest *req, fuse_ino_t, int) {
	QTFUSE_NOT_IMPL(ENOSYS);
}

void QtFuse::priv_qtfuse_opendir(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi) {
	QTFUSE_OBJ_FROM_REQ();
	c->fuse_opendir(QTFUSE_REQ_FI(), ino);
}

void QtFuse::fuse_opendir(QtFuseRequest *req, fuse_ino_t) {
	QTFUSE_NOT_IMPL(ENOSYS);
}

void QtFuse::priv_qtfuse_readdir(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off, struct fuse_file_info *fi) {
	QTFUSE_OBJ_FROM_REQ();
	QtFuseRequest *_req = QTFUSE_REQ_FI();
	_req->prepareBuffer(size);
	c->fuse_readdir(_req, ino, off);
}

void QtFuse::fuse_readdir(QtFuseRequest *req, fuse_ino_t, off_t) {
	QTFUSE_NOT_IMPL(ENOSYS);
}

void QtFuse::priv_qtfuse_releasedir(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi) {
	QTFUSE_OBJ_FROM_REQ();
	c->fuse_releasedir(QTFUSE_REQ_FI(), ino);
}

void QtFuse::fuse_releasedir(QtFuseRequest *req, fuse_ino_t) {
	QTFUSE_NOT_IMPL(ENOSYS);
}

void QtFuse::priv_qtfuse_fsyncdir(fuse_req_t req, fuse_ino_t ino, int datasync, struct fuse_file_info *fi) {
	QTFUSE_OBJ_FROM_REQ();
	c->fuse_fsyncdir(QTFUSE_REQ_FI(), ino, datasync);
}

void QtFuse::fuse_fsyncdir(QtFuseRequest *req, fuse_ino_t, int) {
	QTFUSE_NOT_IMPL(ENOSYS);
}

void QtFuse::priv_qtfuse_statfs(fuse_req_t req, fuse_ino_t ino) {
	QTFUSE_OBJ_FROM_REQ();
	c->fuse_statfs(QTFUSE_REQ(), ino);
}

void QtFuse::fuse_statfs(QtFuseRequest *req, fuse_ino_t) {
	QTFUSE_NOT_IMPL(ENOSYS);
}

void QtFuse::priv_qtfuse_setxattr(fuse_req_t req, fuse_ino_t ino, const char *name, const char *value, size_t size, int flags) {
	QTFUSE_OBJ_FROM_REQ();
	c->fuse_setxattr(QTFUSE_REQ(), ino, name, value, size, flags);
}

void QtFuse::fuse_setxattr(QtFuseRequest *req, fuse_ino_t, const char *, const char *, size_t, int) {
	QTFUSE_NOT_IMPL(ENOTSUP);
}

void QtFuse::priv_qtfuse_getxattr(fuse_req_t req, fuse_ino_t ino, const char *name, size_t size) {
	QTFUSE_OBJ_FROM_REQ();
	c->fuse_getxattr(QTFUSE_REQ(), ino, QByteArray(name), size);
}

void QtFuse::fuse_getxattr(QtFuseRequest *req, fuse_ino_t, const QByteArray&, size_t) {
	QTFUSE_NOT_IMPL(ENOTSUP);
}

void QtFuse::priv_qtfuse_listxattr(fuse_req_t req, fuse_ino_t ino, size_t size) {
	QTFUSE_OBJ_FROM_REQ();
	c->fuse_listxattr(QTFUSE_REQ(), ino, size);
}

void QtFuse::fuse_listxattr(QtFuseRequest *req, fuse_ino_t, size_t) {
	QTFUSE_NOT_IMPL(ENOTSUP);
}

void QtFuse::priv_qtfuse_removexattr(fuse_req_t req, fuse_ino_t ino, const char *name) {
	QTFUSE_OBJ_FROM_REQ();
	c->fuse_removexattr(QTFUSE_REQ(), ino, name);
}

void QtFuse::fuse_removexattr(QtFuseRequest *req, fuse_ino_t, const char *) {
	QTFUSE_NOT_IMPL(ENOTSUP);
}

void QtFuse::priv_qtfuse_access(fuse_req_t req, fuse_ino_t ino, int mask) {
	QTFUSE_OBJ_FROM_REQ();
	c->fuse_access(QTFUSE_REQ(), ino, mask);
}

void QtFuse::fuse_access(QtFuseRequest *req, fuse_ino_t, int) {
	QTFUSE_NOT_IMPL(ENOSYS);
}

void QtFuse::priv_qtfuse_create(fuse_req_t req, fuse_ino_t parent, const char *name, mode_t mode, struct fuse_file_info *fi) {
	QTFUSE_OBJ_FROM_REQ();
	c->fuse_create(QTFUSE_REQ_FI(), parent, name, mode);
}

void QtFuse::fuse_create(QtFuseRequest *req, fuse_ino_t, const char*, mode_t) {
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
	c->fuse_write_buf(QTFUSE_REQ_FI(), ino, bufv, off);
}

void QtFuse::fuse_write_buf(QtFuseRequest *req, fuse_ino_t, struct fuse_bufvec *, off_t) {
	QTFUSE_NOT_IMPL(ENOSYS);
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
	c->fuse_forget_multi(QTFUSE_REQ(), count, forgets);
}

void QtFuse::fuse_forget_multi(QtFuseRequest *req, size_t, struct fuse_forget_data *) {
	req->none();
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
	if (!chan) {
		fuse_opt_free_args(&args);
		QCoreApplication::exit(1);
		return;
	}

	fuse = fuse_lowlevel_new(&args, &qtfuse_op, sizeof(qtfuse_op), this);
	fuse_opt_free_args(&args);
	if (fuse == NULL)
		return; // TODO: correctly unmount fuse

	fuse_session_add_chan(fuse, chan);

	struct fuse_buf fuse_buf;
	memset(&fuse_buf, 0, sizeof(fuse_buf));
	size_t bufsize = fuse_chan_bufsize(chan);
	char *fuse_buf_mem = (char *) malloc(bufsize);

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

