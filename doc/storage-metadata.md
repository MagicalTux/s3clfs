# Metadata

Inodes contain base information, and possibly extra information.

Extra information depend on the inode type (found in the base information).

- Directories: extra information is a table of <filename> => <inodes> streamed as file by file (QByteArray filename, quint64 inode)
- Files: extra info is table of <offset> => <data hash> streamed as quint64 offset => QByteArray hash
- Symlink: extra info is symlink target stored in key+"\x00", streamed as QByteArray

## storage on S3

Metadata is stored on the S3 bucket as a single file.

sample filename:

	metadata/1/01/0000000000000001/0000816909a2b79a.dat

The second part of the filename corresponds to the timestamp of the latest
modification. If a single inode has multiple files, it means they have to be
merged to form a single file.

Each inode should contain a log of all changes that occured within 1 hour of
the inode being stored. If multiple files are found within an inode, they
should be all read and the data merged based on what is available in the log of
each version.

The data stored on S3 is streamed through QDataStream, and starts with the
S3FS_Obj stream (used to find out the nature of that inode). Then follows the
meta-information (contents) of that file, and finally its recent history.

## storage on leveldb (local cache)


