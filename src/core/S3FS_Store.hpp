#include <QObject>
#include <QCryptographicHash>
#include "Keyval.hpp"

class S3FS_Store: public QObject {
	Q_OBJECT

public:
	S3FS_Store(const QByteArray &bucket, QObject *parent = 0);
	~S3FS_Store();

private:
	QString kv_location;
	Keyval kv; // local cache
	QByteArray bucket;
	QCryptographicHash::Algorithm algo;
};

