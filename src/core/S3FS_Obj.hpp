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
#include <QByteArray>
#include <QVariant>
#include <sys/stat.h>

#pragma once

class S3FS_Obj {
public:
	S3FS_Obj();
	S3FS_Obj(const QByteArray&);

	QByteArray encode() const;
	quint64 getInode() const; // get inode number
	bool isValid() const;

	bool isDir() const;
	bool isFifo() const;
	bool isCharDev() const;
	bool isBlock() const;
	bool isFile() const;
	bool isSymlink() const;
	bool isSocket() const;
	quint32 getFiletype() const;

	void reset();
	void makeRoot(); // configure object to act as root
	void makeDir(quint64 ino, int mode, int uid, int gid);
	void makeFile(quint64 ino, int mode, int uid, int gid);
	void makeEntry(quint64 ino, int type, int mode, int uid, int gid);
	bool decode(const QByteArray &);

	const struct stat &constAttr() const;
	void setAttr(const struct stat &s);

	size_t size() const;
	void setSize(size_t);

private:
	struct stat attr;
};

