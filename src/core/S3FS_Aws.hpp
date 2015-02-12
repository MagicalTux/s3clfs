#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>

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

#pragma once

class S3FS_Config;
class S3FS_Aws_S3;
class S3FS_Aws_SQS;
class QNetworkReply;

struct S3FS_Aws_Queue_Entry {
	QNetworkRequest req;
	QByteArray verb;
	QIODevice *data;
	QObject *sender; // sender slot requestStarted(QNetworkReply*) will be called once query starts
	QByteArray subpath;
	QByteArray raw_data;
	int sign;
};

class S3FS_Aws: public QObject {
	Q_OBJECT
public:
	S3FS_Aws(S3FS_Config *cfg, QObject *parent = 0);
	bool isValid();
	const QByteArray &getAwsId() const;

public slots:
	void replyDestroyed(QObject *obj);

protected:
	QByteArray signV2(const QByteArray &string);
	QByteArray signV4(const QByteArray &string, const QByteArray &path, const QByteArray &timestamp, QByteArray &algo);
	QNetworkReply *reqV4(const QByteArray &verb, const QByteArray &subpath, QNetworkRequest req, const QByteArray &data = QByteArray());
	void http(QObject *caller, const QByteArray &verb, const QNetworkRequest &req, QIODevice *data = 0);
	void httpV4(QObject *caller, const QByteArray &verb, const QByteArray &subpath, const QNetworkRequest &req, const QByteArray &data = QByteArray());
	QByteArray getBucketRegion(const QByteArray&bucket);
	void setBucketRegion(const QByteArray&bucket, const QByteArray&region);

	friend class S3FS_Aws_S3;
	friend class S3FS_Aws_SQS;

private:
	QByteArray id;
	QByteArray key;
	QNetworkAccessManager net;

	QSet<QNetworkReply*> http_running;
	QList<S3FS_Aws_Queue_Entry*> http_queue;
	QMap<QByteArray,QByteArray> aws_bucket_region;
	S3FS_Config *cfg;
};

