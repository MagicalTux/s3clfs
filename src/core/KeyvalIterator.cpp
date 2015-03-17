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

#include "KeyvalIterator.hpp"
#include "Keyval.hpp"

// lmdb iterator for Keyval
KeyvalIterator::KeyvalIterator(Keyval *_kv) {
	kv = _kv;
	mdb_txn_begin(kv->mdb_env, NULL, MDB_RDONLY, &txn);
	Q_CHECK_PTR(txn);
	mdb_cursor_open(txn, kv->mdb_dbi, &cursor);
	Q_CHECK_PTR(cursor);

	mdb_cursor_get(cursor, NULL, NULL, MDB_FIRST);
	location = 1; // iterator starts before first value, so we can call hasNext() immediately
}

KeyvalIterator::~KeyvalIterator() {
	mdb_cursor_close(cursor);
	mdb_txn_commit(txn);
}

void KeyvalIterator::toBack() {
	mdb_cursor_get(cursor, NULL, NULL, MDB_LAST);
	location = -1;
}

void KeyvalIterator::toFront() {
	mdb_cursor_get(cursor, NULL, NULL, MDB_FIRST);
	location = 1;
}

bool KeyvalIterator::find(const QByteArray &s) {
	MDB_val db_val;
	db_val.mv_size = s.length();
	db_val.mv_data = const_cast<char*>(s.data());

	location = 0;
	int rc = mdb_cursor_get(cursor, &db_val, NULL, MDB_SET_RANGE);
	return (rc == 0);
}

bool KeyvalIterator::hasNext() {
	int rc;
	switch(location) {
		case -1:
			rc = mdb_cursor_get(cursor, NULL, NULL, MDB_NEXT);
			if (rc != 0) return false;
			location = 0;
		case 0:
			rc = mdb_cursor_get(cursor, NULL, NULL, MDB_NEXT);
			if (rc != 0) return false;
			location = 1;
		case 1:
			return true;
	}
	return false;
}

QByteArray KeyvalIterator::nextKey() {
	if (!hasNext()) return QByteArray();
	if (location != 1) return QByteArray();

	MDB_val db_val;

	int rc = mdb_cursor_get(cursor, &db_val, NULL, MDB_GET_CURRENT);
	if (rc != 0) return QByteArray();

	return QByteArray((char*)db_val.mv_data, db_val.mv_size);
}

bool KeyvalIterator::next() {
	if (!hasNext()) return false;
	if (location != 1) return false;
	location = 0;
	return true;
}

bool KeyvalIterator::hasPrevious() {
	int rc;
	switch(location) {
		case 1:
			rc = mdb_cursor_get(cursor, NULL, NULL, MDB_PREV);
			if (rc != 0) return false;
			location = 0;
		case 0:
			rc = mdb_cursor_get(cursor, NULL, NULL, MDB_PREV);
			if (rc != 0) return false;
			location = -1;
		case -1:
			return true;
	}
	return false;
}

bool KeyvalIterator::previous() {
	if (!hasPrevious()) return false;
	if (location != -1) return false;
	location = 0;
	return true;
}

bool KeyvalIterator::isValid() {
	int rc;
	switch(location) {
		case 1:
			rc = mdb_cursor_get(cursor, NULL, NULL, MDB_PREV);
			if (rc != 0) return false;
			location = 0;
			return true;
		case -1:
			rc = mdb_cursor_get(cursor, NULL, NULL, MDB_NEXT);
			if (rc != 0) return false;
			location = 0;
			return true;
	}
	return true;
}

QByteArray KeyvalIterator::key() {
	if (!isValid()) return QByteArray();
	if (location != 0) return QByteArray();

	MDB_val db_val;

	mdb_cursor_get(cursor, &db_val, NULL, MDB_GET_CURRENT);

	return QByteArray((char*)db_val.mv_data, db_val.mv_size);
}

QByteArray KeyvalIterator::value() {
	if (!isValid()) return QByteArray();
	if (location != 0) return QByteArray();

	MDB_val db_val;
	
	mdb_cursor_get(cursor, NULL, &db_val, MDB_GET_CURRENT);

	return QByteArray((char*)db_val.mv_data, db_val.mv_size);
}

void KeyvalIterator::operator=(Keyval*_kv) {
	kv = _kv;

	mdb_cursor_close(cursor);
	mdb_txn_commit(txn);

	mdb_txn_begin(kv->mdb_env, NULL, MDB_RDONLY, &txn);
	Q_CHECK_PTR(txn);
	mdb_cursor_open(txn, kv->mdb_dbi, &cursor);
	Q_CHECK_PTR(cursor);

	mdb_cursor_get(cursor, NULL, NULL, MDB_FIRST);
	location = 1; // iterator starts before first value, so we can call hasNext() immediately
}

