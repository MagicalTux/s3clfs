#include "S3FS_Aws.hpp"
#include <QObject>
#include <QUrl>

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

