#include <QByteArray>
#include <QVariant>
#include <sys/stat.h>

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
	bool decode(const QByteArray &);

	const struct stat &constAttr() const;
	void setAttr(const struct stat &s);

private:
	struct stat attr;
};

