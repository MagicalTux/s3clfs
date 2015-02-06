#include "S3FS_Store_MetaIterator.hpp"

S3FS_Store_MetaIterator::S3FS_Store_MetaIterator(Keyval*kv, const QByteArray &_prefix): KeyvalIterator(kv) {
	prefix = _prefix;
	KeyvalIterator::find(prefix);
}

bool S3FS_Store_MetaIterator::isValid() {
	return (KeyvalIterator::key().left(prefix.length()) == prefix);
}

bool S3FS_Store_MetaIterator::next() {
	if (!KeyvalIterator::next()) return false;
	return isValid();
}

QByteArray S3FS_Store_MetaIterator::key() {
	if (!isValid()) return QByteArray();
	return KeyvalIterator::key().mid(prefix.length());
}

QByteArray S3FS_Store_MetaIterator::value() {
	if (!isValid()) return QByteArray();
	return KeyvalIterator::value();
}

bool S3FS_Store_MetaIterator::find(const QByteArray &k) {
	return KeyvalIterator::find(prefix+k);
}
