#include "QtFuse.hpp"
#include <QObject>

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

class QtFuseRequest: public QObject {
	Q_OBJECT;

public:
	QtFuseRequest(fuse_req_t req, QtFuse &_parent, struct fuse_file_info *fi = 0);
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

	struct fuse_file_info *fi();

	void setAttr(struct stat*);
	struct stat *attr();

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
	struct fuse_file_info fuse_fi;
	struct stat fuse_attr;
};

Q_DECLARE_METATYPE(QtFuseRequest*);

