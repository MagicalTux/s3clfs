#include "Keyval.hpp"
#include <leveldb/comparator.h>

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


// DATA ACCESS LAYER (instance -> leveldb)

Keyval::Keyval(QObject *parent): QObject(parent) {
	db = NULL;
	// setup default options
	options.create_if_missing = true;
	options.compression = leveldb::kNoCompression;
	// TODO allow tweaking of values through command line
	options.max_open_files = 1000; // TODO check ulimit fileno value and adapt
	options.block_size = 65536; // 64kB, also largest size for values
	options.write_buffer_size = 2*1024*1024; // 2MB (smaller means less stalls during reorg)
	readoptions.verify_checksums = true;
	quick_cache.setMaxCost(1000);
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
	quick_cache.remove(key);
	leveldb::Status status = db->Put(writeoptions, LEVELDB_SLICE(key), LEVELDB_SLICE(value));
	return status.ok();
}

bool Keyval::remove(const QByteArray &key) {
	quick_cache.remove(key);
	leveldb::Status status = db->Delete(writeoptions, LEVELDB_SLICE(key));
	return status.ok();
}

QByteArray Keyval::value(const QByteArray &key) {
	if (quick_cache.contains(key)) return *quick_cache.object(key);
	std::string res;
	leveldb::Status status = db->Get(readoptions, LEVELDB_SLICE(key), &res);
	if (!status.ok()) return QByteArray();
	QByteArray qt_res(res.data(), res.size());
	if (qt_res.length() < 1024) {
		quick_cache.insert(key, new QByteArray(qt_res));
	}
	return qt_res;
}

bool Keyval::contains(const QByteArray &key) {
	if (quick_cache.contains(key)) return true;
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

