#include <QObject>
#include <QCryptographicHash>
#include "Keyval.hpp"
#include <QVariant>

class S3FS_Obj; // inode
class S3FS_Aws;
class S3FS_Aws_S3;
class Callback;
class S3FS_Store_MetaIterator;

class S3FS_Store: public QObject {
	Q_OBJECT

public:
	S3FS_Store(const QByteArray &bucket, QObject *parent = 0);
	~S3FS_Store();

	// filesystem config
	const QVariantMap &getConfig();
	bool readConfig();
	bool setConfig(const QVariantMap&);

	// inodes
	bool hasInode(quint64);
	bool storeInode(const S3FS_Obj&);
	S3FS_Obj getInode(quint64);
	bool hasInodeLocally(quint64);
	void callbackOnInodeCached(quint64, Callback*);
	void sendInodeToAws(quint64);

	// blocks
	QByteArray writeBlock(const QByteArray &buf);
	QByteArray readBlock(const QByteArray &buf);
	bool hasBlockLocally(const QByteArray&);
	void callbackOnBlockCached(const QByteArray&, Callback*);

	// inode meta
	bool hasInodeMeta(quint64 ino, const QByteArray &key);
	QByteArray getInodeMeta(quint64 ino, const QByteArray &key);
	bool setInodeMeta(quint64 ino, const QByteArray &key, const QByteArray &value);
	S3FS_Store_MetaIterator *getInodeMetaIterator(quint64 ino);
	bool removeInodeMeta(quint64 ino, const QByteArray &key);
	bool clearInodeMeta(quint64 ino);

signals:
	void ready();

public slots:
	void readyStateWithoutAws();
	void receivedFormatFile(S3FS_Aws_S3*);
	void receivedInodeList(S3FS_Aws_S3*);

private:
	QString kv_location;
	Keyval kv; // local cache
	QByteArray bucket;
	QCryptographicHash::Algorithm algo;
	QVariantMap config;
	S3FS_Aws *aws;
};

