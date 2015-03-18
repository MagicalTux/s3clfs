#include "QtFuse.hpp"

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

#define S3FUSE_BLOCK_SIZE 65536

#define FOREACH_s3fuseOps(X) \
	X(lookup) X(getattr) X(setattr) X(unlink) X(readlink) \
	X(mkdir) X(rmdir) X(symlink) X(rename) X(link) \
	X(open) X(read) X(write) X(flush) X(release) \
	X(opendir) X(readdir) X(releasedir) \
	X(create)

#define s3fuse_signature(_x) virtual void fuse_ ## _x(QtFuseRequest*);

class S3FS;
class S3FS_Config;

class S3Fuse: public QtFuse {
	Q_OBJECT
public:
	S3Fuse(S3FS_Config *cfg, S3FS*parent);

signals:
	void signal_forget(fuse_ino_t ino, unsigned long nlookup);

protected:
	virtual void fuse_init(struct fuse_conn_info *);
	//virtual void fuse_destroy();
	virtual void fuse_forget(fuse_ino_t ino, unsigned long nlookup);
	virtual void fuse_getxattr(QtFuseRequest *req);

	FOREACH_s3fuseOps(s3fuse_signature)

private:
	S3FS *parent;
};
