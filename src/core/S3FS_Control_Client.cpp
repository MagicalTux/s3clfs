#include "S3FS_Control_Client.hpp"
#include <QJsonDocument>

S3FS_Control_Client::S3FS_Control_Client(S3FS_Control *_parent, QLocalSocket *_socket) {
	parent = _parent;
	socket = _socket;
}

void S3FS_Control_Client::send(const QJsonDocument &doc) {
	send(doc.toJson());
}

void S3FS_Control_Client::send(const QByteArray &data) {
	QDataStream(socket) << (quint32)data.length(); // 32 bytes big endian
	socket->write(data);
}

