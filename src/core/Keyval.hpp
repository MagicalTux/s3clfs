#include <QObject>
#include <leveldb/db.h>

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

#pragma once

#define LEVELDB_SLICE(_x) leveldb::Slice(_x.data(), _x.length())

class KeyvalIterator;

class Keyval: public QObject {
	Q_OBJECT
public:
	Keyval(QObject *parent = 0);
	~Keyval();

	bool open(const QString &filename);
	void close();
	bool create(const QString &filename);
	bool insert(const QByteArray &key, const QByteArray &value);
	bool remove(const QByteArray &key);
	QByteArray value(const QByteArray &key);
	bool contains(const QByteArray &key);

	bool isValid() const;
	bool isEmpty();

	static bool destroy(const QString &filename);

private:
	leveldb::DB *db;
	leveldb::Options options;
	leveldb::ReadOptions readoptions;
	leveldb::WriteOptions writeoptions;
	friend class KeyvalIterator; // grants access to private and protected members of KeyvalIterator
}; 

