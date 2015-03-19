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

#include <QObject>

#pragma once

class QtFuseCallbackDummyCallback;

class QtFuseCallback: public QObject {
	Q_OBJECT
public:
	QtFuseCallback(QObject *parent = 0);

	template<class T> void setMethod(T *obj, void (T::*cb)(QtFuseCallback*)) { setMethod((QtFuseCallbackDummyCallback*)obj, (void(QtFuseCallbackDummyCallback::*)(QtFuseCallback*))cb); }
	void setMethod(QtFuseCallbackDummyCallback *obj, void (QtFuseCallbackDummyCallback::*cb)(QtFuseCallback*));
	int getError() const;

public slots:
	void trigger(); // calls method defined by setMethod()
	void triggerLater(); // calls method defined by setMethod()... later.
	virtual void error(int);

protected:
	virtual void customEvent(QEvent *e);

private:
	QtFuseCallbackDummyCallback *cb_obj;
	void (QtFuseCallbackDummyCallback::*cb_func)(QtFuseCallback*);
	int error_no;
};

