#include "S3FS_Config.hpp"

S3FS_Config::S3FS_Config() {
	cluster_id = 0;
	expire_blocks = 86400;
	expire_inodes = 86400*7;
}

int S3FS_Config::clusterId() const {
	return cluster_id;
}

void S3FS_Config::setClusterId(int c) {
	cluster_id = c;
}

const QByteArray &S3FS_Config::mountOptions() const {
	return mount_options;
}

void S3FS_Config::setMountOptions(const QByteArray &o) {
	mount_options = o;
}

const QByteArray &S3FS_Config::bucket() const {
	return bucket_name;
}

void S3FS_Config::setBucket(const QByteArray &b) {
	bucket_name = b;
}

const QByteArray &S3FS_Config::mountPath() const {
	return mount_path;
}

void S3FS_Config::setMountPath(const QByteArray &p) {
	mount_path = p;
}

const QByteArray &S3FS_Config::queue() const {
	return queue_url;
}

void S3FS_Config::setQueue(const QByteArray &q) {
	queue_url = q;
}

const QString &S3FS_Config::cachePath() const {
	return cache_path;
}

void S3FS_Config::setCachePath(const QString &p) {
	cache_path = p;
}

quint64 S3FS_Config::expireBlocks() const {
	return expire_blocks;
}

void S3FS_Config::setExpireBlocks(quint64 e) {
	expire_blocks = e;
}

quint64 S3FS_Config::expireInodes() const {
	return expire_inodes;
}

void S3FS_Config::setExpireInodes(quint64 e) {
	expire_inodes = e;
}