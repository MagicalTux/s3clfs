#include <S3FS_Obj.hpp>
#include <unistd.h>
#include <sys/types.h>

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
	quint64 t = time(NULL);

	reset();
	attr.st_ino = 1;
	attr.st_mode = 0755 | S_IFDIR;
	attr.st_uid = getuid();
	attr.st_gid = getgid();
	attr.st_size = 0;
	attr.st_ctime = t;
	attr.st_mtime = t;
	attr.st_atime = t;
}

void S3FS_Obj::makeDir(quint64 ino, int mode, int uid, int gid) {
	quint64 t = time(NULL);

	reset();
	attr.st_ino = ino;
	attr.st_mode = (mode & ~S_IFMT) | S_IFDIR;
	attr.st_uid = uid;
	attr.st_gid = gid;
	attr.st_size = 0;
	attr.st_ctime = t;
	attr.st_mtime = t;
	attr.st_atime = t;
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
	STORE_VAL_T(ctime, quint64);
	STORE_VAL_T(mtime, qint64);
	STORE_VAL_T(atime, qint64);

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
	attr.st_ctime = attrs.value("ctime").toULongLong();
	attr.st_mtime = attrs.value("mtime").toULongLong();
	attr.st_atime = attrs.value("atime").toULongLong();

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

