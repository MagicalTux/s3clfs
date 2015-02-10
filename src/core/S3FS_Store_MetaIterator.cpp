#include "S3FS_Store_MetaIterator.hpp"

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

QByteArray S3FS_Store_MetaIterator::fullKey() {
	if (!isValid()) return QByteArray();
	return KeyvalIterator::key();
}

QByteArray S3FS_Store_MetaIterator::value() {
	if (!isValid()) return QByteArray();
	return KeyvalIterator::value();
}

bool S3FS_Store_MetaIterator::find(const QByteArray &k) {
	return KeyvalIterator::find(prefix+k);
}
