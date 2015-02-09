#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>

#pragma once

class S3FS_Aws_S3;
class S3FS_Aws_SQS;
class QNetworkReply;

struct S3FS_Aws_Queue_Entry {
	QNetworkRequest req;
	QByteArray verb;
	QIODevice *data;
	QObject *sender; // sender slot requestStarted(QNetworkReply*) will be called once query starts
};

class S3FS_Aws: public QObject {
	Q_OBJECT
public:
	S3FS_Aws(QObject *parent = 0);
	bool isValid();
	const QByteArray &getAwsId() const;

public slots:
	void replyDestroyed(QObject *obj);

protected:
	QByteArray signV2(const QByteArray &string);
	QByteArray signV4(const QByteArray &string, const QByteArray &path, const QByteArray &timestamp, QByteArray &algo);
	QNetworkReply *reqV4(const QByteArray &verb, const QByteArray &subpath, QNetworkRequest req);
	void http(QObject *caller, const QByteArray &verb, const QNetworkRequest &req, QIODevice *data = 0);

	friend class S3FS_Aws_S3;
	friend class S3FS_Aws_SQS;

private:
	QByteArray id;
	QByteArray key;
	QNetworkAccessManager net;

	QSet<QNetworkReply*> http_running;
	QList<S3FS_Aws_Queue_Entry*> http_queue;
};

