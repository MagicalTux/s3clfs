#include <QCoreApplication>
#include <QCommandLineParser>
#include <QStandardPaths>
#include <S3FS.hpp>

int main(int argc, char *argv[]) {
	QCoreApplication app(argc, argv);
	QCoreApplication::setApplicationName("s3clfs");
	QCoreApplication::setApplicationVersion("1.0");

	// arguments
	QCommandLineParser parser;
	parser.setApplicationDescription("S3 Cluster FileSystem");
	parser.addHelpOption();
	parser.addVersionOption();
	parser.addPositionalArgument("bucket", QCoreApplication::translate("main", "Name of bucket on AWS S3"));
	parser.addPositionalArgument("path", QCoreApplication::translate("main", "Path where to mount file system"));
	parser.addOption({{"o", "options"}, QCoreApplication::translate("main", "comma-separated list of mount options"), "options"}); // if compilation fails here, make sure you have Qt 5.4+
	parser.addOption({{"q", "queue"}, QCoreApplication::translate("main", "URL of SQS queue holding events for this bucket"), "queue"});
	parser.addOption({{"c", "cache"}, QCoreApplication::translate("main", "Where to store cache, default %1/<bucket>").arg(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)), "cache"});

	parser.process(app);

	const QStringList args = parser.positionalArguments();
	if (args.length() != 2) {
		parser.showHelp(1);
		Q_UNREACHABLE();
	}
	// bucket is args.at(0), path is args.at(1)
	
	S3FS s3clfs(args.at(0).toLocal8Bit(), args.at(1).toLocal8Bit(), parser.value(QStringLiteral("queue")).toLocal8Bit(), parser.value(QStringLiteral("options")).toLocal8Bit());

	return app.exec();
}

