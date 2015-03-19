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
#include <QObject>
#include <QLocalServer>
#include <QJsonDocument>

#pragma once

class S3FS;
class S3FS_Config;

class S3FS_Control: public QObject {
	Q_OBJECT
public:
	S3FS_Control(S3FS *parent, S3FS_Config *cfg);
	S3FS *getParent();

public slots:
	void acceptNewClient();

signals:
	void broadcast(const QJsonDocument&);

private:
	S3FS *parent;
	S3FS_Config *cfg;
	QLocalServer *server_socket;
};

