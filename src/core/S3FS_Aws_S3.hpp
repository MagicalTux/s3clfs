#include "S3FS_Aws.hpp"
#include <QObject>
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

class QNetworkReply;
class QBuffer;

class S3FS_Aws_S3: public QObject {
	Q_OBJECT
public:
	~S3FS_Aws_S3();

	static S3FS_Aws_S3 *getFile(const QByteArray &bucket, const QByteArray &path, S3FS_Aws *aws);
	static S3FS_Aws_S3 *listFiles(const QByteArray &bucket, const QByteArray &path, S3FS_Aws *aws);
	static S3FS_Aws_S3 *putFile(const QByteArray &bucket, const QByteArray &path, const QByteArray &data, S3FS_Aws *aws, bool slow = false);
	static S3FS_Aws_S3 *deleteFile(const QByteArray &bucket, const QByteArray &path, S3FS_Aws *aws);

	const QByteArray &body() const;

	QStringList parseListFiles(bool &need_more) const;
	S3FS_Aws_S3 *listMoreFiles(const QByteArray &path, const QStringList &); // continue listing if parseListFiles said need_more=true

public slots:
	void requestFinished();
	void requestStarted(QNetworkReply*);
	void retry();

signals:
	void finished(S3FS_Aws_S3*);

private:
	S3FS_Aws_S3(const QByteArray &bucket, S3FS_Aws*);
	bool getFile(const QByteArray &path);
	bool listFiles(const QByteArray &path, const QByteArray &resume);
	bool putFile(const QByteArray &path, const QByteArray &data, bool slow);
	bool deleteFile(const QByteArray &path);

	void connectReply();

	QByteArray bucket;
	QByteArray subpath; // region/s3
	QByteArray reply_body;
	QByteArray request_body;
	QByteArray verb;
	S3FS_Aws *aws;
	QNetworkRequest request;
	QNetworkReply *reply;
	QBuffer *request_body_buffer;
};

