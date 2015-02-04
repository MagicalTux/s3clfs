#!/bin/sh
set -e
if [ -f Makefile ]; then
	if [ ! -f s3clfs ]; then
		make distclean
	fi
fi
. contrib/qmake.sh

"$QMAKE" CONFIG+="cli debug"
make
exec ./s3clfs s3clfs /mnt/tmp/
