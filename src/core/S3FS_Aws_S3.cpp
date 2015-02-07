#include "S3FS_Aws_S3.hpp"

S3FS_Aws_S3::S3FS_Aws_S3(const QByteArray &_bucket, QObject *parent): S3FS_Aws(parent) {
	bucket = _bucket;
}

