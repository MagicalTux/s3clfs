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

private:
	int cluster_id;
	QByteArray mount_options;
	QByteArray bucket_name;
	QByteArray mount_path;
	QByteArray queue_url;
	QString cache_path;
};

