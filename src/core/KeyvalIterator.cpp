#include "KeyvalIterator.hpp"
#include "Keyval.hpp"
#include <leveldb/iterator.h>

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

// leveldb iterator for Keyval
KeyvalIterator::KeyvalIterator(Keyval *_kv) {
	kv = _kv;
	i = kv->db->NewIterator(kv->readoptions);
	Q_CHECK_PTR(i);

	i->SeekToFirst();
	location = 1; // iterator starts before first value, so we can call hasNext() immediately
}

KeyvalIterator::~KeyvalIterator() {
	delete i;
}

void KeyvalIterator::toBack() {
	i->SeekToLast();
	location = -1;
}

void KeyvalIterator::toFront() {
	i->SeekToFirst();
	location = 1;
}

bool KeyvalIterator::find(const QByteArray &s) {
	location = 0;
	i->Seek(LEVELDB_SLICE(s));
	return i->Valid();
}

bool KeyvalIterator::hasNext() {
	switch(location) {
		case -1:
			i->Next();
		case 0:
			i->Next();
	}
	location = 1;
	return i->Valid();
}

QByteArray KeyvalIterator::nextKey() {
	switch(location) {
		case -1:
			i->Next();
		case 0:
			i->Next();
	}
	location = 1;
	if (!i->Valid()) return QByteArray();
	const auto &k = i->key();
	return QByteArray(k.data(), k.size());
}

bool KeyvalIterator::next() {
	switch(location) {
		case -1:
			i->Next();
		case 0:
			i->Next();
	}
	location = 0;
	return i->Valid();
}

bool KeyvalIterator::hasPrevious() {
	switch(location) {
		case 1:
			i->Prev();
		case 0:
			i->Prev();
	}
	location = -1;
	return i->Valid();
}

bool KeyvalIterator::previous() {
	switch(location) {
		case 1:
			i->Prev();
		case 0:
			i->Prev();
	}
	location = 0;
	return i->Valid();
}

bool KeyvalIterator::isValid() {
	switch(location) {
		case 1: i->Prev(); break;
		case -1: i->Next(); break;
	}
	location = 0;

	return i->Valid();
}

QByteArray KeyvalIterator::key() {
	switch(location) {
		case 1: i->Prev(); break;
		case -1: i->Next(); break;
	}
	location = 0;

	if (!i->Valid()) return QByteArray();

	const auto &k = i->key();
	return QByteArray(k.data(), k.size());
}

QByteArray KeyvalIterator::value() {
	switch(location) {
		case 1: i->Prev(); break;
		case -1: i->Next(); break;
	}
	location = 0;

	const auto &k = i->value();
	return QByteArray(k.data(), k.size());
}

void KeyvalIterator::operator=(Keyval*_kv) {
	kv = _kv;

	delete i;
	i = kv->db->NewIterator(kv->readoptions);
	Q_CHECK_PTR(i);

	i->SeekToFirst();
	location = 1; // iterator starts before first value, so we can call hasNext() immediately
}

