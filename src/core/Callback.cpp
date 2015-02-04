#include "Callback.hpp"
#include <QMetaObject>


Callback::Callback(QObject *_obj, const char *_member, QGenericArgument _val0, QGenericArgument _val1, QGenericArgument _val2, QGenericArgument _val3, QGenericArgument _val4, QGenericArgument _val5, QGenericArgument _val6, QGenericArgument _val7, QGenericArgument _val8, QGenericArgument _val9) {
	obj = _obj;
	member = _member;
	val0 = _val0;
	val1 = _val1;
	val2 = _val2;
	val3 = _val3;
	val4 = _val4;
	val5 = _val5;
	val6 = _val6;
	val7 = _val7;
	val8 = _val8;
	val9 = _val9;
}

void Callback::trigger() {
	if (obj.isNull()) return;

	QMetaObject::invokeMethod(obj.data(), member.data(), val0, val1, val2, val3, val4, val5, val6, val7, val8, val9);
}

