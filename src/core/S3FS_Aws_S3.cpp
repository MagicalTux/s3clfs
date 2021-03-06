#include "S3FS_Aws_S3.hpp"
#include <QUrl>
#include <QUrlQuery>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QtXmlPatterns>

/*  S3ClFS - AWS S3 backed cluster filesystem
 *  Copyright (C) 2015 Mark Karpeles
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

S3FS_Aws_S3::S3FS_Aws_S3(const QByteArray &_bucket, S3FS_Aws *parent): QObject(parent) {
	bucket = _bucket;
	aws = parent;
	reply = 0;
	request_body_buffer = 0;
	verb = QByteArrayLiteral("GET"); // default
	subpath = aws->getBucketRegion(bucket)+"/s3";
}

S3FS_Aws_S3::~S3FS_Aws_S3() {
	if (reply) delete reply;
}

S3FS_Aws_S3 *S3FS_Aws_S3::getFile(const QByteArray &bucket, const QByteArray &path, S3FS_Aws *aws) {
	if (!aws->isValid()) return NULL;
	auto i = new S3FS_Aws_S3(bucket, aws);
	if (!i->getFile(path)) {
		delete i;
		return NULL;
	}
	return i;
}

void S3FS_Aws_S3::requestStarted(QNetworkReply *r) {
	reply = r;
	connectReply();
}

bool S3FS_Aws_S3::getFile(const QByteArray &path) {
	// NOTE: if user is on aws, ssl might not be required?
	QUrl url("https://"+bucket+".s3.amazonaws.com/"+path); // using bucketname.s3.amazonaws.com will ensure query is routed to appropriate region
	request = QNetworkRequest(url);
	request.setRawHeader("X-Amz-Content-SHA256", "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"); // sha256("")

	aws->httpV4(this, verb, subpath, request);
	return true;
}

S3FS_Aws_S3 *S3FS_Aws_S3::listFiles(const QByteArray &bucket, const QByteArray &path, S3FS_Aws *aws) {
	if (!aws->isValid()) return NULL;
	auto i = new S3FS_Aws_S3(bucket, aws);
	if (!i->listFiles(path, QByteArrayLiteral(""))) {
		delete i;
		return NULL;
	}
	return i;
}

S3FS_Aws_S3 *S3FS_Aws_S3::listMoreFiles(const QByteArray &path, const QStringList &latest) {
	auto i = new S3FS_Aws_S3(bucket, aws);
	if (!i->listFiles(path, latest.last().toUtf8())) {
		delete i;
		return NULL;
	}
	return i;
}

bool S3FS_Aws_S3::listFiles(const QByteArray &path, const QByteArray &resume) {
	QUrlQuery url_query;
	url_query.addQueryItem("prefix", QUrl::toPercentEncoding(path));
	if (!resume.isEmpty()) url_query.addQueryItem("marker", QUrl::toPercentEncoding(resume));
	QUrl url("https://"+bucket+".s3.amazonaws.com/");
	url.setQuery(url_query);

	request = QNetworkRequest(url);
	request.setRawHeader("X-Amz-Content-SHA256", "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"); // sha256("")

	aws->httpV4(this, verb, subpath, request);
	return true;
}

S3FS_Aws_S3 *S3FS_Aws_S3::putFile(const QByteArray &bucket, const QByteArray &path, const QByteArray &data, S3FS_Aws *aws) {
	if (!aws->isValid()) return NULL;
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
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/octet-stream"); // RFC 2046
	request.setRawHeader("Content-MD5", QCryptographicHash::hash(data, QCryptographicHash::Md5).toBase64());

	// keep request body around in case we need to retry
	request_body = data;

	verb = "PUT";

	aws->httpSlowV4(this, verb, subpath, request, request_body);
	return true;
}

S3FS_Aws_S3 *S3FS_Aws_S3::deleteFile(const QByteArray &bucket, const QByteArray &path, S3FS_Aws *aws) {
	if (!aws->isValid()) return NULL;
	auto i = new S3FS_Aws_S3(bucket, aws);
	if (!i->deleteFile(path)) {
		delete i;
		return NULL;
	}
	return i;
}

S3FS_Aws_S3 *S3FS_Aws_S3::deleteFile(S3FS_Aws_S3 *req) {
	if (!req->aws->isValid()) return NULL;
	auto i = new S3FS_Aws_S3(req->bucket, req->aws);
	if (!i->deleteFile(req->request.url().path().toUtf8())) {
		delete i;
		return NULL;
	}
	return i;
}

bool S3FS_Aws_S3::deleteFile(const QByteArray &path) {
	QUrl url("https://"+bucket+".s3.amazonaws.com/"+path);
	request = QNetworkRequest(url);
	verb = "DELETE";

	aws->httpV4(this, verb, subpath, request);
	return true;
}

void S3FS_Aws_S3::connectReply() {
	connect(reply, SIGNAL(finished()), this, SLOT(requestFinished()));
}

void S3FS_Aws_S3::requestFinished() {
	if (reply->error() == QNetworkReply::ContentNotFoundError) {
		// file was not found
		reply_body = QByteArray();
		finished(this);
		deleteLater();
		return;
	}
	if (reply->error() != QNetworkReply::NoError) {
		QByteArray response = reply->readAll();
		if (response.indexOf("AuthorizationHeaderMalformed")) {
			// likely wrong region, good one is in the msg
			QRegExp rx("<Region>([a-z0-9-]+)</Region>");
			if (rx.indexIn(response) != -1) {
				qDebug("Detected correct region %s for bucket, retrying...", qPrintable(rx.cap(1)));
				aws->setBucketRegion(bucket, rx.cap(1).toLatin1());
				subpath = rx.cap(1).toLatin1()+"/s3";
				QTimer::singleShot(1000, this, SLOT(retry()));
				return;
			}
		}
		qDebug("ERROR HTTP %s", reply->readAll().data());
		qDebug("%s, re-queuing", qPrintable(reply->errorString()));
		QTimer::singleShot(1000, this, SLOT(retry()));
		return;
	}
//	int http_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
	if (reply->hasRawHeader("location")) {
//		qDebug("**** REDIRECT TO %s", reply->rawHeader("location").data());
		request.setUrl(QUrl(reply->rawHeader("location")));
		QTimer::singleShot(1000, this, SLOT(retry()));
		return;
	}
	reply_body = reply->readAll();
//	qDebug("HTTP REPLY %d %s", http_code, reply_body.data());
	finished(this); // because we're in the same thread, signal will be called immediately
	deleteLater(); // so when this object will be erased will be later
}

void S3FS_Aws_S3::retry() {
	if (reply) {
		reply->deleteLater();
		reply = 0;
	}
	aws->httpV4(this, verb, subpath, request, request_body);
}

const QByteArray &S3FS_Aws_S3::body() const {
	return reply_body;
}

QStringList S3FS_Aws_S3::parseListFiles(bool &need_more) const {
	// Looks like Qt's way to parse XML document needs to involve some complex processing...
	// Anyway, this works, but it feels overly complex
	// (not mentionning support of xml namespaces in QXmlQuery kinda sucks)
	QBuffer device;
	device.setData(reply_body);
	device.open(QIODevice::ReadOnly);

	QXmlQuery query;
	query.bindVariable("reply", &device);
	query.setQuery("string(doc($reply)/*:ListBucketResult/*:IsTruncated)");
	QStringList istruncated_result;
	query.evaluateTo(&istruncated_result);

	if (istruncated_result.at(0).trimmed() == "true") {
		need_more = true;
	} else {
		need_more = false;
	}

	query.setQuery("doc($reply)/*:ListBucketResult/*:Contents/*:Key/text()");
	QXmlResultItems contents_res;
	query.evaluateTo(&contents_res);

	QStringList final_result;

	QXmlItem item(contents_res.next());
	while (!item.isNull()) {
		query.bindVariable("item", item);
		query.setQuery("$item");
		QString temp_value;
		query.evaluateTo(&temp_value);
		final_result << temp_value.trimmed();
		item = contents_res.next();
	}

	return final_result;
}

