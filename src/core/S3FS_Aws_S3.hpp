#include "S3FS_Aws.hpp"
#include <QObject>
#include <QNetworkRequest>

class QNetworkReply;
class QBuffer;

class S3FS_Aws_S3: public QObject {
	Q_OBJECT
public:
	~S3FS_Aws_S3();

	static S3FS_Aws_S3 *getFile(const QByteArray &bucket, const QByteArray &path, S3FS_Aws *aws);
	static S3FS_Aws_S3 *listFiles(const QByteArray &bucket, const QByteArray &path, S3FS_Aws *aws);
	static S3FS_Aws_S3 *putFile(const QByteArray &bucket, const QByteArray &path, const QByteArray &data, S3FS_Aws *aws);
	static S3FS_Aws_S3 *deleteFile(const QByteArray &bucket, const QByteArray &path, S3FS_Aws *aws);

	const QByteArray &body() const;

	QStringList parseListFiles(bool &need_more) const;
	S3FS_Aws_S3 *listMoreFiles(const QByteArray &path, const QStringList &); // continue listing if parseListFiles said need_more=true

public slots:
	void requestFinished();
	void requestStarted(QNetworkReply*);

signals:
	void finished(S3FS_Aws_S3*);

private:
	S3FS_Aws_S3(const QByteArray &bucket, S3FS_Aws*);
	bool getFile(const QByteArray &path);
	bool listFiles(const QByteArray &path, const QByteArray &resume);
	bool putFile(const QByteArray &path, const QByteArray &data);
	bool deleteFile(const QByteArray &path);

	void signRequest(const QByteArray &verb = QByteArrayLiteral("GET"));
	void connectReply();

	QByteArray bucket;
	QByteArray reply_body;
	QByteArray request_body;
	S3FS_Aws *aws;
	QNetworkRequest request;
	QNetworkReply *reply;
	QBuffer *request_body_buffer;
};

