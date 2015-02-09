#include "S3FS_Aws_SQS.hpp"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDateTime>
#include <QUrlQuery>
#include <QCryptographicHash>
#include <QBuffer>
#include <QtXmlPatterns>

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
//	qDebug("Got messages from the queue system");
	auto res = parseMessages(reply->readAll());
	foreach(auto msg, res) {
		auto doc = QJsonDocument::fromJson(msg.value("Body"));
		if (handleMessage(doc)) {
			QUrlQuery url_q;
			url_q.addQueryItem("Action", "DeleteMessage");
			url_q.addQueryItem("ReceiptHandle", QUrl::toPercentEncoding(msg.value("ReceiptHandle").trimmed()));

			QUrl call = queue;
			call.setQuery(url_q);
			QNetworkRequest req(call);
			aws->httpV4(this, "GET", region+"/sqs", req);
//			qDebug() << "\n\n TODO DELETE MSG \n\n" << msg.value("ReceiptHandle").trimmed();
			// TODO http://docs.aws.amazon.com/AWSSimpleQueueService/latest/APIReference/API_DeleteMessage.html
		}
	}
	reply->deleteLater();

	// make new request
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
}

bool S3FS_Aws_SQS::handleMessage(const QJsonDocument &doc) {
	auto msg = QJsonDocument::fromJson(doc.object().value("Message").toString().toUtf8());
	auto records = msg.object().value("Records").toArray();
	foreach(auto record, records) {
		auto s3 = record.toObject().value("s3").toObject();
		QString bucket_name = s3.value("bucket").toObject().value("name").toString();
		QString file = s3.value("object").toObject().value("key").toString();

		newFile(bucket_name, file);
	}
	return true;
}

void S3FS_Aws_SQS::requestStarted(QNetworkReply*r) {
	// actually we don't care about deletes
	connect(r, SIGNAL(finished()), r, SLOT(deleteLater()));
}

QList<QMap<QByteArray,QByteArray> > S3FS_Aws_SQS::parseMessages(const QByteArray &input) {
	QBuffer device;
	device.setData(input);
	device.open(QIODevice::ReadOnly);

	QXmlQuery query;
	query.bindVariable("input", &device);

	query.setQuery("doc($input)/*:ReceiveMessageResponse/*:ReceiveMessageResult/*:Message");
	QXmlResultItems contents_res;
	query.evaluateTo(&contents_res);

	QList<QMap<QByteArray,QByteArray> > final_result;

	// Elements: Body, ReceiptHandle, MD5OfBody, MessageId, Attribute(multiple)
	// Attributes: SenderId, ApproximateFirstReceiveTimestamp, ApproximateReceiveCount, SentTimestamp
	// (actually we don't really care about attributes)

	QXmlItem item(contents_res.next());
	while (!item.isNull()) {
		query.bindVariable("item", item);
		QString temp_value;
		QMap<QByteArray,QByteArray> tmp;

		#define GET_VALUE(_x) query.setQuery("$item/*:" #_x "/text()"); query.evaluateTo(&temp_value); tmp.insert(#_x, temp_value.toUtf8())

		GET_VALUE(Body);
		GET_VALUE(ReceiptHandle);
		GET_VALUE(MD5OfBody);
		GET_VALUE(MessageId);

//		query.setQuery("$item/*:Body/text()");
//		query.evaluateTo(&temp_value);

		final_result << tmp;
		item = contents_res.next();
	}

	return final_result;
}

