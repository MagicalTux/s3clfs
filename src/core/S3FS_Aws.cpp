#include "S3FS_Aws.hpp"
#include <QStandardPaths>
#include <QFile>
#include <QSettings>
#include <QMessageAuthenticationCode>

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

QByteArray S3FS_Aws::sign(const QByteArray &string) {
	// generate signature of string as "AWS AWSAccessKeyId:Signature"
	QByteArray sig = QMessageAuthenticationCode::hash(string, key, QCryptographicHash::Sha1);

	return QByteArray("AWS ")+id+":"+sig.toBase64();
}

