#include "S3FS_Aws.hpp"
#include <QObject>
#include <QNetworkRequest>

class QNetworkReply;

class S3FS_Aws_S3: public QObject {
	Q_OBJECT
public:
	static S3FS_Aws_S3 *getFile(const QByteArray &bucket, const QByteArray &path, S3FS_Aws *aws);
	static S3FS_Aws_S3 *listFiles(const QByteArray &bucket, const QByteArray &path, S3FS_Aws *aws);
	static S3FS_Aws_S3 *putFile(const QByteArray &bucket, const QByteArray &path, const QByteArray &data, S3FS_Aws *aws);

public slots:
	void requestFinished();

signals:
	void finished(S3FS_Aws_S3*);

private:
	S3FS_Aws_S3(const QByteArray &bucket, S3FS_Aws*);
	bool getFile(const QByteArray &path);
	bool listFiles(const QByteArray &path, const QByteArray &resume);
	bool putFile(const QByteArray &path, const QByteArray &data);

	void signRequest(const QByteArray &verb = QByteArrayLiteral("GET"));
	void connectReply();

	QByteArray bucket;
	QByteArray reply_body;
	QByteArray request_body;
	S3FS_Aws *aws;
	QNetworkRequest request;
	QNetworkReply *reply;
};

