#!/usr/bin/make

# qmake CONFIG+=cli
# qmake CONFIG+=test
#
# (build will default to cli)

lessThan(QT_MAJOR_VERSION, 5) {
	error("S3ClFS requires Qt5.4 or better")
}
equals(QT_MAJOR_VERSION, 5) {
	lessThan(QT_MINOR_VERSION, 4) {
		error("S3ClFS requires Qt5.4 or better")
	}
}

cli {
	BUILD_TYPE=cli
} else:test {
	BUILD_TYPE=test
} else {
	message("No build selection done, defaulting to CLI")
	BUILD_TYPE=cli
}

CONFIG(release, debug|release) {
	BUILD_MODE=release
} else {
	BUILD_MODE=debug
}

# shared settings
TEMPLATE = app
DEPENDPATH += src/core
INCLUDEPATH += src/core
QMAKE_CXXFLAGS += -std=c++11
QT += network
DEFINES += S3CLFS_$$upper($${BUILD_TYPE})
SOURCES += src/$${BUILD_TYPE}/main.cpp
CONFIG += silent
LIBS += -lfuse

include(src/leveldb.pri)

x11 {
	CONFIG += link_pkgconfig
	PKGCONFIG += openssl
} else {
	LIBS += -lssl -lcrypto
}

# for MacOS
QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.9
QMAKE_MAC_SDK=macosx10.9

CPP_CLASSES = \
	core/Callback \
	core/Keyval \
	core/KeyvalIterator \
	core/QtFuse \
	core/QtFuseRequest \
	core/S3Fuse \
	core/S3FS

equals(BUILD_TYPE,cli) {
	TARGET = s3clfs
	QT -= gui widgets
	osx:CONFIG-=app_bundle
	CPP_CLASSES += \
		gui/MainWindow \
		gui/BankSelector \
		gui/CompletionPopup
}

equals(BUILD_TYPE,test) {
	TARGET = s3clfs-test
	QT -= gui widgets
	osx:CONFIG-=app_bundle
#	CPP_CLASSES += 
	# force debug mode
	BUILD_MODE=debug
}

# extra stuff for debug mode
equals(BUILD_MODE,debug) {
	CONFIG += debug
	DEFINES += S3CLFS_DEBUG
} else {
	DEFINES += QT_NO_DEBUG
}

# Add classes to sources/headers
for(class,CPP_CLASSES):exists(src/$${class}.cpp):SOURCES += src/$${class}.cpp
for(class,CPP_CLASSES):exists(src/$${class}.hpp):HEADERS += src/$${class}.hpp
for(class,CPP_CLASSES):exists(src/$${class}.ui):FORMS += src/$${class}.ui

DEPENDPATH += src/$${BUILD_TYPE}
OBJECTS_DIR = build/$${BUILD_TYPE}-$${BUILD_MODE}
MOC_DIR = build/$${BUILD_TYPE}-$${BUILD_MODE}
RCC_DIR = build/$${BUILD_TYPE}-$${BUILD_MODE}
UI_DIR = build/$${BUILD_TYPE}-$${BUILD_MODE}
INCLUDEPATH += build/$${BUILD_TYPE}-$${BUILD_MODE}

