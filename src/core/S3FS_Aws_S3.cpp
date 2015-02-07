#include "S3FS_Aws_S3.hpp"
#include <QDateTime>
#include <QUrl>
#include <QUrlQuery>
#include <QNetworkRequest>
#include <QNetworkReply>

S3FS_Aws_S3::S3FS_Aws_S3(const QByteArray &_bucket, S3FS_Aws *parent): QObject(parent) {
	bucket = _bucket;
	aws = parent;
	reply = 0;
}

S3FS_Aws_S3 *S3FS_Aws_S3::getFile(const QByteArray &bucket, const QByteArray &path, S3FS_Aws *aws) {
	auto i = new S3FS_Aws_S3(bucket, aws);
	if (!i->getFile(path)) {
		delete i;
		return NULL;
	}
	return i;
}

bool S3FS_Aws_S3::getFile(const QByteArray &path) {
	// NOTE: if user is on aws, ssl might not be required?
	QUrl url("https://"+bucket+".s3.amazonaws.com/"+path); // using bucketname.s3.amazonaws.com will ensure query is routed to appropriate region
	request = QNetworkRequest(url);
	signRequest();

	reply = aws->net.get(request);
	if (!reply) return false;
	connectReply();
	return true;
}

S3FS_Aws_S3 *S3FS_Aws_S3::listFiles(const QByteArray &bucket, const QByteArray &path, S3FS_Aws *aws) {
	auto i = new S3FS_Aws_S3(bucket, aws);
	if (!i->listFiles(path, QByteArrayLiteral(""))) {
		delete i;
		return NULL;
	}
	return i;
}

bool S3FS_Aws_S3::listFiles(const QByteArray &path, const QByteArray &resume) {
	QUrlQuery url_query;
	url_query.addQueryItem("prefix", path);
	if (!resume.isEmpty()) url_query.addQueryItem("marker", resume);
	QUrl url("https://"+bucket+".s3.amazonaws.com/");
	url.setQuery(url_query);

	request = QNetworkRequest(url);
	signRequest();

	reply = aws->net.get(request);
	if (!reply) return false;
	connectReply();
	return true;
}

S3FS_Aws_S3 *S3FS_Aws_S3::putFile(const QByteArray &bucket, const QByteArray &path, const QByteArray &data, S3FS_Aws *aws) {
	auto i = new S3FS_Aws_S3(bucket, aws);
	if (!i->putFile(path, data)) {
		delete i;
		return NULL;
	}
	return i;
}

bool S3FS_Aws_S3::putFile(const QByteArray &path, const QByteArray &data) {
	QUrl url("https://"+bucket+".s3.amazonaws.com/"+path);
	request = QNetworkRequest(url);
//	request.setHeader
}

void S3FS_Aws_S3::connectReply() {
	connect(reply, SIGNAL(finished()), this, SLOT(requestFinished()));
}

void S3FS_Aws_S3::signRequest(const QByteArray &verb) {
	qDebug("AWS S3: Request %s", request.url().toString().toLatin1().data());
	QByteArray date_header = QDateTime::currentDateTime().toString(QStringLiteral("ddd, dd MMM yyyy hh:mm:ss t")).toLatin1();

	QByteArray sign = verb+"\n";
	if (request.hasRawHeader("Content-MD5")) {
		sign += request.rawHeader("Content-MD5")+"\n";
	} else {
		sign += "\n";
	}
	if (request.hasHeader(QNetworkRequest::ContentTypeHeader)) {
		sign += request.header(QNetworkRequest::ContentTypeHeader)+"\n";
	} else {
		sign += "\n";
	}
	request.setRawHeader("Date", date_header);
	sign += date_header+"\n";

	sign += "/" + bucket + request.url().path(QUrl::FullyEncoded); // TODO also append query string if one of ?versioning ?location ?acl ?torrent ?lifecycle ?versionid

	request.setRawHeader("Authorization", aws->signV2(sign));
}

void S3FS_Aws_S3::requestFinished() {
	reply_body = reply->readAll();
	qDebug("HTTP REPLY %s", reply_body.data());
	finished(this); // because we're in the same thread, signal will be called immediately
	deleteLater(); // so when this object will be erased will be later
}

