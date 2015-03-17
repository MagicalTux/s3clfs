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

#include <QObject>
#include "S3FS_Store.hpp"
#include "Keyval.hpp"
#include "QtFuseRequest.hpp"

#pragma once

class S3FS_Config;

class S3FS: public QObject {
	Q_OBJECT

public:
	S3FS(S3FS_Config *cfg);
	void format();
	bool isReady() const;
	S3FS_Store &getStore();

signals:
	void ready();
	void loadReduced();

public slots:
	void fuse_lookup(QtFuseRequest *req);
	void fuse_forget(fuse_ino_t ino, unsigned long nlookup);
	void fuse_setattr(QtFuseRequest *req);
	void fuse_getattr(QtFuseRequest *req);
	void fuse_unlink(QtFuseRequest *req);
	void fuse_readlink(QtFuseRequest *req);
	void fuse_mkdir(QtFuseRequest *req);
	void fuse_rmdir(QtFuseRequest *req);
	void fuse_symlink(QtFuseRequest *req);
	void fuse_rename(QtFuseRequest *req);
	void fuse_link(QtFuseRequest *req);
	void fuse_flush(QtFuseRequest *req);
	void fuse_release(QtFuseRequest *req);
	void fuse_open(QtFuseRequest *req);
	void fuse_opendir(QtFuseRequest *req);
	void fuse_readdir(QtFuseRequest *req);
	void fuse_releasedir(QtFuseRequest *req);
	void fuse_create(QtFuseRequest *req);
	void fuse_read(QtFuseRequest *req);
	void fuse_write(QtFuseRequest *req);
	void storeIsReady();

	void setOverload(bool);

protected:
	quint64 makeInode();
	bool real_write(S3FS_Obj &ino, const QByteArray &buf, off_t offset, QtFuseRequest *, bool &wait);

private:
	S3FS_Store store;
	bool is_ready;
	bool is_overloaded;
	quint64 last_inode;
	S3FS_Config *cfg;
	int cluster_node_id;
};

