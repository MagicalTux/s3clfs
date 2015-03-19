#include <QObject>
#include <QLocalSocket>

class S3FS_Control;
class QJsonDocument;

class S3FS_Control_Client: public QObject {
	Q_OBJECT
public:
	S3FS_Control_Client(S3FS_Control *parent, QLocalSocket *socket);

public slots:
	void send(const QJsonDocument&);
	void send(const QByteArray&);

private:
	QLocalSocket *socket;
	S3FS_Control *parent;
};

