#include "S3FS_Aws_SQS.hpp"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDateTime>
#include <QUrlQuery>
#include <QCryptographicHash>

S3FS_Aws_SQS::S3FS_Aws_SQS(const QByteArray &_queue, S3FS_Aws *parent): QObject(parent) {
	aws = parent;
	reply = 0;

	queue = QUrl(_queue);
	if (!queue.isValid()) {
		qFatal("Invalid url provided for SQS queue");
	}
	// example sqs.ap-northeast-1.amazonaws.com
	QRegExp rx("sqs\\.([a-z0-9-]+)\\.amazonaws\\.com");
	if (!rx.exactMatch(queue.host())) {
		qFatal("Invalid url provided for SQS queue, host part invalid");
	}
	region = rx.cap(1).toLatin1();

	QUrlQuery url_q;
	url_q.addQueryItem("Action", "ReceiveMessage");
	url_q.addQueryItem("WaitTimeSeconds", "20");
	url_q.addQueryItem("MaxNumberOfMessages", "10");
	url_q.addQueryItem("VisibilityTimeout", "30");
	url_q.addQueryItem("AttributeName", "All");
	url_q.addQueryItem("Version", "2012-11-05");

	QUrl call = queue;
	call.setQuery(url_q);

	QNetworkRequest req(call);

	reply = aws->reqV4("GET", region+"/sqs", req);
	connect(reply, SIGNAL(finished()), this, SLOT(readReply()));

	// TODO
}

void S3FS_Aws_SQS::readReply() {
	qDebug("HTTP SQS REPLY: %s", reply->readAll().data());
	reply->deleteLater();
	reply = 0;
}

