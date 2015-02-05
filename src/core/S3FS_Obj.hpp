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
	mode_t getFiletype() const;

	void reset();
	void makeRoot(); // configure object to act as root
	bool decode(const QByteArray &);

	const struct stat &constAttr() const;

private:
	struct stat attr;
};

