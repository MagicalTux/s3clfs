#include <QByteArray>
#include <QVariant>
#include <sys/stat.h>

class S3FS_Obj {
public:
	S3FS_Obj();
	S3FS_Obj(const QByteArray&);

	QByteArray encode() const;

	void makeRoot(); // configure object to act as root

private:
	struct stat attr;
};

