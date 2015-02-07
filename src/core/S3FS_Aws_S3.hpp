#include "S3FS_Aws.hpp"
#include <QObject>

class QNetworkReply;

class S3FS_Aws_S3: public QObject {
	Q_OBJECT
public:
	static S3FS_Aws_S3 *getFile(const QByteArray &bucket, const QByteArray &path, S3FS_Aws *aws);

public slots:
	void requestFinished();

signals:
	void finished(S3FS_Aws_S3*);

private:
	S3FS_Aws_S3(const QByteArray &bucket, S3FS_Aws*);
	bool getFile(const QByteArray &path);

	void signRequest(QNetworkRequest&, const QByteArray &verb = QByteArrayLiteral("GET"));
	void connectReply();

	QByteArray bucket;
	S3FS_Aws *aws;
	QNetworkReply *reply;
};

