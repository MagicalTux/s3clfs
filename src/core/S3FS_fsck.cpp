#include "S3FS.hpp"
#include "S3FS_Store.hpp"
#include "S3FS_fsck.hpp"
#include "S3FS_Control_Client.hpp"
#include <QTimer>

S3FS_fsck::S3FS_fsck(S3FS *_main, S3FS_Control_Client *requestor, QVariant _id): store(main->store) {
	main = _main;
	id = _id;
	status = 0;

	connect(this, SIGNAL(send(const QVariant&)), requestor, SLOT(send(const QVariant&)));

	qDebug("S3FS_fsck: Starting filesystem check");

	send(QVariantMap({{"command","fsck_reply"},{"id",id},{"status","initializing"}}));

	QTimer::singleShot(0, this, SLOT(process()));
}

void S3FS_fsck::process() {
	switch(status) {
		case 0: process_0(); return;
		case 1:
			qDebug("S3FS_fsck: Finished");
			send(QVariantMap({{"command","fsck_reply"},{"id",id},{"status","complete"}}));
			deleteLater();
			return;
	}
}

void S3FS_fsck::process_0() {
	if (!main->isReady()) {
		connect(main, SIGNAL(ready()), this, SLOT(process()));
		return;
	}
	send(QVariantMap({{"command","fsck_reply"},{"id",id},{"status","started"}}));
	status = 1;
	QTimer::singleShot(0, this, SLOT(process()));
}

