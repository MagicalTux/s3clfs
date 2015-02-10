#include <QObject>
#include <QGenericArgument>
#include <QPointer>
#include <QVariant>

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

class Callback: public QObject {
	Q_OBJECT
public:
	Callback(QObject *obj, const char *member,
		QGenericArgument val0 = QGenericArgument(0),
		QGenericArgument val1 = QGenericArgument(),
		QGenericArgument val2 = QGenericArgument(),
		QGenericArgument val3 = QGenericArgument(),
		QGenericArgument val4 = QGenericArgument(),
		QGenericArgument val5 = QGenericArgument(),
		QGenericArgument val6 = QGenericArgument(),
		QGenericArgument val7 = QGenericArgument(),
		QGenericArgument val8 = QGenericArgument(),
		QGenericArgument val9 = QGenericArgument());

	Callback(QObject *obj, const char *member, const QList<QGenericArgument>&);

public slots:
	void trigger();

private:
	QPointer<QObject> obj;
	QByteArray member; // using QByteArray to ensure copy is kept
	QVariant v0, v1, v2, v3, v4, v5, v6, v7, v8, v9;
	QByteArray n0, n1, n2, n3, n4, n5, n6, n7, n8, n9;
};

