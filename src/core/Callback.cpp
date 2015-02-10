#include "Callback.hpp"
#include <QMetaObject>

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

Callback::Callback(QObject *_obj, const char *_member, QGenericArgument _val0, QGenericArgument _val1, QGenericArgument _val2, QGenericArgument _val3, QGenericArgument _val4, QGenericArgument _val5, QGenericArgument _val6, QGenericArgument _val7, QGenericArgument _val8, QGenericArgument _val9) {
	obj = _obj;
	member = _member;

	#define STORE_VAL(_x) v ## _x = QVariant(QVariant::nameToType(_val ## _x.name()), _val ## _x.data()); n ## _x = _val ## _x.name()

	STORE_VAL(0);
	STORE_VAL(1);
	STORE_VAL(2);
	STORE_VAL(3);
	STORE_VAL(4);
	STORE_VAL(5);
	STORE_VAL(6);
	STORE_VAL(7);
	STORE_VAL(8);
	STORE_VAL(9);
}

Callback::Callback(QObject *_obj, const char *_member, const QList<QGenericArgument>&args) {
	obj = _obj;
	member = _member;

	#define STORE_VAL2(_x) if (args.length() > _x) { v ## _x = QVariant(QVariant::nameToType(args.at(_x).name()), args.at(_x).data()); n ## _x = args.at(_x).name(); }

	STORE_VAL2(0);
	STORE_VAL2(1);
	STORE_VAL2(2);
	STORE_VAL2(3);
	STORE_VAL2(4);
	STORE_VAL2(5);
	STORE_VAL2(6);
	STORE_VAL2(7);
	STORE_VAL2(8);
	STORE_VAL2(9);
}

void Callback::trigger() {
	if (obj.isNull()) return;

	#define ARG(_x) QGenericArgument(n ## _x.constData(), &(v ## _x.data_ptr().data))

	QMetaObject::invokeMethod(obj.data(), member.data(), ARG(0), ARG(1), ARG(2), ARG(3), ARG(4), ARG(5), ARG(6), ARG(7), ARG(8), ARG(9));
	deleteLater();
}

