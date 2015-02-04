#include "Keyval.hpp"
#include <leveldb/comparator.h>


// DATA ACCESS LAYER (instance -> leveldb)

Keyval::Keyval(QObject *parent): QObject(parent) {
	db = NULL;
	// setup default options
	options.create_if_missing = true;
	options.compression = leveldb::kNoCompression;
	options.max_open_files = 64;
	readoptions.verify_checksums = true;
}

Keyval::~Keyval() {
	if (db) {
		delete db;
		db = NULL;
	}
}

void Keyval::close() {
	if (db) {
		delete db;
		db = NULL;
	}
}

bool Keyval::destroy(const QString &filename) {
	leveldb::Options options;
	leveldb::Status status = leveldb::DestroyDB(filename.toStdString(), options); // XXX leveldb do not handle wstring
	return status.ok();
}

bool Keyval::open(const QString &filename) {
	leveldb::Status status = leveldb::DB::Open(options, filename.toStdString(), &db); // XXX leveldb do not handle wstring
	if (!status.ok()) {
		qCritical("Failed to open database %s: %s", qPrintable(filename), status.ToString().c_str());
		return false;
	}
	return true;
}

bool Keyval::create(const QString &filename) {
	options.error_if_exists = true;
	leveldb::Status status = leveldb::DB::Open(options, filename.toStdString(), &db); // XXX leveldb do not handle wstring
	if (!status.ok()) {
		qCritical("Failed to create database %s: %s", qPrintable(filename), status.ToString().c_str());
		return false;
	}
	return true;
}

bool Keyval::insert(const QByteArray &key, const QByteArray &value) {
	leveldb::Status status = db->Put(writeoptions, LEVELDB_SLICE(key), LEVELDB_SLICE(value));
	return status.ok();
}

bool Keyval::remove(const QByteArray &key) {
	leveldb::Status status = db->Delete(writeoptions, LEVELDB_SLICE(key));
	return status.ok();
}

QByteArray Keyval::value(const QByteArray &key) {
	std::string res;
	leveldb::Status status = db->Get(readoptions, LEVELDB_SLICE(key), &res);
	if (!status.ok()) return QByteArray();
	return QByteArray(res.data(), res.size());
}

bool Keyval::contains(const QByteArray &key) {
	std::string res;
	leveldb::Status status = db->Get(readoptions, LEVELDB_SLICE(key), &res);
	return status.ok(); // could be another error, but not so likely
}

bool Keyval::isValid() const {
	return (bool)db;
}

bool Keyval::isEmpty() {
	auto i = db->NewIterator(readoptions);
	i->SeekToFirst();
	bool res = !(i->Valid());
	delete i;
	return res;
}

