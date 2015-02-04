#include <QObject>
#include <leveldb/db.h>

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

