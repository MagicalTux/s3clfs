#include "S3FS.hpp"
#include "S3FS_Store.hpp"
#include "S3FS_fsck.hpp"
#include "S3FS_Control_Client.hpp"
#include "S3FS_Store_MetaIterator.hpp"
#include <QDateTime>

S3FS_fsck::S3FS_fsck(S3FS *_main, S3FS_Control_Client *requestor, QVariant _id): store(_main->store) {
	main = _main;
	id = _id;
	status = 0;
	scan_inode = 1;
	known_inodes.insert(1);
	iterator = NULL;
	fsck_ino_max = main->makeInode();

	connect(this, SIGNAL(send(const QVariant&)), requestor, SLOT(send(const QVariant&)));

	qDebug("S3FS_fsck: Starting filesystem check");

	send(QVariantMap({{"command","fsck_reply"},{"id",id},{"status","initializing"}}));

	connect(&idle_timer, SIGNAL(timeout()), this, SLOT(process()));
	idle_timer.setSingleShot(false);
	idle_timer.start(0);
}

void S3FS_fsck::process() {
	switch(status) {
		case 0: process_0(); return;
		case 1: process_1(); return;
		case 2: process_2(); return;
		case 3: process_1(); return; // re-scan
		case 4: process_2(); return; // cleanup stray inodes this time
		case 5:
			qDebug("S3FS_fsck: Finished");
			send(QVariantMap({{"command","fsck_reply"},{"id",id},{"status","complete"}}));
			idle_timer.stop();
			deleteLater();
			return;
	}
}

void S3FS_fsck::process_0(QtFuseCallback *_cb) {
	if (_cb) { // called from a callback
		_cb->deleteLater();
		if (_cb->getError()) {
			qDebug("S3FS_fsck: got error while trying to get an inode! Giving up!!");
			send(QVariantMap({{"command","fsck_reply"},{"id",id},{"status","panic"}}));
			idle_timer.stop();
			deleteLater();
			return;
		}
	}

	if (!main->isReady()) {
		auto cb = new QtFuseCallback(this);
		cb->setMethod<S3FS_fsck>(this, &S3FS_fsck::process_0);
		connect(main, SIGNAL(ready()), cb, SLOT(trigger()));
		idle_timer.stop();
		return;
	}

	if (!store.hasInodeLocally(1)) {
		auto cb = new QtFuseCallback(this);
		cb->setMethod<S3FS_fsck>(this, &S3FS_fsck::process_0);
		store.callbackOnInodeCached(1, cb);
		idle_timer.stop();
		return;
	}

	quint64 lost_found_ino;

	if (!store.hasInodeMeta(1, "lost+found")) {
		// we need to create that directory
		S3FS_Obj new_dir;
		new_dir.makeDir(main->makeInode(), 0700, 0, 0); // only root can read this
		store.storeInode(new_dir);

		// add . and .. in that dir
		QByteArray dir_entry;
		QDataStream(&dir_entry, QIODevice::WriteOnly) << new_dir.getInode() << new_dir.getFiletype();
		store.setInodeMeta(new_dir.getInode(), ".", dir_entry);

		store.setInodeMeta(1, "lost+found", dir_entry);

		// add ..
		dir_entry.clear();
		QDataStream(&dir_entry, QIODevice::WriteOnly) << (quint64)1 << (quint32)S_IFDIR;
		store.setInodeMeta(new_dir.getInode(), "..", dir_entry);
		lost_found_ino = new_dir.getInode();
	} else {
		QByteArray res_ino_bin = store.getInodeMeta(1, "lost+found");
		QDataStream(res_ino_bin) >> lost_found_ino;
	}

	// make sure we have a lost & found inode
	if (!store.hasInodeLocally(lost_found_ino)) {
		auto cb = new QtFuseCallback(this);
		cb->setMethod<S3FS_fsck>(this, &S3FS_fsck::process_0);
		store.callbackOnInodeCached(lost_found_ino, cb);
		idle_timer.stop();
		return;
	}

	// create a directory for this time's fsck
	QByteArray dir_name = QByteArrayLiteral("fsck_") + QDateTime::currentDateTimeUtc().toString(Qt::ISODate).toLatin1();
	if (store.hasInodeMeta(lost_found_ino, dir_name)) {
		int i = 2;
		do {
			QByteArray dir_name_2 = dir_name+"_"+QByteArray::number(i);
			if (store.hasInodeMeta(lost_found_ino, dir_name_2)) {
				i++;
				continue;
			}
			dir_name = dir_name_2;
			break;
		} while(true);
	}

	// store dir
	S3FS_Obj new_dir;
	new_dir.makeDir(main->makeInode(), 0700, 0, 0); // only root can read this
	store.storeInode(new_dir);

	// add . and .. in that dir
	QByteArray dir_entry;
	QDataStream(&dir_entry, QIODevice::WriteOnly) << new_dir.getInode() << new_dir.getFiletype();
	store.setInodeMeta(new_dir.getInode(), ".", dir_entry);
	store.setInodeMeta(lost_found_ino, dir_name, dir_entry);

	// add ..
	dir_entry.clear();
	QDataStream(&dir_entry, QIODevice::WriteOnly) << lost_found_ino << (quint32)S_IFDIR;
	store.setInodeMeta(new_dir.getInode(), "..", dir_entry);

	fsck_ino = new_dir.getInode();

	send(QVariantMap({{"command","fsck_reply"},{"id",id},{"status","started"},{"directory",dir_name}}));
	status++;
	idle_timer.start(0);
}

void S3FS_fsck::process_1(QtFuseCallback *_cb) {
	if (_cb) {//called from a callback
		_cb->deleteLater();
		if (_cb->getError()) {
			qDebug("S3FS_fsck: got error while trying to get an inode! Giving up!!");
			send(QVariantMap({{"command","fsck_reply"},{"id",id},{"status","panic"}}));
			idle_timer.stop();
			deleteLater();
			return;
		}
	}

	// go through the tree, check for all inodes
	if (!store.hasInodeLocally(scan_inode)) {
		auto cb = new QtFuseCallback(this);
		cb->setMethod<S3FS_fsck>(this, &S3FS_fsck::process_1);
		qDebug("S3FS_fsck: getting inode %llu", scan_inode);
		store.callbackOnInodeCached(scan_inode, cb);
		idle_timer.stop();
		return;
	}
	qDebug("S3FS_fsck: scanning inode %llu", scan_inode);

	auto ino = store.getInode(scan_inode);

	if (ino->isDir()) {
		// recurse!
		auto it = store.getInodeMetaIterator(scan_inode);
		do {
			if (!it->isValid()) return;
			if (it->key() == "") {
				continue;
			}
			quint64 ino_n; quint32 mode_n;
			QDataStream(it->value()) >> ino_n >> mode_n;
			if (!store.hasInode(ino_n)) {
				send(QVariantMap({{"command","fsck_reply"},{"id",id},{"status","invalid_inode"},{"parent_ino",scan_inode},{"ino",ino_n},{"type",mode_n}}));
				store.removeInodeMeta(scan_inode, it->key());
				continue;
			}
			if (it->key() == ".") {
				// TODO check if . has directory mode
				if (ino_n != scan_inode) {
					send(QVariantMap({{"command","fsck_reply"},{"id",id},{"status","invalid_curdir"},{"parent_ino",scan_inode},{"ino",ino_n},{"type",mode_n}}));
					// TODO: reset . to point to cur dir
				}
				continue;
			}
			if (it->key() == "..") {
				// TODO check if indeed parent, and indeed a directory
				continue;
			}
			known_inodes.insert(ino_n);
			switch(mode_n & S_IFMT) {
				case S_IFDIR:
					scan_inode_queue.append(ino_n);
					break;
				case S_IFREG:
				case S_IFLNK:
					break;
				default:
					// unsupported file type, shouldn't exist
					send(QVariantMap({{"command","fsck_reply"},{"id",id},{"status","invalid_inode_type"},{"parent_ino",scan_inode},{"ino",ino_n},{"type",mode_n}}));
					store.removeInodeMeta(scan_inode, it->key());
			}
		} while(it->next());
		delete it;
	}

	if (scan_inode_queue.length()) {
		scan_inode = scan_inode_queue.takeFirst();
	} else {
		send(QVariantMap({{"command","fsck_reply"},{"id",id},{"status","running"},{"found_inodes",known_inodes.size()}}));
		status++;
	}
	idle_timer.start(0);
}

void S3FS_fsck::process_2(QtFuseCallback *_cb) {
	if (_cb) {//called from a callback
		_cb->deleteLater();
		if (_cb->getError()) {
			if (iterator) {
				qDebug("S3FS_fsck: got error while trying to get an inode, skipping");
				if (!iterator->next()) {
					status++;
					idle_timer.start(0);
					return;
				}
			} else {
				qDebug("S3FS_fsck: got error in step 3 before even starting! Giving up!!");
				send(QVariantMap({{"command","fsck_reply"},{"id",id},{"status","panic"}}));
				idle_timer.stop();
				deleteLater();
				if (iterator) delete iterator;
				return;
			}
		}
	}

	// list all inodes found in the system. If a dir & empty, remove completely
	if (!iterator) iterator = store.getInodeListIterator();

	do {
		if (!iterator->isValid()) break;
		quint64 ino_n;
		QDataStream(iterator->key()) >> ino_n;
		if (ino_n > fsck_ino_max) continue; // out of bound inode (we should probably be able to break here)
		if (known_inodes.contains(ino_n)) continue; // inode is in the known inodes tree
		if (!store.hasInodeLocally(ino_n)) {
			auto cb = new QtFuseCallback(this);
			cb->setMethod<S3FS_fsck>(this, &S3FS_fsck::process_2);
			store.callbackOnInodeCached(ino_n, cb);
			idle_timer.stop();
			return;
		}
		auto ino = store.getInode(ino_n);
		qDebug("Found orphan inode %llu", ino_n);

		// check if not empty
		auto it = store.getInodeMetaIterator(ino_n);
		int meta_count = 0;
		do {
			if (it->key() != "") meta_count++;
		} while(it->next());
		delete it;

		if (!meta_count) {
			// empty!
			qDebug("Found orphan and empty inode %llu, dropped", ino_n);
			store.destroyInode(ino_n);
			continue;
		}

		if (ino->getFiletype() == S_IFDIR) {
			// directory, check content's ..
			if(!store.hasInodeMeta(ino_n, "..")) {
				qDebug("Found orphan directory inode %llu but it is missing an entry for ..", ino_n);
				// TODO not sure... how should we take care of that? drop the dir?
				continue;
			}
			// check if that dir's parent is known (if it is not, do not restore)
			QByteArray parent_info = store.getInodeMeta(ino_n, "..");
			quint64 parent_ino_n;
			QDataStream(parent_info) >> parent_ino_n;
			if (!known_inodes.contains(parent_ino_n)) {
				if (store.hasInode(parent_ino_n)) {
					qDebug("Skipping orphan inode %llu since it might be a child of another inode we have yet to recover", ino_n);
					continue;
				}
			}
		}

		if (ino->getFiletype() == S_IFREG) {
			if (status < 3) continue; // too soon to get rid of files
			if (fsck_found_files == 0) continue; // ignore files (default)
			if (fsck_found_files == -1) {
				// get rid of file
				store.destroyInode(ino_n);
				continue;
			}
			// if value is 1, continue to connect inode
		}

		// connect inode to fsck_ino
		if (!store.hasInodeLocally(fsck_ino)) {
			auto cb = new QtFuseCallback(this);
			cb->setMethod<S3FS_fsck>(this, &S3FS_fsck::process_2);
			store.callbackOnInodeCached(fsck_ino, cb);
			idle_timer.stop();
			return;
		}

		QByteArray inode_entry;
		QDataStream(&inode_entry, QIODevice::WriteOnly) << ino->getInode() << ino->getFiletype();
		store.setInodeMeta(fsck_ino, QByteArrayLiteral("orphan_")+QByteArray::number(ino->getInode(), 16), inode_entry);
	} while(iterator->next());

	status++;
	idle_timer.start(0);
}

