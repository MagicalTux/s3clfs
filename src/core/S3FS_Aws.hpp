#include <QObject>
#include <QNetworkAccessManager>

class S3FS_Aws_S3;

class S3FS_Aws: public QObject {
	Q_OBJECT
public:
	S3FS_Aws(QObject *parent = 0);
	bool isValid();

protected:
	QByteArray sign(const QByteArray &string);
	QNetworkAccessManager net;

	friend class S3FS_Aws_S3;

private:
	QByteArray id;
	QByteArray key;
};

