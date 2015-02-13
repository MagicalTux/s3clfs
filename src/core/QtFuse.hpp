#define _FILE_OFFSET_BITS 64
#define FUSE_USE_VERSION 26
#include <QObject>
#include <QMutex>
#include <QMap>
#include <pthread.h>
#include <errno.h>

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

#pragma once

#include <fuse.h>
#include <fuse/fuse_lowlevel.h>

struct qtfuse_callback_data {
	int spair[2];
};

class QtFuse;
class QtFuseRequest;

class QtFuse: public QObject {
	Q_OBJECT;
public:
	QtFuse(const QByteArray &mp, const QByteArray &src = QByteArrayLiteral("QtFuse"), const QByteArray &opts = QByteArray());
	~QtFuse();
	static void prepare();

signals:
	void ready();

public slots:
	void init();
	void quit();

protected:
	virtual void fuse_init(struct fuse_conn_info *);
	virtual void fuse_destroy();
	virtual void fuse_lookup(QtFuseRequest *req, fuse_ino_t parent, const QByteArray &name);
	virtual void fuse_forget(QtFuseRequest *req, fuse_ino_t node, unsigned long nlookup);
	virtual void fuse_getattr(QtFuseRequest *req, fuse_ino_t node);
	virtual void fuse_setattr(QtFuseRequest *req, fuse_ino_t node, struct stat *attr, int to_set);
	virtual void fuse_readlink(QtFuseRequest *req, fuse_ino_t node);
	virtual void fuse_mkdir(QtFuseRequest *req, fuse_ino_t parent, const QByteArray &name, int mode);
	virtual void fuse_unlink(QtFuseRequest *req, fuse_ino_t parent, const QByteArray &name);
	virtual void fuse_rmdir(QtFuseRequest *req, fuse_ino_t parent, const QByteArray &name);
	virtual void fuse_symlink(QtFuseRequest *req, const QByteArray &link, fuse_ino_t parent, const QByteArray &name);
	virtual void fuse_rename(QtFuseRequest *req, fuse_ino_t parent, const QByteArray &name, fuse_ino_t newparent, const QByteArray &newname);
	virtual void fuse_link(QtFuseRequest *req, fuse_ino_t ino, fuse_ino_t newparent, const QByteArray &newname);
	virtual void fuse_mknod(QtFuseRequest *req, fuse_ino_t parent, const QByteArray &name, mode_t mode, dev_t dev);
	virtual void fuse_open(QtFuseRequest *req, fuse_ino_t ino);
	virtual void fuse_read(QtFuseRequest *req, fuse_ino_t ino, size_t size, off_t offset);
	virtual void fuse_write(QtFuseRequest *req, fuse_ino_t ino, const QByteArray&, off_t offset);
	virtual void fuse_flush(QtFuseRequest *req, fuse_ino_t ino);
	virtual void fuse_release(QtFuseRequest *req, fuse_ino_t ino);
	virtual void fuse_fsync(QtFuseRequest *req, fuse_ino_t ino, int datasync);
	virtual void fuse_opendir(QtFuseRequest *req, fuse_ino_t ino);
	virtual void fuse_readdir(QtFuseRequest *req, fuse_ino_t ino, off_t off);
	virtual void fuse_releasedir(QtFuseRequest *req, fuse_ino_t ino);
	virtual void fuse_fsyncdir(QtFuseRequest *req, fuse_ino_t ino, int datasync);
	virtual void fuse_statfs(QtFuseRequest *req, fuse_ino_t ino);
	virtual void fuse_setxattr(QtFuseRequest *req, fuse_ino_t ino, const char *name, const char *value, size_t size, int flags);
	virtual void fuse_getxattr(QtFuseRequest *req, fuse_ino_t ino, const char *name, size_t size);
	virtual void fuse_listxattr(QtFuseRequest *req, fuse_ino_t ino, size_t size);
	virtual void fuse_removexattr(QtFuseRequest *req, fuse_ino_t ino, const char *name);
	virtual void fuse_access(QtFuseRequest *req, fuse_ino_t ino, int mask);
	virtual void fuse_create(QtFuseRequest *req, fuse_ino_t parent, const char *name, mode_t mode);
	virtual void fuse_getlk(QtFuseRequest *req, fuse_ino_t ino, struct flock *fl);
	virtual void fuse_setlk(QtFuseRequest *req, fuse_ino_t ino, struct flock *lock, int sleep);
	virtual void fuse_bmap(QtFuseRequest *req, fuse_ino_t ino, size_t blocksize, uint64_t idx);
	virtual void fuse_ioctl(QtFuseRequest *req, fuse_ino_t ino, int cmd, void *arg, unsigned flags, const void *in_buf, size_t in_bufsz, size_t out_bufsz);
	virtual void fuse_poll(QtFuseRequest *req, fuse_ino_t ino, struct fuse_pollhandle *ph);
	virtual void fuse_write_buf(QtFuseRequest *req, fuse_ino_t ino, struct fuse_bufvec *bufv, off_t off);
	virtual void fuse_retrieve_reply(QtFuseRequest *req, void *cookie, fuse_ino_t ino, off_t offset, struct fuse_bufvec *bufv);
	virtual void fuse_forget_multi(QtFuseRequest *req, size_t count, struct fuse_forget_data *forgets);
	virtual void fuse_flock(QtFuseRequest *req, fuse_ino_t ino, int op);
	virtual void fuse_fallocate(QtFuseRequest *req, fuse_ino_t ino, int mode, off_t offset, off_t length);

private:
	QByteArray mp; // mount point
	QByteArray src;
	QByteArray opts;
	pthread_t thread;
	bool fuse_cleaned;

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
};

typedef struct stat struct_stat;

Q_DECLARE_METATYPE(QtFuse*);
Q_DECLARE_METATYPE(fuse_ino_t);
Q_DECLARE_METATYPE(mode_t);
Q_DECLARE_METATYPE(struct_stat*);
Q_DECLARE_METATYPE(struct fuse_bufvec*);
