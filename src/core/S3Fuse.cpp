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
#include "S3Fuse.hpp"
#include "S3FS.hpp"
#include "S3FS_Config.hpp"
#include "QtFuseRequest.hpp"

S3Fuse::S3Fuse(S3FS_Config *cfg, S3FS *_parent): QtFuse(cfg->mountPath(), cfg->bucket(), cfg->mountOptions(), _parent) {
	parent = _parent;

	// connect
	connect(this, &S3Fuse::signal_forget, parent, &S3FS::fuse_forget);
}

void S3Fuse::fuse_init(struct fuse_conn_info *ci) {
	ci->async_read = 1;
	ci->max_write = S3FUSE_BLOCK_SIZE;
	ci->max_readahead = S3FUSE_BLOCK_SIZE * 32;
	ci->capable &= ~FUSE_CAP_SPLICE_READ;
	ci->want = FUSE_CAP_ASYNC_READ | FUSE_CAP_ATOMIC_O_TRUNC | FUSE_CAP_EXPORT_SUPPORT | FUSE_CAP_BIG_WRITES | FUSE_CAP_IOCTL_DIR;
	ci->max_background = 16;
	ci->congestion_threshold = 32;
}

void S3Fuse::fuse_forget(fuse_ino_t ino, unsigned long nlookup) {
	signal_forget(ino, nlookup);
}

void S3Fuse::fuse_getxattr(QtFuseRequest *req) {
	req->error(ENOTSUP); // just return ENOSYS here to avoid log full of "getxattr not impl"
}

#define s3fuse_sig_handle(_x) void S3Fuse::fuse_ ## _x(QtFuseRequest *req) { req->setMethod<S3FS>(parent, &S3FS::fuse_ ## _x); req->triggerLater(); }
FOREACH_s3fuseOps(s3fuse_sig_handle);

