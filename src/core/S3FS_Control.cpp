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
#include "S3FS_Control.hpp"
#include "S3FS_Control_Client.hpp"
#include "S3FS.hpp"
#include "S3FS_Config.hpp"

S3FS_Control::S3FS_Control(S3FS *_parent, S3FS_Config *_cfg): QObject(_parent) {
	parent = _parent;
	cfg = _cfg;
	server_socket = new QLocalServer(this);
#if QT_VERSION >= 0x050400
	// there is a bug in older Qt where setting a socket option will cause the generation to fail if the socket path is absolute
	// see: https://qt.gitorious.org/qt/qtbase/commit/320360131559df76ba1f635b665659f57e147665
	server_socket->setSocketOptions(QLocalServer::UserAccessOption);
#endif
	connect(server_socket, SIGNAL(newConnection()), this, SLOT(acceptNewClient()));

	QString sock_loc = cfg->controlSocket();
	if (sock_loc.isEmpty()) {
		if (cfg->cachePath().isEmpty()) {
			sock_loc = QDir::temp().filePath(QString("s3clfs-")+cfg->bucket()+".sock");
		} else {
			sock_loc = cfg->cachePath()+".sock";
		}
	}

	if (!server_socket->listen(sock_loc)) {
		qCritical("S3FS_Control: Failed to create control socket %s: %s, remote control will not be available", qPrintable(sock_loc), qPrintable(server_socket->errorString()));
		return;
	}
}

void S3FS_Control::acceptNewClient() {
	while(server_socket->hasPendingConnections()) {
		auto socket = server_socket->nextPendingConnection();
		if (!socket) break;
		auto client = new S3FS_Control_Client(this, socket);
		connect(this, SIGNAL(broadcast(const QJsonDocument&)), client, SLOT(send(const QJsonDocument&)));
	}
}

S3FS *S3FS_Control::getParent() {
	return parent;
}
