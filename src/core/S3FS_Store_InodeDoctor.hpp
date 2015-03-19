#include <QObject>
#include <QStringList>

class S3FS_Store;
class S3FS_Aws_S3;

class S3FS_Store_InodeDoctor: public QObject {
	Q_OBJECT
public:
	S3FS_Store_InodeDoctor(S3FS_Store *parent, quint64 ino);

public slots:
	void receivedRevisionsList(S3FS_Aws_S3*);
	void receivedInode(S3FS_Aws_S3*);

private:
	S3FS_Store *parent;
	quint64 ino;
	QStringList revisionsList;
	QByteArray list_prefix;
	QByteArray current_test_rev;

	void getLastRevision();

	void failed();
	void success();
};

