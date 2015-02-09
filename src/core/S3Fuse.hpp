#include "QtFuse.hpp"

#pragma once

#define S3FUSE_BLOCK_SIZE 65536

class S3FS;

class S3Fuse: public QtFuse {
	Q_OBJECT
public:
	S3Fuse(const QByteArray &bucket, const QByteArray &path, const QByteArray &opts, S3FS*parent);

protected:
	virtual void fuse_init(struct fuse_conn_info *);
	//virtual void fuse_destroy();
	virtual void fuse_lookup(QtFuseRequest *req, fuse_ino_t parent, const QByteArray &name);
	virtual void fuse_forget(QtFuseRequest *req, fuse_ino_t ino, unsigned long nlookup);
	virtual void fuse_getattr(QtFuseRequest *req, fuse_ino_t node, struct fuse_file_info *fi);
	virtual void fuse_setattr(QtFuseRequest *req, fuse_ino_t node, struct stat *attr, int to_set, struct fuse_file_info *fi);
	virtual void fuse_unlink(QtFuseRequest *req, fuse_ino_t parent, const QByteArray &name);
	virtual void fuse_readlink(QtFuseRequest *req, fuse_ino_t node);
	virtual void fuse_mkdir(QtFuseRequest *req, fuse_ino_t parent, const QByteArray &name, int mode);
	virtual void fuse_rmdir(QtFuseRequest *req, fuse_ino_t parent, const QByteArray &name);
	virtual void fuse_symlink(QtFuseRequest *req, const QByteArray &link, fuse_ino_t parent, const QByteArray &name);
	virtual void fuse_rename(QtFuseRequest *req, fuse_ino_t parent, const QByteArray &name, fuse_ino_t newparent, const QByteArray &newname);
	virtual void fuse_link(QtFuseRequest *req, fuse_ino_t ino, fuse_ino_t newparent, const QByteArray &newname);
	//virtual void fuse_mknod(QtFuseRequest *req, fuse_ino_t parent, const QByteArray &name, mode_t mode, dev_t dev);
	virtual void fuse_open(QtFuseRequest *req, fuse_ino_t ino, struct fuse_file_info *fi);
	virtual void fuse_read(QtFuseRequest *req, fuse_ino_t ino, size_t size, off_t offset, struct fuse_file_info *fi);
	virtual void fuse_write(QtFuseRequest *req, fuse_ino_t ino, const QByteArray&, off_t offset, struct fuse_file_info *fi);
	virtual void fuse_flush(QtFuseRequest *req, fuse_ino_t ino, struct fuse_file_info *fi);
	virtual void fuse_release(QtFuseRequest *req, fuse_ino_t ino, struct fuse_file_info *fi);
	//virtual void fuse_fsync(QtFuseRequest *req, fuse_ino_t ino, int datasync, struct fuse_file_info *fi);
	virtual void fuse_opendir(QtFuseRequest *req, fuse_ino_t ino, struct fuse_file_info *fi);
	virtual void fuse_readdir(QtFuseRequest *req, fuse_ino_t ino, off_t off, struct fuse_file_info *fi);
	virtual void fuse_releasedir(QtFuseRequest *req, fuse_ino_t ino, struct fuse_file_info *fi);
	//virtual void fuse_fsyncdir(QtFuseRequest *req, fuse_ino_t ino, int datasync, struct fuse_file_info *fi);
	//virtual void fuse_statfs(QtFuseRequest *req, fuse_ino_t ino);
	//virtual void fuse_setxattr(QtFuseRequest *req, fuse_ino_t ino, const char *name, const char *value, size_t size, int flags);
	//virtual void fuse_getxattr(QtFuseRequest *req, fuse_ino_t ino, const char *name, size_t size);
	//virtual void fuse_listxattr(QtFuseRequest *req, fuse_ino_t ino, size_t size);
	//virtual void fuse_removexattr(QtFuseRequest *req, fuse_ino_t ino, const char *name);
	virtual void fuse_create(QtFuseRequest *req, fuse_ino_t parent, const char *name, mode_t mode, struct fuse_file_info *fi);
	//virtual void fuse_getlk(QtFuseRequest *req, fuse_ino_t ino, struct fuse_file_info *fi, struct flock *fl);
	//virtual void fuse_setlk(QtFuseRequest *req, fuse_ino_t ino, struct fuse_file_info *fi, struct flock *lock, int sleep);
	//virtual void fuse_bmap(QtFuseRequest *req, fuse_ino_t ino, size_t blocksize, uint64_t idx);
	//virtual void fuse_ioctl(QtFuseRequest *req, fuse_ino_t ino, int cmd, void *arg, struct fuse_file_info *fi, unsigned flags, const void *in_buf, size_t in_bufsz, size_t out_bufsz);
	//virtual void fuse_poll(QtFuseRequest *req, fuse_ino_t ino, struct fuse_file_info *fi, struct fuse_pollhandle *ph);
	virtual void fuse_write_buf(QtFuseRequest *req, fuse_ino_t ino, struct fuse_bufvec *bufv, off_t off, struct fuse_file_info *fi);
	//virtual void fuse_retrieve_reply(QtFuseRequest *req, void *cookie, fuse_ino_t ino, off_t offset, struct fuse_bufvec *bufv);
	//virtual void fuse_forget_multi(QtFuseRequest *req, size_t count, struct fuse_forget_data *forgets);
	//virtual void fuse_flock(QtFuseRequest *req, fuse_ino_t ino, struct fuse_file_info *fi, int op);
	//virtual void fuse_fallocate(QtFuseRequest *req, fuse_ino_t ino, int mode, off_t offset, off_t length, struct fuse_file_info *fi);

private:
	S3FS *parent;
};
