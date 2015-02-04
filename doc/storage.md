Storage on S3 includes two prefixes:

- data/
- metadata/

Files under data are named after hash (depending on hash algo), for example
data/a/ab/abcdef0123456789.

A file named metadata/format.json contains details on the exact storage format
including hash algo in use, version, encryption, block size, etc.

Metadata also contains hashed data, especially a file at metadata/0/00/0000...
which contains files from the root directory. Meta-data are stored as inodes,
which are generated through microtime precision integers. The directory name
is actually the uint64 value.

For example storing a file under inode 142288133207962 would result in a file
name of:

	metadata/a/9a/0000816909a2b79a

Root directory is stored at inode 1:

	metadata/1/01/0000000000000001

Each metadata subdir contains latest version of the contents of the dir, and
details on what caused the change to happen. When a directory is changed, a
new files is uploaded and the previous one is discarded.

If multiple files are found, then the changes (log) from each one are merged
then a new file is created and the old one is discarded.

This means that to read a directory, two operations are required:

- list appropriate metadata directory
- get all files in there

However to improve speed, the system will initially (and from times to times)
list all files under metadata directory and keep a cache of these.
This means that in most times, only one request will be required to get a
directory contents (if the get fails, it means the directory contents changed
and we need to list the meta-dir so we know what was in there).

Note that this also allows to re-validate the whole cache in one single query.

