#include "KeyvalIterator.hpp"

#pragma once

class S3FS_Store_MetaIterator: private KeyvalIterator {
public:
	S3FS_Store_MetaIterator(Keyval*, const QByteArray &prefix); // prefix is the actual key
	QByteArray key();
	QByteArray fullKey();
	QByteArray value();

	bool next();
	bool isValid();
	bool find(const QByteArray &);

private:
	QByteArray prefix;
};
