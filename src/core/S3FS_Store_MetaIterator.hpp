#include "KeyvalIterator.hpp"

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
