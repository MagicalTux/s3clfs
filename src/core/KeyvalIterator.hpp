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

	bool hasNext();
	bool next();

	bool hasPrevious();
	bool previous();

	void toBack();
	void toFront();

	bool find(const QByteArray &);

	void operator=(Keyval*);

private:
	leveldb::Iterator *i;
	Keyval *kv;
	int location;
};

