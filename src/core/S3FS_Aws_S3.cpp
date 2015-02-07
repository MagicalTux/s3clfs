#include "S3FS_Aws_S3.hpp"
#include <QDateTime>
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>

S3FS_Aws_S3::S3FS_Aws_S3(const QByteArray &_bucket, S3FS_Aws *parent): QObject(parent) {
	bucket = _bucket;
	aws = parent;
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
	QNetworkRequest req(url);
	signRequest(req);

	reply = aws->net.get(req);
	if (!reply) return false;
	connectReply();
	return true;
}

void S3FS_Aws_S3::connectReply() {
	connect(reply, SIGNAL(finished()), this, SLOT(requestFinished()));
}

void S3FS_Aws_S3::signRequest(QNetworkRequest&req, const QByteArray &verb) {
	QByteArray date_header = QDateTime::currentDateTime().toString(Qt::RFC2822Date).toLatin1();

	QByteArray sign = verb+"\n";
	if (req.hasRawHeader("Content-MD5")) {
		sign += req.rawHeader("Content-MD5")+"\n";
	} else {
		sign += "\n";
	}
	if (req.hasRawHeader("Content-Type")) {
		sign += req.rawHeader("Content-Type")+"\n";
	} else {
		sign += "\n";
	}
	req.setRawHeader("Date", date_header);
	sign += date_header+"\n";

	sign += "/" + bucket + req.url().path(QUrl::FullyEncoded); // TODO also append query string if one of ?versioning ?location ?acl ?torrent ?lifecycle ?versionid

	req.setRawHeader("Authorization", aws->sign(sign));
}

void S3FS_Aws_S3::requestFinished() {
	finished(this);
	deleteLater();
}

