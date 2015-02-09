#include <QObject>
#include "S3Fuse.hpp"
#include "S3FS_Store.hpp"
#include "Keyval.hpp"

#pragma once

class S3FS: public QObject {
	Q_OBJECT

public:
	S3FS(const QByteArray &bucket, const QByteArray &path, const QByteArray &queue, const QByteArray &fuse_opts, const QString &cache);
	void format();
	bool isReady() const;
	S3FS_Store &getStore();

signals:
	void ready();

public slots:
	void fuse_lookup(QtFuseRequest *req, fuse_ino_t ino, const QByteArray &path);
	void fuse_forget(QtFuseRequest *req, fuse_ino_t ino, unsigned long nlookup);
	void fuse_setattr(QtFuseRequest *req, fuse_ino_t node, struct_stat *attr, int to_set, struct fuse_file_info *fi);
	void fuse_getattr(QtFuseRequest *req, fuse_ino_t node, struct fuse_file_info *fi);
	void fuse_unlink(QtFuseRequest *req, fuse_ino_t parent_ino, const QByteArray &name);
	void fuse_readlink(QtFuseRequest *req, fuse_ino_t node);
	void fuse_mkdir(QtFuseRequest *req, fuse_ino_t parent, const QByteArray &name, int mode);
	void fuse_rmdir(QtFuseRequest *req, fuse_ino_t parent, const QByteArray &name);
	void fuse_symlink(QtFuseRequest *req, const QByteArray &link, fuse_ino_t parent, const QByteArray &name);
	void fuse_rename(QtFuseRequest *req, fuse_ino_t parent, const QByteArray &name, fuse_ino_t newparent, const QByteArray &newname);
	void fuse_link(QtFuseRequest *req, fuse_ino_t ino, fuse_ino_t newparent, const QByteArray &newname);
	void fuse_flush(QtFuseRequest *req, fuse_ino_t ino, struct fuse_file_info *fi);
	void fuse_release(QtFuseRequest *req, fuse_ino_t ino, struct fuse_file_info *fi);
	void fuse_open(QtFuseRequest *req, fuse_ino_t ino, struct fuse_file_info *fi);
	void fuse_opendir(QtFuseRequest *req, fuse_ino_t ino, struct fuse_file_info *fi);
	void fuse_readdir(QtFuseRequest *req, fuse_ino_t ino, off_t off, struct fuse_file_info *fi);
	void fuse_releasedir(QtFuseRequest *req, fuse_ino_t ino, struct fuse_file_info *fi);
	void fuse_create(QtFuseRequest *req, fuse_ino_t parent, const char *name, mode_t mode, struct fuse_file_info *fi);
	void fuse_read(QtFuseRequest *req, fuse_ino_t ino, size_t size, off_t offset, struct fuse_file_info *fi);
	void fuse_write(QtFuseRequest *req, fuse_ino_t ino, const QByteArray &buf, off_t offset, struct fuse_file_info *fi);
	void fuse_write_buf(QtFuseRequest *req, fuse_ino_t ino, struct fuse_bufvec *bufv, off_t off, struct fuse_file_info *fi);
	void storeIsReady();

protected:
	quint64 makeInode();
	bool real_write(S3FS_Obj &ino, const QByteArray &buf, off_t offset, QList<QGenericArgument>&, bool &wait);

private:
	S3Fuse fuse;
	S3FS_Store store;
	QByteArray bucket;
	bool is_ready;
	quint64 last_inode;
};

