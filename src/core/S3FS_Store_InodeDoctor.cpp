#include "S3FS_Store_InodeDoctor.hpp"
#include "S3FS_Aws_S3.hpp"
#include "S3FS_Store.hpp"
#include "QtFuseCallback.hpp"

#define INT_TO_BYTES(_x) QByteArray _x ## _b; { QDataStream s_tmp(&_x ## _b, QIODevice::WriteOnly); s_tmp << _x; }

// this class is instanciated when the current (latest) version of a given inode is broken, or some other kind of issue happened
// First we will list all files in that inode's dir and attempt to load the latest file. 
// If that latest file fails checks (missing "" entry, entry inode number invalid, or anything else) we delete it and attempt the next file.

S3FS_Store_InodeDoctor::S3FS_Store_InodeDoctor(S3FS_Store *_parent, quint64 _ino): QObject(_parent) {
	parent = _parent;
	ino = _ino;

	qWarning("S3FS_Store_InodeDoctor: broken inode %llu found, attempting to fix it", ino);

	INT_TO_BYTES(ino);

	QByteArray ino_hex = ino_b.toHex();
	list_prefix = "metadata/"+ino_hex.right(1)+"/"+ino_hex.right(2)+"/"+ino_hex+"/";

	connect(S3FS_Aws_S3::listFiles(parent->bucket, list_prefix, parent->aws), SIGNAL(finished(S3FS_Aws_S3*)), this, SLOT(receivedRevisionsList(S3FS_Aws_S3*)));
}

void S3FS_Store_InodeDoctor::receivedRevisionsList(S3FS_Aws_S3 *r) {
	bool need_more;
	QStringList list = r->parseListFiles(need_more);
	revisionsList.append(list);
	if (need_more) {
		connect(r->listMoreFiles(list_prefix, list), SIGNAL(finished(S3FS_Aws_S3*)), this, SLOT(receivedRevisionsList(S3FS_Aws_S3*)));
		return;
	}

	// OK, we got all the revisions for this inode, now let's get the last one and start trying
	getLastRevision();
}

void S3FS_Store_InodeDoctor::getLastRevision() {
	if (revisionsList.isEmpty()) {
		failed();
		return;
	}

	current_test_rev = revisionsList.takeLast().toUtf8();
	qDebug("S3FS_Store_InodeDoctor: attempting recovery of inode %llu from revision %s", ino, current_test_rev.data());

	S3FS_Aws_S3 *req = S3FS_Aws_S3::getFile(parent->bucket, current_test_rev, parent->aws);
	connect(req, SIGNAL(finished(S3FS_Aws_S3*)), this, SLOT(receivedInode(S3FS_Aws_S3*)));
}

void S3FS_Store_InodeDoctor::receivedInode(S3FS_Aws_S3 *r) {
	QByteArray data = r->body();
	if (data.isEmpty()) {
		// invalid!
//		S3FS_Aws_S3::deleteFile(parent->bucket, current_test_rev, parent->aws);
		getLastRevision();
		return;
	}

	INT_TO_BYTES(ino);

	QDataStream s(data);
	while(!s.atEnd()) {
		QByteArray key, val;
		s >> key >> val;
		parent->kv.insert(QByteArrayLiteral("\x01")+ino_b+key, val);
	}
	if (!parent->kv.contains(QByteArrayLiteral("\x01")+ino_b)) {
		// missing "" entry
//		S3FS_Aws_S3::deleteFile(parent->bucket, current_test_rev, parent->aws);
		parent->kv.remove(QByteArrayLiteral("\x01")+ino_b); // avoid others to get this data
		getLastRevision();
		return;
	}

	parent->inodes_cache.remove(ino);
	auto ino_o = parent->getInode(ino);
	if ((!ino_o->isValid()) || (ino_o->getInode() != ino)) {
		// still not right
//		S3FS_Aws_S3::deleteFile(parent->bucket, current_test_rev, parent->aws);
		parent->kv.remove(QByteArrayLiteral("\x01")+ino_b);
		parent->inodes_cache.remove(ino);
		getLastRevision();
		return;
	}

	parent->storeInode(*ino_o); // make a new revision with the correct data

	// YAY! this worked!
	success();
}

void S3FS_Store_InodeDoctor::failed() {
	qWarning("S3FS_Store_InodeDoctor: broken inode %llu could not be fixed, removing data and notifying failure", ino);

//	parent->destroyInode(ino);

	QList<QtFuseCallback*> list = parent->inode_download_callback.take(ino);
	foreach(auto cb, list)
		cb->error(EIO);
	deleteLater();
}

void S3FS_Store_InodeDoctor::success() {
	qWarning("S3FS_Store_InodeDoctor: broken inode %llu fixed successfully by recovering older revision", ino);

	QList<QtFuseCallback*> list = parent->inode_download_callback.take(ino);
	foreach(auto cb, list)
		cb->trigger();
	deleteLater();
}
