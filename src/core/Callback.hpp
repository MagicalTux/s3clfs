#include <QObject>
#include <QGenericArgument>
#include <QPointer>

class Callback: public QObject {
	Q_OBJECT
public:
	Callback(QObject *obj, const char *member,
		QGenericArgument val0 = QGenericArgument(0),
		QGenericArgument val1 = QGenericArgument(),
		QGenericArgument val2 = QGenericArgument(),
		QGenericArgument val3 = QGenericArgument(),
		QGenericArgument val4 = QGenericArgument(),
		QGenericArgument val5 = QGenericArgument(),
		QGenericArgument val6 = QGenericArgument(),
		QGenericArgument val7 = QGenericArgument(),
		QGenericArgument val8 = QGenericArgument(),
		QGenericArgument val9 = QGenericArgument());

public slots:
	void trigger();

private:
	QPointer<QObject> obj;
	QByteArray member; // using QByteArray to ensure copy is kept
	QGenericArgument val0, val1, val2, val3, val4, val5, val6, val7, val8, val9;
};

