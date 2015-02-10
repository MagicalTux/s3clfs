#include <QObject>
#include <QCryptographicHash>
#include "Keyval.hpp"
#include <QVariant>
#include <QSet>
#include <QTimer>

/*  S3ClFS - AWS S3 backed cluster filesystem
 *  Copyright (C) 2015 Mark Karpeles
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

class S3FS_Obj; // inode
class S3FS_Aws;
class S3FS_Aws_S3;
class S3FS_Aws_SQS;
class S3FS_Config;
class Callback;
class S3FS_Store_MetaIterator;

class S3FS_Store: public QObject {
	Q_OBJECT

public:
	S3FS_Store(S3FS_Config *cfg, QObject *parent = 0);

	// filesystem config
	const QVariantMap &getConfig();
	bool readConfig();
	bool setConfig(const QVariantMap&);

	// inodes
	bool hasInode(quint64);
	bool storeInode(const S3FS_Obj&);
	S3FS_Obj getInode(quint64);
	bool hasInodeLocally(quint64);
	void callbackOnInodeCached(quint64, Callback*);
	void removeInodeFromCache(quint64);
	void brokenInode(quint64);

	// blocks
	QByteArray writeBlock(const QByteArray &buf);
	QByteArray readBlock(const QByteArray &buf);
	bool hasBlockLocally(const QByteArray&);
	void callbackOnBlockCached(const QByteArray&, Callback*);

	// inode meta
	bool hasInodeMeta(quint64 ino, const QByteArray &key);
	QByteArray getInodeMeta(quint64 ino, const QByteArray &key);
	bool setInodeMeta(quint64 ino, const QByteArray &key, const QByteArray &value);
	S3FS_Store_MetaIterator *getInodeMetaIterator(quint64 ino);
	bool removeInodeMeta(quint64 ino, const QByteArray &key);
	bool clearInodeMeta(quint64 ino);

signals:
	void ready();

public slots:
	void readyStateWithoutAws();
	void receivedFormatFile(S3FS_Aws_S3*);
	void receivedInodeList(S3FS_Aws_S3*);
	void receivedInode(S3FS_Aws_S3*);
	void receivedBlock(S3FS_Aws_S3*);
	void updateInodes();
	void getInodesList();
	void gotNewFile(const QString&,const QString&);

	void lastaccess_update();
	void lastaccess_clean();

private:
	void sendInodeToAws(quint64);
	void inodeUpdated(quint64);
	void learnFile(const QString&, bool);

	quint64 makeInodeRev();

	QSet<quint64> inodes_to_update;
	QTimer inodes_updater;
	QTimer cache_updater;
	QMap<quint64, QList<Callback*> > inode_download_callback;
	QMap<QByteArray, QList<Callback*> > block_download_callback;

	// lastaccess pruning system
	QTimer lastaccess_updater;
	QTimer lastaccess_cleaner;
	QSet<quint64> lastaccess_inode;
	QSet<QByteArray> lastaccess_data;
	quint64 expire_blocks; // expiration of cached blocks, in seconds
	quint64 expire_inodes; // expiration of cached inodes, in seconds

	bool aws_list_ready;
	bool aws_format_ready;

	int cluster_node_id;
	QString kv_location;
	Keyval kv; // local cache
	QByteArray bucket;
	QCryptographicHash::Algorithm algo;
	QVariantMap config;
	S3FS_Aws *aws;
	S3FS_Aws_SQS *aws_sqs;
	quint64 last_inode_rev;
	QRegExp file_match;
	S3FS_Config *cfg;
};

