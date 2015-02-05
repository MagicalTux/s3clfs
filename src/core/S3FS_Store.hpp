#include <QObject>
#include <QCryptographicHash>
#include "Keyval.hpp"

class S3FS_Obj; // inode
class Callback;

class S3FS_Store: public QObject {
	Q_OBJECT

public:
	S3FS_Store(const QByteArray &bucket, QObject *parent = 0);
	~S3FS_Store();

	// inodes
	bool hasInode(quint64);
	bool storeInode(const S3FS_Obj&);
	S3FS_Obj getInode(quint64);
	bool hasInodeLocally(quint64);
	void callbackOnInodeCached(quint64, Callback*);

signals:
	void ready();

public slots:
	void test_setready();

private:
	QString kv_location;
	Keyval kv; // local cache
	QByteArray bucket;
	QCryptographicHash::Algorithm algo;
};

