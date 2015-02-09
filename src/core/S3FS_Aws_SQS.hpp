#include "S3FS_Aws.hpp"
#include <QObject>
#include <QUrl>

#pragma once

class QNetworkReply;

class S3FS_Aws_SQS: public QObject {
	Q_OBJECT
public:
	S3FS_Aws_SQS(const QByteArray &queue, S3FS_Aws *parent);

signals:
	void newFile(const QString&, const QString&);

public slots:
	void readReply();
	void requestStarted(QNetworkReply*);

protected:
	QList<QMap<QByteArray,QByteArray> > parseMessages(const QByteArray &input);
	bool handleMessage(const QJsonDocument &doc);

private:
	S3FS_Aws *aws;
	QUrl queue;
	QByteArray region;

	QNetworkReply *reply;
};

