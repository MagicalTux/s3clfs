#include "S3FS_Config.hpp"

S3FS_Config::S3FS_Config() {
	cluster_id = 0;
	expire_blocks = 86400;
	list_fetch_interval = 3600*48;
	cache_data = true;
	database_max_size = 2;
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

const QString &S3FS_Config::dataPath() const {
	return data_path;
}

void S3FS_Config::setDataPath(const QString &p) {
	data_path = p;
}

quint64 S3FS_Config::expireBlocks() const {
	return expire_blocks;
}

void S3FS_Config::setExpireBlocks(quint64 e) {
	expire_blocks = e;
}

quint64 S3FS_Config::listFetchInterval() const {
	return list_fetch_interval;
}

void S3FS_Config::setListFetchInterval(quint64 i) {
	list_fetch_interval = i;
}

bool S3FS_Config::cacheData() const {
	return cache_data;
}

void S3FS_Config::setCacheData(bool b) {
	cache_data = b;
}

const QString &S3FS_Config::awsCredentialsUrl() const {
	return aws_credentials_url;
}

void S3FS_Config::setAwsCredentialsUrl(const QString &s) {
	aws_credentials_url = s;
}

const QString &S3FS_Config::controlSocket() const {
	return control_socket;
}

void S3FS_Config::setControlSocket(const QString &p) {
	control_socket = p;
}

int S3FS_Config::databaseMaxSize() const {
	return database_max_size;
}

void S3FS_Config::setDatabaseMaxSize(int s) {
	database_max_size = s;
}

