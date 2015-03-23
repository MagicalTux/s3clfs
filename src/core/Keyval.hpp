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
#include <QObject>
#include <lmdb.h>
#include <QCache>

#pragma once

class KeyvalIterator;

class Keyval: public QObject {
	Q_OBJECT
public:
	Keyval(QObject *parent = 0);
	~Keyval();

	bool open(const QString &filename, quint64 max_size = 2LL * 1024 * 1024 * 1024);
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
	MDB_env *mdb_env;
	MDB_dbi mdb_dbi;
	friend class KeyvalIterator; // grants access to private and protected members of KeyvalIterator
}; 

