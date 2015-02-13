#include <QByteArray>
#include <QList>

class QByteArrayList: public QList<QByteArray> {
public:
	QByteArrayList(const QList<QByteArray> &p): QList<QByteArray>(p) { }
	QByteArray join(char c) { QByteArray res; bool first = true; foreach(auto x, *this) { if (!first) res.append(c); else first = false; res.append(x); } return res; }
};
