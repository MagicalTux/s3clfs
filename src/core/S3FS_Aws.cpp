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
#include "S3FS_Aws.hpp"
#include <QStandardPaths>
#include <QFile>
#include <QSettings>
#include <QMessageAuthenticationCode>
#include <QNetworkReply>
#include <QBuffer>
#if QT_VERSION < 0x050300
#include <contrib/QByteArrayList.hpp>
#endif

S3FS_Aws::S3FS_Aws(S3FS_Config *_cfg, QObject *parent): QObject(parent) {
	cfg = _cfg;
	overload_status = false;
	QString path = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)+"/aws/credentials";
	if (!QFile::exists(path)) {
		qCritical("WARNING! AWS configuration is missing, your data WILL NOT be saved and will be LOST on umount.\nPlease create a configuration file %s with the required information.", qPrintable(path));
		return;
	}

	QSettings settings(path, QSettings::IniFormat);

	id = settings.value("default/aws_access_key_id").toByteArray();
	key = settings.value("default/aws_secret_access_key").toByteArray();

	if (id.isEmpty()) {
		qCritical("WARNING! AWS configuration is missing, your data WILL NOT be saved and will be LOST on umount.\nPlease create a configuration file %s with the required information.", qPrintable(path));
		return;
	}

	connect(&status_timer, &QTimer::timeout, this, &S3FS_Aws::showStatus);
	status_timer.setSingleShot(false);
	status_timer.start(5000);
}

const QByteArray &S3FS_Aws::getAwsId() const {
	return id;
}

bool S3FS_Aws::isValid() {
	return !id.isEmpty();
}

QByteArray S3FS_Aws::signV2(const QByteArray &string) {
	// generate signature of string as "AWS AWSAccessKeyId:Signature"
	QByteArray sig = QMessageAuthenticationCode::hash(string, key, QCryptographicHash::Sha1);

	return QByteArray("AWS ")+id+":"+sig.toBase64();
}

QByteArray S3FS_Aws::signV4(const QByteArray &string, const QByteArray &path, const QByteArray &timestamp, QByteArray &algo) {
	// generate the required elements to generate a V4 signature
	QByteArray buffer = QByteArrayLiteral("AWS4") + key;
	auto path_split = path.split('/');
	QByteArray elem;
	foreach(elem, path_split)
		buffer = QMessageAuthenticationCode::hash(elem, buffer, QCryptographicHash::Sha256);

	algo = QByteArrayLiteral("AWS4-HMAC-SHA256");
	QByteArray StringToSign = QByteArrayLiteral("AWS4-HMAC-SHA256") + QByteArrayLiteral("\n");
	StringToSign += timestamp + QByteArrayLiteral("\n");
	StringToSign += path + QByteArrayLiteral("\n"); // date/region/service/aws4_request
	StringToSign += QCryptographicHash::hash(string, QCryptographicHash::Sha256).toHex();

	return QMessageAuthenticationCode::hash(StringToSign, buffer, QCryptographicHash::Sha256);
}

QNetworkReply *S3FS_Aws::reqV4(const QByteArray &verb, const QByteArray &subpath, QNetworkRequest req, const QByteArray &data) {
	QByteArray timestamp = QDateTime::currentDateTimeUtc().toString("yyyyMMddTHHmmssZ").toLatin1();
	QByteArray content_sha256 = QCryptographicHash::hash(data, QCryptographicHash::Sha256).toHex();

	// at the very minimum, we need this header
	req.setRawHeader("X-Amz-Date", timestamp);
	req.setRawHeader("X-Amz-Content-SHA256", content_sha256);

	// compute canonical url query
	QByteArray canonical_query_string = req.url().query(QUrl::FullyEncoded).toLatin1();
	QByteArrayList q_split = canonical_query_string.split('&');
	std::sort(q_split.begin(), q_split.end());
	canonical_query_string = q_split.join('&');

	// gather headers that needs to be signed
	QMap<QByteArray,QByteArray> headers;
	auto raw_headers_list = req.rawHeaderList();
	foreach(auto tmp, raw_headers_list)
		headers.insert(tmp.toLower(), req.rawHeader(tmp));

	// check also other headers
	headers.insert("host", req.url().host().toLatin1());
	if (req.header(QNetworkRequest::ContentTypeHeader).isValid())
		headers.insert("content-type", req.header(QNetworkRequest::ContentTypeHeader).toByteArray());
	if (req.header(QNetworkRequest::ContentLengthHeader).isValid())
		headers.insert("content-length", req.header(QNetworkRequest::ContentLengthHeader).toByteArray());

	// Generate the canonical request
	QByteArray CanonicalRequest = verb + QByteArrayLiteral("\n"); // verb
	CanonicalRequest += req.url().path() + QByteArrayLiteral("\n"); // path
	CanonicalRequest += canonical_query_string + QByteArrayLiteral("\n"); // query string (sorted)
	// add the headers
	for(auto i = headers.begin(); i != headers.end(); i++)
		CanonicalRequest += i.key()+":"+i.value()+QByteArrayLiteral("\n");
	CanonicalRequest += QByteArrayLiteral("\n"); // an empty line

#if QT_VERSION < 0x050300
	CanonicalRequest += QByteArrayList(headers.keys()).join(';') + QByteArrayLiteral("\n"); // list of signed headers... 
#else
	CanonicalRequest += headers.keys().join(';') + QByteArrayLiteral("\n"); // list of signed headers... 
#endif
	CanonicalRequest += content_sha256; // and hash of the request body

//	qDebug("CANONICAL REQUEST: %s", CanonicalRequest.data());

	// call signv4 to generate the signature
	QByteArray algo;
	QByteArray path = timestamp.left(8) + "/"+subpath+"/aws4_request";
	QByteArray sign = signV4(CanonicalRequest, path, timestamp, algo);

	// and put it in the Authorization header
#if QT_VERSION < 0x050300
	QByteArray auth_header = algo+" Credential="+id+"/"+path+", SignedHeaders="+QByteArrayList(headers.keys()).join(';')+", Signature="+sign.toHex();
#else
	QByteArray auth_header = algo+" Credential="+id+"/"+path+", SignedHeaders="+headers.keys().join(";")+", Signature="+sign.toHex();
#endif
	req.setRawHeader("Authorization", auth_header);

//	qDebug("Authorization: %s", auth_header.data());

	QBuffer *data_dev = 0;
	if (!data.isEmpty()) {
		data_dev = new QBuffer();
		data_dev->setData(data);
		data_dev->open(QIODevice::ReadOnly);
	}
	auto res = net.sendCustomRequest(req, verb, data_dev);

	if (data_dev) data_dev->setParent(res); // ensures QBuffer() will die

	return res;
}

void S3FS_Aws::httpV4(QObject *caller, const QByteArray &verb, const QByteArray &subpath, const QNetworkRequest &req, const QByteArray &data) {
//	qDebug("AWS: Request(v4) %s %s", verb.data(), req.url().toString().toLatin1().data());
	if (http_running.size() < 8) {
		auto reply = reqV4(verb, subpath, req, data);
		http_running.insert(reply);
		connect(reply, SIGNAL(destroyed(QObject*)), this, SLOT(replyDestroyed(QObject*)));
//		qDebug("S3FS_Aws: queries status %d/8", http_running.size());
		QMetaObject::invokeMethod(caller, "requestStarted", Q_ARG(QNetworkReply*, reply));
		return;
	}
	if ((!overload_status) && (http_queue.size() > 10000)) {
		// signal overload
		overload_status = true;
		overloadStatus(overload_status);
	}
	// queue this
	auto q = new S3FS_Aws_Queue_Entry;
	q->req = req;
	q->verb = verb;
	q->subpath = subpath;
	q->raw_data = data;
	q->sender = caller;
	q->sign = 4;
	http_queue.append(q);
}

void S3FS_Aws::http(QObject *caller, const QByteArray &verb, const QNetworkRequest &req, QIODevice *data) {
//	qDebug("AWS: Request %s %s", verb.data(), req.url().toString().toLatin1().data());
	if (http_running.size() < 8) {
		auto reply = net.sendCustomRequest(req, verb, data);
		http_running.insert(reply);
		connect(reply, SIGNAL(destroyed(QObject*)), this, SLOT(replyDestroyed(QObject*)));
//		qDebug("S3FS_Aws: queries status %d/8", http_running.size());
		QMetaObject::invokeMethod(caller, "requestStarted", Q_ARG(QNetworkReply*, reply));
		return;
	}
	if ((!overload_status) && (http_queue.size() > 10000)) {
		// signal overload
		overload_status = true;
		overloadStatus(overload_status);
	}
	// queue this
	auto q = new S3FS_Aws_Queue_Entry;
	q->req = req;
	q->verb = verb;
	q->data = data;
	q->sender = caller;
	http_queue.append(q);
}

void S3FS_Aws::replyDestroyed(QObject *obj) {
	http_running.remove((QNetworkReply*)obj);
	if ((overload_status) && (http_queue.size() < 5000)) {
		// signal overload
		overload_status = false;
		overloadStatus(overload_status);
	}
	if (http_queue.size()) {
		auto q = http_queue.takeFirst();
		QNetworkReply *reply;
		switch(q->sign) {
			case 4: // signV4
				reply = reqV4(q->verb, q->subpath, q->req, q->raw_data);
				break;
			default:
				reply = net.sendCustomRequest(q->req, q->verb, q->data);
		}
		http_running.insert(reply);
		connect(reply, SIGNAL(destroyed(QObject*)), this, SLOT(replyDestroyed(QObject*)));
		QMetaObject::invokeMethod(q->sender, "requestStarted", Q_ARG(QNetworkReply*, reply));
		delete q;
	}
//	qDebug("S3FS_Aws: queries status %d/8 (%d in queue)", http_running.size(), http_queue.size());
}

void S3FS_Aws::showStatus() {
	qDebug("S3FS_Aws: queries status %d/8 (%d in queue)", http_running.size(), http_queue.size());
}

QByteArray S3FS_Aws::getBucketRegion(const QByteArray&bucket) {
	if (aws_bucket_region.contains(bucket)) {
		return aws_bucket_region.value(bucket);
	}
	return "unknown";
}

void S3FS_Aws::setBucketRegion(const QByteArray&bucket, const QByteArray&region) {
	aws_bucket_region.insert(bucket, region);
}

