#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>

class S3FS_Aws_S3;
class QNetworkReply;

struct S3FS_Aws_Queue_Entry {
	QNetworkRequest req;
	QByteArray verb;
	QIODevice *data;
	QObject *sender; // sender slot requestStarted(QNetworkReply*) will be called once query starts
};

class S3FS_Aws: public QObject {
	Q_OBJECT
public:
	S3FS_Aws(QObject *parent = 0);
	bool isValid();

public slots:
	void replyDestroyed(QObject *obj);

protected:
	QByteArray signV2(const QByteArray &string);
	void http(QObject *caller, const QByteArray &verb, const QNetworkRequest req, QIODevice *data = 0);

	friend class S3FS_Aws_S3;

private:
	QByteArray id;
	QByteArray key;
	QNetworkAccessManager net;

	QSet<QNetworkReply*> http_running;
	QList<S3FS_Aws_Queue_Entry*> http_queue;
};

