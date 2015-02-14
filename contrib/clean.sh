#!/bin/sh
if [ -f Makefile ]; then
	make distclean
	rm -f Makefile
fi
rm -fr build
rm -f s3clfs s3clfs-test

