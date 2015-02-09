#include <QByteArray>

#pragma once

namespace leveldb {
	class Iterator;
}
class Keyval;

class KeyvalIterator {
public:
	KeyvalIterator(Keyval*);
	~KeyvalIterator();
	QByteArray key();
	QByteArray value();
	QByteArray nextKey();

	bool hasNext();
	bool next();

	bool hasPrevious();
	bool previous();

	void toBack();
	void toFront();

	bool find(const QByteArray &);
	bool isValid();

	void operator=(Keyval*);

private:
	leveldb::Iterator *i;
	Keyval *kv;
	int location;
};

