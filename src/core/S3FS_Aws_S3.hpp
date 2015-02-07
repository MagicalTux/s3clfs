#include "S3FS_Aws.hpp"

class S3FS_Aws_S3: public S3FS_Aws {
	Q_OBJECT
public:
	S3FS_Aws_S3(const QByteArray &bucket, QObject *parent = 0);

private:
	QByteArray bucket;
};

