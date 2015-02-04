#include "QtFuse.hpp"
#include "QtFuseRequest.hpp"
#include <signal.h>
#include <stdlib.h>
#include <QCoreApplication>

#define QTFUSE_OBJ_FROM_REQ() QtFuse *c = (QtFuse*)fuse_req_userdata(req)
#define QTFUSE_CHECK_INODE(ino) if (!c->inode_exists(ino)) { fuse_reply_err(req, EINVAL); return; }
#define QTFUSE_CHECK_INODE2(ino) if (!inode_exists(ino)) { req->error(EINVAL); return; }
#define QTFUSE_REQ() (new QtFuseRequest(req, *c))
#define QTFUSE_NOT_IMPL(e) qDebug("fuse: %s not implemented, returning " #e, __FUNCTION__); req->error(e)

bool QtFuse::inode_exists(fuse_ino_t ino) {
	return (ino == 1); // root
}

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
	QTFUSE_CHECK_INODE(parent);
	c->fuse_lookup(QTFUSE_REQ(), parent, name);
}

void QtFuse::fuse_lookup(QtFuseRequest *req, fuse_ino_t, const QByteArray &) {
	QTFUSE_NOT_IMPL(ENOENT);
}

void QtFuse::priv_qtfuse_forget(fuse_req_t req, fuse_ino_t ino, unsigned long nlookup) {
	QTFUSE_OBJ_FROM_REQ();
	QTFUSE_CHECK_INODE(ino);
	c->fuse_forget(QTFUSE_REQ(), ino, nlookup);
}

void QtFuse::fuse_forget(QtFuseRequest *req, fuse_ino_t node, unsigned long nlookup) {
	qDebug("TODO forget(%ld, %ld)", node, nlookup);
	req->none();
}

void QtFuse::priv_qtfuse_getattr(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi) {
	QTFUSE_OBJ_FROM_REQ();
	c->fuse_getattr(QTFUSE_REQ(), ino, fi);
}

void QtFuse::fuse_getattr(QtFuseRequest *req, fuse_ino_t, struct fuse_file_info *) {
//	req->attr(node->getAttr());
	QTFUSE_NOT_IMPL(ENOSYS);
}

void QtFuse::priv_qtfuse_setattr(fuse_req_t req, fuse_ino_t ino, struct stat *attr, int to_set, struct fuse_file_info *fi) {
	QTFUSE_OBJ_FROM_REQ();
	QTFUSE_CHECK_INODE(ino);
	c->fuse_setattr(QTFUSE_REQ(), ino, attr, to_set, fi);
}

void QtFuse::fuse_setattr(QtFuseRequest *req, fuse_ino_t, struct stat *, int, struct fuse_file_info *) {
	QTFUSE_NOT_IMPL(ENOSYS);
}

void QtFuse::priv_qtfuse_readlink(fuse_req_t req, fuse_ino_t ino) {
	QTFUSE_OBJ_FROM_REQ();
	QTFUSE_CHECK_INODE(ino);
	c->fuse_readlink(QTFUSE_REQ(), ino);
}

void QtFuse::fuse_readlink(QtFuseRequest *req, fuse_ino_t ) {
	QTFUSE_NOT_IMPL(ENOSYS);
}

void QtFuse::priv_qtfuse_mknod(fuse_req_t req, fuse_ino_t parent, const char *name, mode_t mode, dev_t rdev) {
	QTFUSE_OBJ_FROM_REQ();
	QTFUSE_CHECK_INODE(parent);
	c->fuse_mknod(QTFUSE_REQ(), parent, name, mode, rdev);
}

void QtFuse::fuse_mknod(QtFuseRequest *req, fuse_ino_t, const QByteArray&, mode_t, dev_t) {
	QTFUSE_NOT_IMPL(EPERM);
}

void QtFuse::priv_qtfuse_mkdir(fuse_req_t req, fuse_ino_t parent, const char *name, mode_t mode) {
	QTFUSE_OBJ_FROM_REQ();
	QTFUSE_CHECK_INODE(parent);
	c->fuse_mkdir(QTFUSE_REQ(), parent, name, mode);
}

void QtFuse::fuse_mkdir(QtFuseRequest *req, fuse_ino_t, const QByteArray&, int) {
	QTFUSE_NOT_IMPL(ENOSYS);
}

void QtFuse::priv_qtfuse_unlink(fuse_req_t req, fuse_ino_t parent, const char *name) {
	QTFUSE_OBJ_FROM_REQ();
	QTFUSE_CHECK_INODE(parent);
	c->fuse_unlink(QTFUSE_REQ(), parent, name);
}

void QtFuse::fuse_unlink(QtFuseRequest *req, fuse_ino_t, const QByteArray&) {
	QTFUSE_NOT_IMPL(ENOSYS);
}

void QtFuse::priv_qtfuse_rmdir(fuse_req_t req, fuse_ino_t parent, const char *name) {
	QTFUSE_OBJ_FROM_REQ();
	QTFUSE_CHECK_INODE(parent);
	c->fuse_rmdir(QTFUSE_REQ(), parent, name);
}

void QtFuse::fuse_rmdir(QtFuseRequest *req, fuse_ino_t, const QByteArray &) {
	QTFUSE_NOT_IMPL(ENOSYS);
}

void QtFuse::priv_qtfuse_symlink(fuse_req_t req, const char *link, fuse_ino_t parent, const char *name) {
	QTFUSE_OBJ_FROM_REQ();
	QTFUSE_CHECK_INODE(parent);
	c->fuse_symlink(QTFUSE_REQ(), link, parent, name);
}

void QtFuse::fuse_symlink(QtFuseRequest *req, const QByteArray&, fuse_ino_t, const QByteArray&) {
	QTFUSE_NOT_IMPL(ENOSYS);
}

void QtFuse::priv_qtfuse_rename(fuse_req_t req, fuse_ino_t parent, const char *name, fuse_ino_t newparent, const char *newname) {
	QTFUSE_OBJ_FROM_REQ();
	QTFUSE_CHECK_INODE(parent);
	if (parent != newparent)
		QTFUSE_CHECK_INODE(newparent);
	c->fuse_rename(QTFUSE_REQ(), parent, name, newparent, newname);
}

void QtFuse::fuse_rename(QtFuseRequest *req, fuse_ino_t, const QByteArray&, fuse_ino_t, const QByteArray&) {
	QTFUSE_NOT_IMPL(ENOSYS);
}

void QtFuse::priv_qtfuse_link(fuse_req_t req, fuse_ino_t ino, fuse_ino_t newparent, const char *newname) {
	QTFUSE_OBJ_FROM_REQ();
	QTFUSE_CHECK_INODE(ino);
//	if (S_ISDIR(ino->getAttr()->st_mode)) {
//		fuse_reply_err(req, EPERM); // "oldpath is a directory."
//		return;
//	}
	QTFUSE_CHECK_INODE(newparent);
	c->fuse_link(QTFUSE_REQ(), ino, newparent, newname);
}

void QtFuse::fuse_link(QtFuseRequest *req, fuse_ino_t, fuse_ino_t, const QByteArray&) {
	QTFUSE_NOT_IMPL(EPERM); // "The file system containing oldpath and newpath does not support the creation of hard links."
}

void QtFuse::priv_qtfuse_open(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi) {
	QTFUSE_OBJ_FROM_REQ();
	QTFUSE_CHECK_INODE(ino);
	c->fuse_open(QTFUSE_REQ(), ino, fi);
}

void QtFuse::fuse_open(QtFuseRequest *req, fuse_ino_t, struct fuse_file_info *) {
	QTFUSE_NOT_IMPL(ENOSYS);
}

void QtFuse::priv_qtfuse_read(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off, struct fuse_file_info *fi) {
	QTFUSE_OBJ_FROM_REQ();
	QTFUSE_CHECK_INODE(ino);
	c->fuse_read(QTFUSE_REQ(), ino, size, off, fi);
}

void QtFuse::fuse_read(QtFuseRequest *req, fuse_ino_t, size_t, off_t, struct fuse_file_info*) {
	QTFUSE_NOT_IMPL(ENOSYS);
}

void QtFuse::priv_qtfuse_write(fuse_req_t req, fuse_ino_t ino, const char *buf, size_t size, off_t off, struct fuse_file_info *fi) {
	QTFUSE_OBJ_FROM_REQ();
	QTFUSE_CHECK_INODE(ino);
	c->fuse_write(QTFUSE_REQ(), ino, buf, size, off, fi);
}

void QtFuse::fuse_write(QtFuseRequest *req, fuse_ino_t, const char *, size_t, off_t, struct fuse_file_info *) {
	QTFUSE_NOT_IMPL(ENOSYS);
}

void QtFuse::priv_qtfuse_flush(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi) {
	QTFUSE_OBJ_FROM_REQ();
	QTFUSE_CHECK_INODE(ino);
	c->fuse_flush(QTFUSE_REQ(), ino, fi);
}

void QtFuse::fuse_flush(QtFuseRequest *req, fuse_ino_t, struct fuse_file_info *) {
	QTFUSE_NOT_IMPL(ENOSYS);
}

void QtFuse::priv_qtfuse_release(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi) {
	QTFUSE_OBJ_FROM_REQ();
	QTFUSE_CHECK_INODE(ino);
	c->fuse_release(QTFUSE_REQ(), ino, fi);
}

void QtFuse::fuse_release(QtFuseRequest *req, fuse_ino_t, struct fuse_file_info *) {
	QTFUSE_NOT_IMPL(ENOSYS);
}

void QtFuse::priv_qtfuse_fsync(fuse_req_t req, fuse_ino_t ino, int datasync, struct fuse_file_info *fi) {
	QTFUSE_OBJ_FROM_REQ();
	QTFUSE_CHECK_INODE(ino);
	c->fuse_fsync(QTFUSE_REQ(), ino, datasync, fi);
}

void QtFuse::fuse_fsync(QtFuseRequest *req, fuse_ino_t, int, struct fuse_file_info*) {
	QTFUSE_NOT_IMPL(ENOSYS);
}

void QtFuse::priv_qtfuse_opendir(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi) {
	QTFUSE_OBJ_FROM_REQ();
	QTFUSE_CHECK_INODE(ino);
	c->fuse_opendir(QTFUSE_REQ(), ino, fi);
}

void QtFuse::fuse_opendir(QtFuseRequest *req, fuse_ino_t, struct fuse_file_info *) {
	QTFUSE_NOT_IMPL(ENOSYS);
}

void QtFuse::priv_qtfuse_readdir(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off, struct fuse_file_info *fi) {
	QTFUSE_OBJ_FROM_REQ();
	QTFUSE_CHECK_INODE(ino);
	QtFuseRequest *_req = QTFUSE_REQ();
	_req->prepareBuffer(size);
	c->fuse_readdir(_req, ino, off, fi);
}

void QtFuse::fuse_readdir(QtFuseRequest *req, fuse_ino_t, off_t, struct fuse_file_info *) {
	QTFUSE_NOT_IMPL(ENOSYS);
}

void QtFuse::priv_qtfuse_releasedir(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi) {
	QTFUSE_OBJ_FROM_REQ();
	QTFUSE_CHECK_INODE(ino);
	c->fuse_releasedir(QTFUSE_REQ(), ino, fi);
}

void QtFuse::fuse_releasedir(QtFuseRequest *req, fuse_ino_t, struct fuse_file_info *) {
	QTFUSE_NOT_IMPL(ENOSYS);
}

void QtFuse::priv_qtfuse_fsyncdir(fuse_req_t req, fuse_ino_t ino, int datasync, struct fuse_file_info *fi) {
	QTFUSE_OBJ_FROM_REQ();
	QTFUSE_CHECK_INODE(ino);
	c->fuse_fsyncdir(QTFUSE_REQ(), ino, datasync, fi);
}

void QtFuse::fuse_fsyncdir(QtFuseRequest *req, fuse_ino_t, int, struct fuse_file_info *) {
	QTFUSE_NOT_IMPL(ENOSYS);
}

void QtFuse::priv_qtfuse_statfs(fuse_req_t req, fuse_ino_t ino) {
	QTFUSE_OBJ_FROM_REQ();
	QTFUSE_CHECK_INODE(ino);
	c->fuse_statfs(QTFUSE_REQ(), ino);
}

void QtFuse::fuse_statfs(QtFuseRequest *req, fuse_ino_t) {
	QTFUSE_NOT_IMPL(ENOSYS);
}

void QtFuse::priv_qtfuse_setxattr(fuse_req_t req, fuse_ino_t ino, const char *name, const char *value, size_t size, int flags) {
	QTFUSE_OBJ_FROM_REQ();
	QTFUSE_CHECK_INODE(ino);
	c->fuse_setxattr(QTFUSE_REQ(), ino, name, value, size, flags);
}

void QtFuse::fuse_setxattr(QtFuseRequest *req, fuse_ino_t, const char *, const char *, size_t, int) {
	QTFUSE_NOT_IMPL(ENOTSUP);
}

void QtFuse::priv_qtfuse_getxattr(fuse_req_t req, fuse_ino_t ino, const char *name, size_t size) {
	QTFUSE_OBJ_FROM_REQ();
	QTFUSE_CHECK_INODE(ino);
	c->fuse_getxattr(QTFUSE_REQ(), ino, name, size);
}

void QtFuse::fuse_getxattr(QtFuseRequest *req, fuse_ino_t, const char*, size_t) {
	QTFUSE_NOT_IMPL(ENOTSUP);
}

void QtFuse::priv_qtfuse_listxattr(fuse_req_t req, fuse_ino_t ino, size_t size) {
	QTFUSE_OBJ_FROM_REQ();
	QTFUSE_CHECK_INODE(ino);
	c->fuse_listxattr(QTFUSE_REQ(), ino, size);
}

void QtFuse::fuse_listxattr(QtFuseRequest *req, fuse_ino_t, size_t) {
	QTFUSE_NOT_IMPL(ENOTSUP);
}

void QtFuse::priv_qtfuse_removexattr(fuse_req_t req, fuse_ino_t ino, const char *name) {
	QTFUSE_OBJ_FROM_REQ();
	QTFUSE_CHECK_INODE(ino);
	c->fuse_removexattr(QTFUSE_REQ(), ino, name);
}

void QtFuse::fuse_removexattr(QtFuseRequest *req, fuse_ino_t, const char *) {
	QTFUSE_NOT_IMPL(ENOTSUP);
}

void QtFuse::priv_qtfuse_access(fuse_req_t req, fuse_ino_t ino, int mask) {
	QTFUSE_OBJ_FROM_REQ();
	QTFUSE_CHECK_INODE(ino);
	c->fuse_access(QTFUSE_REQ(), ino, mask);
}

void QtFuse::fuse_access(QtFuseRequest *req, fuse_ino_t, int) {
	QTFUSE_NOT_IMPL(ENOSYS);
}

void QtFuse::priv_qtfuse_create(fuse_req_t req, fuse_ino_t parent, const char *name, mode_t mode, struct fuse_file_info *fi) {
	QTFUSE_OBJ_FROM_REQ();
	QTFUSE_CHECK_INODE(parent);
	c->fuse_create(QTFUSE_REQ(), parent, name, mode, fi);
}

void QtFuse::fuse_create(QtFuseRequest *req, fuse_ino_t, const char*, mode_t, struct fuse_file_info*) {
	QTFUSE_NOT_IMPL(ENOSYS);
}

void QtFuse::priv_qtfuse_getlk(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi, struct flock *lock) {
	QTFUSE_OBJ_FROM_REQ();
	QTFUSE_CHECK_INODE(ino);
	c->fuse_getlk(QTFUSE_REQ(), ino, fi, lock);
}

void QtFuse::fuse_getlk(QtFuseRequest *req, fuse_ino_t, struct fuse_file_info*, struct flock*) {
	QTFUSE_NOT_IMPL(ENOSYS);
}

void QtFuse::priv_qtfuse_setlk(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi, struct flock *lock, int sleep) {
	QTFUSE_OBJ_FROM_REQ();
	QTFUSE_CHECK_INODE(ino);
	c->fuse_setlk(QTFUSE_REQ(), ino, fi, lock, sleep);
}

void QtFuse::fuse_setlk(QtFuseRequest *req, fuse_ino_t, struct fuse_file_info*, struct flock*, int) {
	QTFUSE_NOT_IMPL(ENOSYS);
}

void QtFuse::priv_qtfuse_bmap(fuse_req_t req, fuse_ino_t ino, size_t blocksize, uint64_t idx) {
	QTFUSE_OBJ_FROM_REQ();
	QTFUSE_CHECK_INODE(ino);
	c->fuse_bmap(QTFUSE_REQ(), ino, blocksize, idx);
}

void QtFuse::fuse_bmap(QtFuseRequest *req, fuse_ino_t, size_t, uint64_t) {
	QTFUSE_NOT_IMPL(ENOSYS);
}

void QtFuse::priv_qtfuse_ioctl(fuse_req_t req, fuse_ino_t ino, int cmd, void *arg, struct fuse_file_info *fi, unsigned flags, const void *in_buf, size_t in_bufsz, size_t out_bufsz) {
	QTFUSE_OBJ_FROM_REQ();
	QTFUSE_CHECK_INODE(ino);
	c->fuse_ioctl(QTFUSE_REQ(), ino, cmd, arg, fi, flags, in_buf, in_bufsz, out_bufsz);
}

void QtFuse::fuse_ioctl(QtFuseRequest *req, fuse_ino_t, int, void *, struct fuse_file_info *, unsigned, const void *, size_t, size_t) {
	QTFUSE_NOT_IMPL(ENOSYS);
}

void QtFuse::priv_qtfuse_poll(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi, struct fuse_pollhandle *ph) {
	QTFUSE_OBJ_FROM_REQ();
	QTFUSE_CHECK_INODE(ino);
	c->fuse_poll(QTFUSE_REQ(), ino, fi, ph);
}

void QtFuse::fuse_poll(QtFuseRequest *req, fuse_ino_t, struct fuse_file_info*, struct fuse_pollhandle*) {
	QTFUSE_NOT_IMPL(ENOSYS);
}

void QtFuse::priv_qtfuse_write_buf(fuse_req_t req, fuse_ino_t ino, struct fuse_bufvec *bufv, off_t off, struct fuse_file_info *fi) {
	QTFUSE_OBJ_FROM_REQ();
	QTFUSE_CHECK_INODE(ino);
	c->fuse_write_buf(QTFUSE_REQ(), ino, bufv, off, fi);
}

void QtFuse::fuse_write_buf(QtFuseRequest *req, fuse_ino_t, struct fuse_bufvec *, off_t, struct fuse_file_info *) {
	QTFUSE_NOT_IMPL(ENOSYS);
}

void QtFuse::priv_qtfuse_retrieve_reply(fuse_req_t req, void *cookie, fuse_ino_t ino, off_t offset, struct fuse_bufvec *bufv) {
	QTFUSE_OBJ_FROM_REQ();
	QTFUSE_CHECK_INODE(ino);
	c->fuse_retrieve_reply(QTFUSE_REQ(), cookie, ino, offset, bufv);
}

void QtFuse::fuse_retrieve_reply(QtFuseRequest *req, void*, fuse_ino_t, off_t, struct fuse_bufvec *) {
	req->none();
}

void QtFuse::priv_qtfuse_forget_multi(fuse_req_t req, size_t count, struct fuse_forget_data *forgets) {
	QTFUSE_OBJ_FROM_REQ();
	c->fuse_forget_multi(QTFUSE_REQ(), count, forgets);
}

void QtFuse::fuse_forget_multi(QtFuseRequest *req, size_t count, struct fuse_forget_data *forgets) {
	for(size_t i = 0; i < count; i++) {
		fuse_ino_t ino = forgets[i].ino;
		QTFUSE_CHECK_INODE2(ino);
		fuse_forget(req, ino, forgets[i].nlookup);
	}
	req->none(); // just in case count==0
}

void QtFuse::priv_qtfuse_flock(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi, int op) {
	QTFUSE_OBJ_FROM_REQ();
	QTFUSE_CHECK_INODE(ino);
	c->fuse_flock(QTFUSE_REQ(), ino, fi, op);
}

void QtFuse::fuse_flock(QtFuseRequest *req, fuse_ino_t, struct fuse_file_info *, int) {
	QTFUSE_NOT_IMPL(ENOSYS);
}

void QtFuse::priv_qtfuse_fallocate(fuse_req_t req, fuse_ino_t ino, int mode, off_t offset, off_t length, struct fuse_file_info *fi) {
	QTFUSE_OBJ_FROM_REQ();
	QTFUSE_CHECK_INODE(ino);
	c->fuse_fallocate(QTFUSE_REQ(), ino, mode, offset, length, fi);
}

void QtFuse::fuse_fallocate(QtFuseRequest *req, fuse_ino_t, int, off_t, off_t, struct fuse_file_info *) {
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

void *QtFuse::qtfuse_start_thread(void *_c) {
	QtFuse *c = (QtFuse*)_c;
	char *argv[] = { strdup("fuse"), strdup("-f"), strdup(c->mp.data()) };
	int argc = 3;

	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
	int foreground;
	int multithreaded;
	int res;

	res = fuse_parse_cmdline(&args, &c->mountpoint, &multithreaded, &foreground);
	if (res == -1)
		return NULL;

	c->chan = fuse_mount(c->mountpoint, &args);
	if (!c->chan) {
		fuse_opt_free_args(&args);
		return NULL;
	}

	c->fuse = fuse_lowlevel_new(&args, &qtfuse_op, sizeof(qtfuse_op), _c);
	fuse_opt_free_args(&args);
	if (c->fuse == NULL)
		return NULL; // TODO: correctly unmount fuse

	fuse_session_add_chan(c->fuse, c->chan);
	
	size_t bufsize = fuse_chan_bufsize(c->chan);
	c->fuse_buf = (char *) malloc(bufsize);

	c->ready();

	while (!fuse_session_exited(c->fuse)) {
		c->tmpch = c->chan;
		res = fuse_chan_recv(&c->tmpch, c->fuse_buf, bufsize);
		if (res == -EINTR)
			continue;
		if (res <= 0)
			break;
		c->fuse_buf_len = res;
		QMetaObject::invokeMethod(c, "priv_fuse_session_process", Qt::BlockingQueuedConnection);
	}

	free(c->fuse_buf);
	fuse_session_reset(c->fuse);

	fuse_unmount(c->mountpoint, c->chan);
	fuse_session_destroy(c->fuse);

	return NULL;
}

void QtFuse::priv_fuse_session_process() {
	fuse_session_process(fuse, fuse_buf, fuse_buf_len, tmpch);
}

QtFuse::QtFuse(const QByteArray &_mp) {
	// required in some cases by Qt
	qRegisterMetaType<QtFuse*>("QtFuse*");
	qRegisterMetaType<QtFuseRequest*>("QtFuseRequest*");
	qRegisterMetaType<fuse_ino_t>("fuse_ino_t");
	qRegisterMetaType<struct fuse_file_info *>("struct fuse_file_info *");
	qRegisterMetaType<struct flock *>("struct flock *");

	mp = _mp;
	// so we can catch ^C and killed processes, make those signal call QCoreApplication::quit()
	if (!signals_set) {
		signals_set = true;
		set_one_signal_handler(SIGHUP, exit_handler);
		set_one_signal_handler(SIGINT, exit_handler);
		set_one_signal_handler(SIGTERM, exit_handler);
	}
	connect(QCoreApplication::instance(), SIGNAL(aboutToQuit()), this, SLOT(quit()));
}

void QtFuse::init() {
	pthread_create(&thread, NULL, qtfuse_start_thread, this);
	return;
}

void QtFuse::quit() {
	// fuse_session_exit(fuse_session)
	if (pthread_cancel(thread) != 0) return; // thread already dead
	pthread_join(thread, NULL);
	// teardown fuse
	fuse_session_reset(fuse);
	fuse_unmount(mountpoint, chan);
	fuse_session_destroy(fuse);
}

QtFuse::~QtFuse() {
	// fuse_session_exit(fuse_session)
	if (pthread_cancel(thread) != 0) return; // thread already dead
	pthread_join(thread, NULL);
	// teardown fuse
	fuse_session_reset(fuse);
	fuse_unmount(mountpoint, chan);
	fuse_session_destroy(fuse);
}

