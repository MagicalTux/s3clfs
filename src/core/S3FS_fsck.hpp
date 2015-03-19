#include <QObject>
#include <QVariant>

class S3FS;
class S3FS_Control_Client;

class S3FS_fsck: public QObject {
	Q_OBJECT
public:
	S3FS_fsck(S3FS *main, S3FS_Control_Client *requestor, QVariant id);

signals:
	void send(const QVariant&);

private:
	S3FS *main;
	QVariant id;
};

