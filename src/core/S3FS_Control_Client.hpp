#include <QObject>
#include <QLocalSocket>

class S3FS_Control;
class QJsonDocument;
class QJsonObject;

class S3FS_Control_Client: public QObject {
	Q_OBJECT
public:
	S3FS_Control_Client(S3FS_Control *parent, QLocalSocket *socket);
	~S3FS_Control_Client();

	void cmd_ping(const QJsonObject&);
	void cmd_fsck(const QJsonObject&);

public slots:
	void send(const QVariant&);
	void send(const QJsonObject&);
	void send(const QJsonDocument&);
	void send(const QByteArray&);
	void close();
	void read();

protected:
	void handlePacket(const QByteArray &packet);

private:
	QLocalSocket *socket;
	S3FS_Control *parent;
	QByteArray read_buf;
};

