#include "QtFuse.hpp"
#include <QObject>

class QtFuseNode: public QObject {
	Q_OBJECT;

public:
	QtFuseNode(QtFuse &fuse, struct stat *attr, QString name, QtFuseNode *_parent);
	const struct stat *getAttr() const;
	bool isRoot() const;

protected:
	QtFuse &fuse;
	int ino;
	struct stat attr;
	QString name;
	QtFuseNode *parent;
};

