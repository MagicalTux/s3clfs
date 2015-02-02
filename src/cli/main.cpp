#include <QCoreApplication>
#include "TplFuse.h"

int main(int argc, char *argv[]) {
	QCoreApplication app(argc, argv);
	TplFuse tplfuse("/tmp/mnt");
	tplfuse.init();

	return app.exec();
}

