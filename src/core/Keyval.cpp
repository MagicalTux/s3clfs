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

#include <QDir>
#include "Keyval.hpp"

// DATA ACCESS LAYER (instance -> lmdb)

Keyval::Keyval(QObject *parent): QObject(parent) {
	mdb_env = NULL;
}

Keyval::~Keyval() {
	if (mdb_env) {
		mdb_env_close(mdb_env);
		mdb_env = NULL;
	}
}

void Keyval::close() {
	if (mdb_env) {
		mdb_env_close(mdb_env);
		mdb_env = NULL;
	}
}

bool Keyval::destroy(const QString &) {
	return false; // TODO
}

bool Keyval::open(const QString &filename, quint64 max_size) {
	int rc;
	if (mdb_env) close();

	rc = mdb_env_create(&mdb_env);
	if (rc != 0) {
		qCritical("Failed to initialize LMDB");
		return false;
	}
	mdb_env_set_mapsize(mdb_env, max_size);
	mdb_env_set_maxreaders(mdb_env, 1024);
	QDir(filename).mkpath(".");
	rc = mdb_env_open(mdb_env, filename.toLocal8Bit().data(), MDB_NOMETASYNC | MDB_NOSYNC | MDB_NORDAHEAD | MDB_NOTLS, 0664);
	if (rc != 0) {
		qCritical("Failed to open database %s: %s", qPrintable(filename), mdb_strerror(rc));
		return false;
	}

	// open in a transaction
	MDB_txn *mdb_txn;
	mdb_txn_begin(mdb_env, NULL, 0, &mdb_txn);
	rc = mdb_dbi_open(mdb_txn, NULL, MDB_CREATE, &mdb_dbi);
	if (rc != 0) {
		qCritical("Failed to open database %s: %s", qPrintable(filename), mdb_strerror(rc));
		mdb_txn_abort(mdb_txn);
		return false;
	}
	rc = mdb_txn_commit(mdb_txn);
	if (rc != 0) {
		qCritical("Failed to open database %s: %s", qPrintable(filename), mdb_strerror(rc));
		return false;
	}
	return true;
}

bool Keyval::create(const QString &filename) {
	return open(filename); // TODO
}

bool Keyval::insert(const QByteArray &key, const QByteArray &value) {
	MDB_val db_key, db_data;
	MDB_txn *txn;
	int rc;
	db_key.mv_size = key.length();
	db_key.mv_data = const_cast<char*>(key.data());
	db_data.mv_size = value.length();
	db_data.mv_data = const_cast<char*>(value.data());

	rc = mdb_txn_begin(mdb_env, NULL, 0, &txn);
	rc = mdb_put(txn, mdb_dbi, &db_key, &db_data, 0);
	rc = mdb_txn_commit(txn);
	if (rc != 0) {
		qCritical("Failed to insert in db: %s", mdb_strerror(rc));
		return false;
	}
	return true;
}

bool Keyval::remove(const QByteArray &key) {
	MDB_val db_key;
	MDB_txn *txn;
	int rc;

	db_key.mv_size = key.length();
	db_key.mv_data = const_cast<char*>(key.data());

	rc = mdb_txn_begin(mdb_env, NULL, 0, &txn);
	rc = mdb_del(txn, mdb_dbi, &db_key, NULL);
	rc = mdb_txn_commit(txn);
	if (rc != 0) {
		qCritical("Failed to remove from db: %s", mdb_strerror(rc));
		return false;
	}
	return true;
}

QByteArray Keyval::value(const QByteArray &key) {
	MDB_val db_key, db_data;
	MDB_txn *txn;
	int rc;
	db_key.mv_size = key.length();
	db_key.mv_data = const_cast<char*>(key.data());

	rc = mdb_txn_begin(mdb_env, NULL, MDB_RDONLY, &txn);
	rc = mdb_get(txn, mdb_dbi, &db_key, &db_data);
	if (rc != 0) {
		mdb_txn_abort(txn);
		return QByteArray();
	}
	QByteArray res((char*)db_data.mv_data, db_data.mv_size); // deep copy
	mdb_txn_commit(txn);
	return res;
}

bool Keyval::contains(const QByteArray &key) {
	MDB_val db_key, db_data;
	MDB_txn *txn;
	int rc;
	db_key.mv_size = key.length();
	db_key.mv_data = const_cast<char*>(key.data());

	rc = mdb_txn_begin(mdb_env, NULL, MDB_RDONLY, &txn);
	if (rc != 0) {
		qFatal("Failed to create transaction: %s", mdb_strerror(rc));
	}
	rc = mdb_get(txn, mdb_dbi, &db_key, &db_data);
	mdb_txn_commit(txn);
	if (rc != 0) { // ie. MDB_NOTFOUND
		return false;
	}
	return true;
}

bool Keyval::isValid() const {
	return (bool)mdb_dbi;
}

bool Keyval::isEmpty() {
	MDB_txn *txn;
	MDB_cursor *cursor;
	int rc;

	rc = mdb_txn_begin(mdb_env, NULL, MDB_RDONLY, &txn);
	rc = mdb_cursor_open(txn, mdb_dbi, &cursor);
	rc = mdb_cursor_get(cursor, NULL, NULL, MDB_FIRST);
	
	mdb_cursor_close(cursor);
	mdb_txn_commit(txn);

	return rc == MDB_NOTFOUND;
}

