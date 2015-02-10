#include <QCoreApplication>
#include <QCommandLineParser>
#include <QStandardPaths>
#include <S3FS.hpp>

/*  S3ClFS - AWS S3 backed cluster filesystem
 *  Copyright (C) 2015 Mark Karpeles
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
	parser.addOption({{"c", "cache"}, QCoreApplication::translate("main", "Where to store cache, default %1/s3clfs-<bucket>").arg(QStandardPaths::writableLocation(QStandardPaths::TempLocation)), "cache"});

	parser.process(app);

	const QStringList args = parser.positionalArguments();
	if (args.length() != 2) {
		parser.showHelp(1);
		Q_UNREACHABLE();
	}
	// bucket is args.at(0), path is args.at(1)
	
	S3FS s3clfs(args.at(0).toLocal8Bit(), args.at(1).toLocal8Bit(), parser.value(QStringLiteral("queue")).toLocal8Bit(), parser.value(QStringLiteral("options")).toLocal8Bit(), parser.value(QStringLiteral("cache")));

	return app.exec();
}

