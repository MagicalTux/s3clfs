#include "QtFuseCallback.hpp"
#include <QCoreApplication>

QtFuseCallback::QtFuseCallback(QObject *parent): QObject(parent) {
	cb_obj = NULL;
	error_no = 0;
}

void QtFuseCallback::setMethod(QtFuseCallbackDummyCallback *obj, void (QtFuseCallbackDummyCallback::*cb)(QtFuseCallback*)) {
	cb_obj = obj;
	cb_func = cb;
}

void QtFuseCallback::trigger() {
	if (cb_obj == NULL) return; // nope!
	(cb_obj->*cb_func)(this);
}

void QtFuseCallback::triggerLater() {
	QCoreApplication::postEvent(this, new QEvent((QEvent::Type)(QEvent::User+1)));
}

void QtFuseCallback::customEvent(QEvent *e) {
	switch((int)e->type()) {
		case QEvent::User+1: trigger(); break;
	}
}

void QtFuseCallback::error(int _errno) {
	error_no = _errno;
	triggerLater();
}

int QtFuseCallback::getError() const {
	return error_no;
}

