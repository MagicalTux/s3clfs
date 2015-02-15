# s3clfs

S3 Cluster FS

Cache-enabled file system backed by S3, with support for multiple hosts
accessing at the same time.

Licensed under GPLv3. See attached LICENSE file for details.

## Compiling

S3ClFS requires Qt5.4+ (core, network and xmlpatterns) and Fuse 2.9+.

	qmake -qt=qt5 (or /usr/lib/qt5/bin/qmake)
	make

## Configuring

If you use AWS command line, configuration can be as easy as:

	ln -s ../.aws ~/.config/aws

If not, you will need to create a configuration file in ~/.config/aws/credentials with this contents:

	[default]
	aws_access_key_id = (your key id)
	aws_secret_access_key = (secret value)

## AWS config

You want to have the following:

- a bucket configured normally (see doc/aws_rule.txt for policy to give access to an IAM account)
- a SNS topic
- configure bucket to send notifications to the SNS topic
- a SQS queue per machine, with receive message wait time set to the maximum (20 secs)
- configure SNS to send message to each SQS queue
- make sure messages can go from the bucket to the SQS queue

Note that if you have only one host accessing a given bucket at a given time, you can opt to only create a bucket (with its right) and skip all the SNS/SQS setup.
