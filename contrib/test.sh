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
if [ ! -d fuse ]; then
	mkdir fuse
fi

exec ./s3clfs -q https://sqs.ap-northeast-1.amazonaws.com/133293341851/bucket-s3clfs-1 s3clfs fuse
