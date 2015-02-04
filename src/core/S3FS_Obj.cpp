#include <S3FS_Obj.hpp>

S3FS_Obj::S3FS_Obj() {
	memset(&attr, 0, sizeof(attr));

	// some default settings
	attr.st_dev = 0x1337;
	attr.st_ino = 0;
	attr.st_mode = 0755 | S_IFDIR;
	attr.st_nlink = 1;
	attr.st_blksize = 512;
}

void S3FS_Obj::makeRoot() {
	quint64 t = time(NULL);

	attr.st_ino = 1;
	attr.st_mode = 0755 | S_IFDIR;
	attr.st_nlink = 1;
	attr.st_blksize = 512;
	attr.st_uid = 0;
	attr.st_gid = 0;
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

