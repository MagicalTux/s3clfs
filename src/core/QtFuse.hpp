#define _FILE_OFFSET_BITS 64
#define FUSE_USE_VERSION 26
#include <QObject>
#include <QMutex>
#include <QMap>
#include <pthread.h>
#include <errno.h>

#pragma once

#include <fuse.h>
#include <fuse/fuse_lowlevel.h>

struct qtfuse_callback_data {
	int spair[2];
};

class QtFuse;
class QtFuseNode;
class QtFuseRequest;

class QtFuse: public QObject {
	Q_OBJECT;
public:
	QtFuse(const QByteArray &mp);
	~QtFuse();
	void init();

signals:
	void ready();

public slots:
	void quit();
	void priv_fuse_session_process();

protected:
	virtual void fuse_init(struct fuse_conn_info *);
	virtual void fuse_destroy();
	virtual void fuse_lookup(QtFuseRequest *req, QtFuseNode *parent, const QByteArray &name);
	virtual void fuse_lookup_root(QtFuseRequest *req); // called to lookup the root node
	virtual void fuse_forget(QtFuseRequest *req, QtFuseNode *node, unsigned long nlookup);
	virtual void fuse_getattr(QtFuseRequest *req, QtFuseNode *node, struct fuse_file_info *fi);
	virtual void fuse_setattr(QtFuseRequest *req, QtFuseNode *node, struct stat *attr, int to_set, struct fuse_file_info *fi);
	virtual void fuse_readlink(QtFuseRequest *req, QtFuseNode *node);
	virtual void fuse_mkdir(QtFuseRequest *req, QtFuseNode *parent, const QByteArray &name, int mode);
	virtual void fuse_unlink(QtFuseRequest *req, QtFuseNode *parent, const QByteArray &name);
	virtual void fuse_rmdir(QtFuseRequest *req, QtFuseNode *parent, const QByteArray &name);
	virtual void fuse_symlink(QtFuseRequest *req, const QByteArray &link, QtFuseNode *parent, const QByteArray &name);
	virtual void fuse_rename(QtFuseRequest *req, QtFuseNode *parent, const QByteArray &name, QtFuseNode *newparent, const QByteArray &newname);
	virtual void fuse_link(QtFuseRequest *req, QtFuseNode *ino, QtFuseNode *newparent, const QByteArray &newname);
	virtual void fuse_mknod(QtFuseRequest *req, QtFuseNode *parent, const QByteArray &name, mode_t mode, dev_t dev);
	virtual void fuse_open(QtFuseRequest *req, QtFuseNode *ino, struct fuse_file_info *fi);
	virtual void fuse_read(QtFuseRequest *req, QtFuseNode *ino, size_t size, off_t offset, struct fuse_file_info *fi);
	virtual void fuse_write(QtFuseRequest *req, QtFuseNode *ino, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi);
	virtual void fuse_flush(QtFuseRequest *req, QtFuseNode *ino, struct fuse_file_info *fi);
	virtual void fuse_release(QtFuseRequest *req, QtFuseNode *ino, struct fuse_file_info *fi);
	virtual void fuse_fsync(QtFuseRequest *req, QtFuseNode *ino, int datasync, struct fuse_file_info *fi);
	virtual void fuse_opendir(QtFuseRequest *req, QtFuseNode *ino, struct fuse_file_info *fi);
	virtual void fuse_readdir(QtFuseRequest *req, QtFuseNode *ino, off_t off, struct fuse_file_info *fi);
	virtual void fuse_releasedir(QtFuseRequest *req, QtFuseNode *ino, struct fuse_file_info *fi);
	virtual void fuse_fsyncdir(QtFuseRequest *req, QtFuseNode *ino, int datasync, struct fuse_file_info *fi);
	virtual void fuse_statfs(QtFuseRequest *req, QtFuseNode *ino);
	virtual void fuse_setxattr(QtFuseRequest *req, QtFuseNode *ino, const char *name, const char *value, size_t size, int flags);
	virtual void fuse_getxattr(QtFuseRequest *req, QtFuseNode *ino, const char *name, size_t size);
	virtual void fuse_listxattr(QtFuseRequest *req, QtFuseNode *ino, size_t size);
	virtual void fuse_removexattr(QtFuseRequest *req, QtFuseNode *ino, const char *name);
	virtual void fuse_access(QtFuseRequest *req, QtFuseNode *ino, int mask);
	virtual void fuse_create(QtFuseRequest *req, QtFuseNode *parent, const char *name, mode_t mode, struct fuse_file_info *fi);
	virtual void fuse_getlk(QtFuseRequest *req, QtFuseNode *ino, struct fuse_file_info *fi, struct flock *fl);
	virtual void fuse_setlk(QtFuseRequest *req, QtFuseNode *ino, struct fuse_file_info *fi, struct flock *lock, int sleep);
	virtual void fuse_bmap(QtFuseRequest *req, QtFuseNode *ino, size_t blocksize, uint64_t idx);
	virtual void fuse_ioctl(QtFuseRequest *req, QtFuseNode *ino, int cmd, void *arg, struct fuse_file_info *fi, unsigned flags, const void *in_buf, size_t in_bufsz, size_t out_bufsz);
	virtual void fuse_poll(QtFuseRequest *req, QtFuseNode *ino, struct fuse_file_info *fi, struct fuse_pollhandle *ph);
	virtual void fuse_write_buf(QtFuseRequest *req, QtFuseNode *ino, struct fuse_bufvec *bufv, off_t off, struct fuse_file_info *fi);
	virtual void fuse_retrieve_reply(QtFuseRequest *req, void *cookie, QtFuseNode *ino, off_t offset, struct fuse_bufvec *bufv);
	virtual void fuse_forget_multi(QtFuseRequest *req, size_t count, struct fuse_forget_data *forgets);
	virtual void fuse_flock(QtFuseRequest *req, QtFuseNode *ino, struct fuse_file_info *fi, int op);
	virtual void fuse_fallocate(QtFuseRequest *req, QtFuseNode *ino, int mode, off_t offset, off_t length, struct fuse_file_info *fi);

	virtual bool inode_exists(fuse_ino_t);
	virtual QtFuseNode *inode_get(fuse_ino_t);

	virtual QtFuseNode *fuse_make_root_node(struct stat *attr);
	QMap<long int, QtFuseNode*> inode_map;
	long int inode_map_idx;
	int inode_map_generation;
private:
	QByteArray mp; // mount point
	pthread_t thread;

	static void *qtfuse_start_thread(void *_c);
	static void priv_qtfuse_init(void *userdata, struct fuse_conn_info *conn);
	static void priv_qtfuse_destroy(void *userdata);
	static void priv_qtfuse_lookup(fuse_req_t req, fuse_ino_t parent, const char *name);
	static void priv_qtfuse_forget(fuse_req_t req, fuse_ino_t ino, unsigned long nlookup);
	static void priv_qtfuse_getattr(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi);
	static void priv_qtfuse_setattr(fuse_req_t req, fuse_ino_t ino, struct stat *attr, int to_set, struct fuse_file_info *fi);
	static void priv_qtfuse_readlink(fuse_req_t req, fuse_ino_t ino);
	static void priv_qtfuse_mknod(fuse_req_t req, fuse_ino_t parent, const char *name, mode_t mode, dev_t rdev);
	static void priv_qtfuse_mkdir(fuse_req_t req, fuse_ino_t parent, const char *name, mode_t mode);
	static void priv_qtfuse_unlink(fuse_req_t req, fuse_ino_t parent, const char *name);
	static void priv_qtfuse_rmdir(fuse_req_t req, fuse_ino_t parent, const char *name);
	static void priv_qtfuse_symlink(fuse_req_t req, const char *link, fuse_ino_t parent, const char *name);
	static void priv_qtfuse_rename(fuse_req_t req, fuse_ino_t parent, const char *name, fuse_ino_t newparent, const char *newname);
	static void priv_qtfuse_link(fuse_req_t req, fuse_ino_t ino, fuse_ino_t newparent, const char *newname);
	static void priv_qtfuse_open(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi);
	static void priv_qtfuse_read(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off, struct fuse_file_info *fi);
	static void priv_qtfuse_write(fuse_req_t req, fuse_ino_t ino, const char *buf, size_t size, off_t off, struct fuse_file_info *fi);
	static void priv_qtfuse_flush(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi);
	static void priv_qtfuse_release(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi);
	static void priv_qtfuse_fsync(fuse_req_t req, fuse_ino_t ino, int datasync, struct fuse_file_info *fi);
	static void priv_qtfuse_opendir(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi);
	static void priv_qtfuse_readdir(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off, struct fuse_file_info *fi);
	static void priv_qtfuse_releasedir(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi);
	static void priv_qtfuse_fsyncdir(fuse_req_t req, fuse_ino_t ino, int datasync, struct fuse_file_info *fi);
	static void priv_qtfuse_statfs(fuse_req_t req, fuse_ino_t ino);
	static void priv_qtfuse_setxattr(fuse_req_t req, fuse_ino_t ino, const char *name, const char *value, size_t size, int flags);
	static void priv_qtfuse_getxattr(fuse_req_t req, fuse_ino_t ino, const char *name, size_t size);
	static void priv_qtfuse_listxattr(fuse_req_t req, fuse_ino_t ino, size_t size);
	static void priv_qtfuse_removexattr(fuse_req_t req, fuse_ino_t ino, const char *name);
	static void priv_qtfuse_access(fuse_req_t req, fuse_ino_t ino, int mask);
	static void priv_qtfuse_create(fuse_req_t req, fuse_ino_t parent, const char *name, mode_t mode, struct fuse_file_info *fi);
	static void priv_qtfuse_getlk(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi, struct flock *lock);
	static void priv_qtfuse_setlk(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi, struct flock *lock, int sleep);
	static void priv_qtfuse_bmap(fuse_req_t req, fuse_ino_t ino, size_t blocksize, uint64_t idx);
	static void priv_qtfuse_ioctl(fuse_req_t req, fuse_ino_t ino, int cmd, void *arg, struct fuse_file_info *fi, unsigned flags, const void *in_buf, size_t in_bufsz, size_t out_bufsz);
	static void priv_qtfuse_poll(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi, struct fuse_pollhandle *ph);
	static void priv_qtfuse_write_buf(fuse_req_t req, fuse_ino_t ino, struct fuse_bufvec *bufv, off_t off, struct fuse_file_info *fi);
	static void priv_qtfuse_retrieve_reply(fuse_req_t req, void *cookie, fuse_ino_t ino, off_t offset, struct fuse_bufvec *bufv);
	static void priv_qtfuse_forget_multi(fuse_req_t req, size_t count, struct fuse_forget_data *forgets);
	static void priv_qtfuse_flock(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi, int op);
	static void priv_qtfuse_fallocate(fuse_req_t req, fuse_ino_t ino, int mode, off_t offset, off_t length, struct fuse_file_info *fi);

	static struct fuse_lowlevel_ops qtfuse_op;

	// for fuse use
	struct fuse_session *fuse;
	struct fuse_chan *chan, *tmpch;
	char *mountpoint;
	char *fuse_buf;
	int fuse_buf_len;
};

