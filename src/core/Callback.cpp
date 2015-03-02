#include "Callback.hpp"
#include <QMetaObject>
#include "S3FS.hpp"

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

Callback::Callback(S3FS *_parent, callback_method _cb, QtFuseRequest *_req) {
	parent = _parent;
	cb = _cb;
	req = _req;
}

void Callback::trigger() {
	(parent->*cb)(req);
	deleteLater();
}

