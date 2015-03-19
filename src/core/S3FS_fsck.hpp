#include <QObject>
#include <QVariant>
#include <QTimer>
#include <QSet>

class S3FS;
class S3FS_Store;
class S3FS_Control_Client;
class S3FS_Store_MetaIterator;
class QtFuseCallback;

class S3FS_fsck: public QObject {
	Q_OBJECT
public:
	S3FS_fsck(S3FS *main, S3FS_Control_Client *requestor, QVariant id);

public slots:
	void process();

signals:
	void send(const QVariant&);

private:
	S3FS *main;
	QVariant id;
	int status;
	QTimer idle_timer;
	S3FS_Store_MetaIterator *iterator;
	quint64 fsck_ino; // inode where to store various fsck related stuff

	void process_0(QtFuseCallback *n = 0);
	void process_1(QtFuseCallback *n = 0);
	void process_2(QtFuseCallback *n = 0);
	S3FS_Store &store;

	quint64 scan_inode;
	QList<quint64> scan_inode_queue;
	QSet<quint64> known_inodes;
};

