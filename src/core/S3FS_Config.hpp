#include <QByteArray>
#include <QString>

class S3FS_Config {
public:
	S3FS_Config();
	int clusterId() const; // 1~100 id within cluster (to avoid overlaps in inode numbers)
	void setClusterId(int);
	
	const QByteArray &mountOptions() const;
	void setMountOptions(const QByteArray &);

	const QByteArray &bucket() const;
	void setBucket(const QByteArray &);

	const QByteArray &mountPath() const;
	void setMountPath(const QByteArray &);

	const QByteArray &queue() const;
	void setQueue(const QByteArray &);

	const QString &cachePath() const;
	void setCachePath(const QString &);

	quint64 expireBlocks() const;
	void setExpireBlocks(quint64);

	quint64 expireInodes() const;
	void setExpireInodes(quint64);

	quint64 listFetchInterval() const;
	void setListFetchInterval(quint64);

private:
	int cluster_id; // node id within cluster
	QByteArray mount_options; // mount options (allow_other, etc)
	QByteArray bucket_name; // name of AWS S3 bucket
	QByteArray mount_path; // path where to mount fuse filesystem
	QByteArray queue_url; // URL of SQS queue (connected to SNS, connected to bucket notifications)
	QString cache_path; // Path where to store leveldb
	quint64 expire_blocks; // expiration of cached blocks, in seconds
	quint64 expire_inodes; // expiration of cached inodes, in seconds
	quint64 list_fetch_interval; // how often to re-fetch the inodes list on S3
};

