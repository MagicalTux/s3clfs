#include "S3FS_Aws.hpp"
#include <QObject>
#include <QUrl>

#pragma once

class QNetworkReply;

class S3FS_Aws_SQS: public QObject {
	Q_OBJECT
public:
	S3FS_Aws_SQS(const QByteArray &queue, S3FS_Aws *parent);

public slots:
	void readReply();

private:
	S3FS_Aws *aws;
	QUrl queue;
	QByteArray region;

	QNetworkReply *reply;
};

