#include "QtFuse.hpp"
#include <QObject>

#pragma once

class QtFuseRequest: public QObject {
	Q_OBJECT;

public:
	QtFuseRequest(fuse_req_t req, QtFuse &_parent);
	~QtFuseRequest();

public slots:
	void error(int);
	void none();

public:
	void entry(const struct stat*, int generation = 1);
	void create(const struct stat*, const struct fuse_file_info *fi, int generation = 1);
	void attr(const struct stat*, double attr_timeout = 0);
	void readlink(const QByteArray &link);
	void open(const struct fuse_file_info *fi);
	void write(size_t count);
	void buf(const QByteArray &data);
	void iov(const struct iovec *iov, int count);
	void statfs(const struct statvfs *stbuf);
	void xattr(size_t count);
	void lock(struct flock *lock);
	void bmap(uint64_t idx);

	const struct fuse_ctx *context() const;

	bool dir_add(const QByteArray &name, const struct stat *stbuf, off_t next_offset);
	void dir_send();

protected:
	void prepareBuffer(size_t size);

	friend class QtFuse;

private:
	fuse_req_t req;
	QtFuse &parent;
	bool answered;
	
	char *data_buf;
	size_t buf_pos, buf_size;
};

Q_DECLARE_METATYPE(QtFuseRequest*);

