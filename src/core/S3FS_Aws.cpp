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

bool S3FS_Aws::isValid() {
	return !id.isEmpty();
}

QByteArray S3FS_Aws::signV2(const QByteArray &string) {
	// generate signature of string as "AWS AWSAccessKeyId:Signature"
	QByteArray sig = QMessageAuthenticationCode::hash(string, key, QCryptographicHash::Sha1);

	return QByteArray("AWS ")+id+":"+sig.toBase64();
}

void S3FS_Aws::http(QObject *caller, const QByteArray &verb, const QNetworkRequest req, QIODevice *data) {
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

