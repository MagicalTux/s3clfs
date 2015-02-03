#ifndef __HEADER_QTFUSEREQUEST_H
#define __HEADER_QTFUSEREQUEST_H

#include "QtFuse.hpp"
#include <QObject>

class QtFuseRequest: private QObject {
	Q_OBJECT;

public:
	QtFuseRequest(fuse_req_t req, QtFuse &_parent);
	~QtFuseRequest();

public slots:
	void error(int);
	void none();

public:
	void entry(const QtFuseNode*);
	void create(const struct fuse_entry_param *e, const struct fuse_file_info *fi);
	void attr(const struct stat*, double attr_timeout = 0);
	void readlink(const QString &link);
	void open(const struct fuse_file_info *fi);
	void write(size_t count);
	void buf(const QByteArray &data);
	void iov(const struct iovec *iov, int count);
	void statfs(const struct statvfs *stbuf);
	void xattr(size_t count);
	void lock(struct flock *lock);
	void bmap(uint64_t idx);

	bool dir_add(const QString &name, const struct stat *stbuf, off_t next_offset);
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

#endif /* __HEADER_QTFUSEREQUEST_H */
