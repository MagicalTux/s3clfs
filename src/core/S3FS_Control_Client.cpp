#include <QJsonDocument>
#include <QJsonObject>
#include <QCoreApplication>
#include "S3FS_Control.hpp"
#include "S3FS_Control_Client.hpp"
#include "S3FS_fsck.hpp"

static QMap<QString, void(S3FS_Control_Client::*)(const QJsonObject&)> control_cmds({
	{"ping",&S3FS_Control_Client::cmd_ping},
	{"fsck",&S3FS_Control_Client::cmd_fsck}
});

S3FS_Control_Client::S3FS_Control_Client(S3FS_Control *_parent, QLocalSocket *_socket) {
	parent = _parent;
	socket = _socket;
	connect(socket, SIGNAL(disconnected()), this, SLOT(close()));
	connect(socket, SIGNAL(readyRead()), this, SLOT(read()));

	send(QVariantMap({
		{"api_version",1},
		{"version",QCoreApplication::applicationVersion()},
		{"qt_version",qVersion()},
		{"qt_compile_version",QT_VERSION_STR},
		{"pid",QCoreApplication::applicationPid()},
		{"capabilities",QVariantMap({
			{"json",true}
		})}
	}));
}

S3FS_Control_Client::~S3FS_Control_Client() {
	delete socket;
}

void S3FS_Control_Client::cmd_fsck(const QJsonObject &pkt) {
	QVariant id = pkt.value("id").toVariant();

	QJsonObject res;
	res.insert("command", "fsck_reply");
	res.insert("id", QJsonValue::fromVariant(id));
	res.insert("status", "ack");
	send(res);

	// TODO: handle fsck options (recover or delete orphan inodes?)
	new S3FS_fsck(parent->getParent(), this, id);
}

void S3FS_Control_Client::cmd_ping(const QJsonObject &pkt) {
	QJsonObject res(pkt);
	res.insert("command", "pong");
	send(res);
}

void S3FS_Control_Client::handlePacket(const QByteArray &packet) {
	QJsonDocument doc(QJsonDocument::fromJson(packet));
	QJsonObject obj(doc.object());
	if (obj.isEmpty()) {
		qWarning("Invalid JSON data from control client, dropping connection");
		close();
		return;
	}
	auto cmd = obj.value("command").toString();
	if (!control_cmds.contains(cmd)) {
		QJsonObject res;
		res.insert("command","error");
		res.insert("error","invalid_command");
		res.insert("unknown_command",cmd);
		if (obj.contains("id")) res.insert("id", obj.value("id"));
		send(res);
		return;
	}
	(this->*control_cmds.value(cmd))(obj);
}

void S3FS_Control_Client::send(const QVariant &var) {
	send(QJsonDocument::fromVariant(var));
}

void S3FS_Control_Client::send(const QJsonObject &obj) {
	send(QJsonDocument(obj));
}

void S3FS_Control_Client::send(const QJsonDocument &doc) {
	send(doc.toJson());
}

void S3FS_Control_Client::send(const QByteArray &data) {
	if (data.length() == 0) {
		qWarning("Attempted to send empty packet, shouldn't happen");
		return;
	}
	QDataStream(socket) << (quint32)data.length(); // 32 bytes big endian
	socket->write(data);
}

void S3FS_Control_Client::close() {
	socket->close();
	deleteLater();
}

void S3FS_Control_Client::read() {
	if (socket->state() != QLocalSocket::ConnectedState) {
		close();
		return;
	}
	read_buf += socket->readAll();

	while(true) {
		if (read_buf.length() < 4) return;
		quint32 len;
		QDataStream(read_buf) >> len;
		if (len > 128*1024) { // drop client if sending too large of a packet (128k)
			qWarning("Received large packet from control connection, client dropped");
			read_buf.clear();
			close();
			return;
		}
		if ((quint32)read_buf.length() < (4+len)) return; // not enough data
		QByteArray packet = read_buf.mid(4,len);
		read_buf.remove(0,4+len);
		handlePacket(packet);
	}
}

