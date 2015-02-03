#include "QtFuse.hpp"
#include <QObject>

class QtFuseNode: public QObject {
	Q_OBJECT;

public:
	QtFuseNode(QtFuse &fuse, struct stat *attr, QString name, QtFuseNode *_parent);
	const struct stat *getAttr() const;
	bool isRoot() const;
	fuse_ino_t getIno() const;

protected:
	QtFuse &fuse;
	fuse_ino_t ino;
	struct stat attr;
	QString name;
	QtFuseNode *parent;
};

