#include <S3FS_Obj.hpp>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>

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

S3FS_Obj::S3FS_Obj() {
	reset();
}

S3FS_Obj::S3FS_Obj(const QByteArray &buf) {
	decode(buf);
}

bool S3FS_Obj::isValid() const {
	return attr.st_ino > 0;
}

void S3FS_Obj::reset() {
	memset(&attr, 0, sizeof(attr));

	// some default settings
	attr.st_dev = 0x1337;
	attr.st_nlink = 1;
	attr.st_blksize = 512;
}

void S3FS_Obj::makeRoot() {
	makeEntry(1, S_IFDIR, 0755, getuid(), getgid());
}

void S3FS_Obj::makeDir(quint64 ino, int mode, int uid, int gid) {
	makeEntry(ino, S_IFDIR, mode, uid, gid);
}

void S3FS_Obj::makeFile(quint64 ino, int mode, int uid, int gid) {
	makeEntry(ino, S_IFREG, mode, uid, gid);
}
void S3FS_Obj::touch(bool mtime) {
	struct timeval tmp;
	gettimeofday(&tmp, NULL);
	struct timespec t;
	t.tv_sec = tmp.tv_sec;
	t.tv_nsec = tmp.tv_usec * 1000;

	attr.st_ctim = t;
	if (mtime) attr.st_mtim = t;
}

void S3FS_Obj::makeEntry(quint64 ino, int type, int mode, int uid, int gid) {
	struct timeval tmp;
	gettimeofday(&tmp, NULL);
	struct timespec t;
	t.tv_sec = tmp.tv_sec;
	t.tv_nsec = tmp.tv_usec * 1000;

	reset();
	attr.st_ino = ino;
	attr.st_mode = (mode & ~S_IFMT) | (type & S_IFMT);
	attr.st_uid = uid;
	attr.st_gid = gid;
	attr.st_size = 0;
	attr.st_ctim = t;
	attr.st_mtim = t;
	attr.st_atim = t;
}

QByteArray S3FS_Obj::encode() const {
	// create storable object
	QVariantMap attrs;
	#define STORE_VAL(_x) attrs.insert(#_x, attr.st_ ## _x)
	#define STORE_VAL_T(_x, _y) attrs.insert(#_x, (_y)(attr.st_ ## _x))
	STORE_VAL_T(ino, quint64);
	STORE_VAL(mode);
	STORE_VAL(uid);
	STORE_VAL(gid);
	STORE_VAL_T(rdev, quint64);
	STORE_VAL_T(ctime, qint64);
	STORE_VAL_T(mtime, qint64);
	STORE_VAL_T(atime, qint64);
	STORE_VAL_T(ctim.tv_nsec, quint64);
	STORE_VAL_T(mtim.tv_nsec, quint64);
	STORE_VAL_T(atim.tv_nsec, quint64);
	STORE_VAL_T(size, quint64);

	QVariantMap props;
	props.insert("version", (quint32)0);
	props.insert("attrs", attrs);

	QByteArray buf;
	QDataStream s(&buf, QIODevice::WriteOnly);
	s << props;

	return buf;
}

bool S3FS_Obj::decode(const QByteArray &buf) {
	QVariantMap props;
	QDataStream s(buf);

	s >> props;

	if (props.isEmpty()) {
		qDebug("S3FS_Obj: corrupted object, props are not valid");
		return false;
	}

	QVariantMap attrs = props.value("attrs").toMap();

	if (attrs.isEmpty()) {
		qDebug("S3FS_Obj: corrupted object, attrs are empty");
		return false;
	}

	reset();
	attr.st_ino = attrs.value("ino").toULongLong();
	attr.st_mode = attrs.value("mode").toUInt();
	attr.st_uid = attrs.value("uid").toUInt();
	attr.st_gid = attrs.value("gid").toUInt();
	attr.st_rdev = attrs.value("rdev").toULongLong(); // dev_t is currently a uint32, but it can't hurt to store it more
	attr.st_ctime = attrs.value("ctime").toLongLong();
	attr.st_mtime = attrs.value("mtime").toLongLong();
	attr.st_atime = attrs.value("atime").toLongLong();
	attr.st_ctim.tv_nsec = attrs.value("ctim.tv_nsec").toULongLong();
	attr.st_mtim.tv_nsec = attrs.value("mtim.tv_nsec").toULongLong();
	attr.st_atim.tv_nsec = attrs.value("atim.tv_nsec").toULongLong();
	attr.st_size = attrs.value("size").toULongLong();

	return true;
}

quint64 S3FS_Obj::getInode() const {
	return attr.st_ino;
}

const struct stat &S3FS_Obj::constAttr() const {
	return attr;
}

void S3FS_Obj::setAttr(const struct stat &s) {
	attr = s;
}

quint32 S3FS_Obj::getFiletype() const {
	return attr.st_mode & S_IFMT;
}

bool S3FS_Obj::isDir() const {
	return (attr.st_mode & S_IFMT) == S_IFDIR; // meta content: inodes index
}

bool S3FS_Obj::isFifo() const {
	return (attr.st_mode & S_IFMT) == S_IFIFO;
}

bool S3FS_Obj::isCharDev() const {
	return (attr.st_mode & S_IFMT) == S_IFCHR;
}

bool S3FS_Obj::isBlock() const {
	return (attr.st_mode & S_IFMT) == S_IFBLK;
}

bool S3FS_Obj::isFile() const {
	return (attr.st_mode & S_IFMT) == S_IFREG; // meta content: offset of blocks
}

bool S3FS_Obj::isSymlink() const {
	return (attr.st_mode & S_IFMT) == S_IFLNK; // meta content: symlink target
}

bool S3FS_Obj::isSocket() const {
	return (attr.st_mode & S_IFMT) == S_IFSOCK;
}

size_t S3FS_Obj::size() const {
	return attr.st_size;
}

void S3FS_Obj::setSize(size_t s) {
	attr.st_size = s;
}

