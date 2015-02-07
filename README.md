# s3clfs

S3 Cluster FS

Cache-enabled file system backed by S3, with support for multiple hosts
accessing at the same time.

## Compiling

	/usr/lib/qt5/bin/qmake
	make

## Configuring

If you use AWS command line, configuration can be as easy as:

	ln -s ../.aws ~/.config/aws
