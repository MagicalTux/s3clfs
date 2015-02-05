#include <QObject>
#include <QCryptographicHash>
#include "Keyval.hpp"

class S3FS_Obj; // inode
class Callback;
class S3FS_Store_MetaIterator;

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

	bool hasInodeMeta(quint64 ino, const QByteArray &key);
	QByteArray getInodeMeta(quint64 ino, const QByteArray &key);
	bool setInodeMeta(quint64 ino, const QByteArray &key, const QByteArray &value);
	S3FS_Store_MetaIterator *getInodeMetaIterator(quint64 ino);

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

