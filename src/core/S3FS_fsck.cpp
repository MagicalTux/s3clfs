#include "S3FS_fsck.hpp"
#include "S3FS_Control_Client.hpp"

S3FS_fsck::S3FS_fsck(S3FS *_main, S3FS_Control_Client *requestor, QVariant _id) {
	main = _main;
	id = _id;

	connect(this, SIGNAL(send(const QVariant&)), requestor, SLOT(send(const QVariant&)));

	qDebug("S3FS_fsck: Starting filesystem check");

	send(QVariantMap({{"command","fsck_reply"},{"id",id},{"status","initializing"}}));
}

