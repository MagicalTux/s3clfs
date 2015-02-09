#include "S3FS_Aws.hpp"
#include <QStandardPaths>
#include <QFile>
#include <QSettings>
#include <QMessageAuthenticationCode>
#include <QNetworkReply>

S3FS_Aws::S3FS_Aws(QObject *parent): QObject(parent) {
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

//	qDebug("** AWS ** StringToSign = %s", StringToSign.data());

	return QMessageAuthenticationCode::hash(StringToSign, buffer, QCryptographicHash::Sha256);
}

QNetworkReply *S3FS_Aws::reqV4(const QByteArray &verb, const QByteArray &subpath, QNetworkRequest req) {
	QByteArray timestamp = QDateTime::currentDateTimeUtc().toString("yyyyMMddTHHmmssZ").toLatin1();

	req.setRawHeader("X-Amz-Date", timestamp);

	// compute canonical url query
	QByteArray canonical_query_string = req.url().query(QUrl::FullyEncoded).toLatin1();
	auto q_split = canonical_query_string.split('&');
	std::sort(q_split.begin(), q_split.end());
	canonical_query_string = q_split.join('&');

	QMap<QByteArray,QByteArray> headers;
	auto raw_headers_list = req.rawHeaderList();
	QByteArray tmp;
	foreach(tmp, raw_headers_list)
		headers.insert(tmp.toLower(), req.rawHeader(tmp));

	// check other headers
	headers.insert("host", req.url().host().toLatin1());
	if (req.header(QNetworkRequest::ContentTypeHeader).isValid())
		headers.insert("content-type", req.header(QNetworkRequest::ContentTypeHeader).toByteArray());
	if (req.header(QNetworkRequest::ContentLengthHeader).isValid())
		headers.insert("content-length", req.header(QNetworkRequest::ContentLengthHeader).toByteArray());

	// OK, so...
	QByteArray CanonicalRequest = verb + QByteArrayLiteral("\n");
	CanonicalRequest += req.url().path() + QByteArrayLiteral("\n");
	CanonicalRequest += canonical_query_string + QByteArrayLiteral("\n");
	// headers
	for(auto i = headers.begin(); i != headers.end(); i++)
		CanonicalRequest += i.key()+":"+i.value()+QByteArrayLiteral("\n");
	CanonicalRequest += QByteArrayLiteral("\n");

	CanonicalRequest += headers.keys().join(";") + QByteArrayLiteral("\n");
	CanonicalRequest += QCryptographicHash::hash("", QCryptographicHash::Sha256).toHex();

	// generate signature
	QByteArray algo;
	QByteArray path = timestamp.left(8) + "/"+subpath+"/aws4_request";
	QByteArray sign = signV4(CanonicalRequest, path, timestamp, algo);

	QByteArray auth_header = algo+" Credential="+id+"/"+path+", SignedHeaders=host;x-amz-date, Signature="+sign.toHex();

	req.setRawHeader("Authorization", auth_header);

	qDebug("auth header = %s", auth_header.data());

	return net.sendCustomRequest(req, verb);
}

void S3FS_Aws::http(QObject *caller, const QByteArray &verb, const QNetworkRequest &req, QIODevice *data) {
	if (http_running.size() < 8) {
		auto reply = net.sendCustomRequest(req, verb, data);
		http_running.insert(reply);
		connect(reply, SIGNAL(destroyed(QObject*)), this, SLOT(replyDestroyed(QObject*)));
		qDebug("S3FS_Aws: queries status %d/8", http_running.size());
		QMetaObject::invokeMethod(caller, "requestStarted", Q_ARG(QNetworkReply*, reply));
		return;
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
	if (http_queue.size()) {
		auto q = http_queue.takeFirst();
		auto reply = net.sendCustomRequest(q->req, q->verb, q->data);
		http_running.insert(reply);
		connect(reply, SIGNAL(destroyed(QObject*)), this, SLOT(replyDestroyed(QObject*)));
		QMetaObject::invokeMethod(q->sender, "requestStarted", Q_ARG(QNetworkReply*, reply));
		delete q;
	}
	qDebug("S3FS_Aws: queries status %d/8 (%d in queue)", http_running.size(), http_queue.size());
}

