#include "S3FS_Aws.hpp"
#include <QStandardPaths>
#include <QFile>
#include <QSettings>

S3FS_Aws::S3FS_Aws(QObject *parent): QObject(parent) {
	QString path = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)+"/aws/credentials";
	if (!QFile::exists(path)) {
		qCritical("WARNING! AWS configuration is missing, your data WILL NOT be saved and will be LOST on umount.\nPlease create a configuration file %s with the required information.", qPrintable(path));
		return;
	}

	QSettings settings(path, QSettings::IniFormat);
	qDebug("loc = %s", qPrintable(path));
}
