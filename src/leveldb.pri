# pro file for leveldb compilation through Qt

# This method of finding leveldb files is based on source of leveldb/build_detect_platform
# We modify this a bit to use what qmake already provides us, and avoid having to use leveldb's
# own thing :)

# ensure leveldb is there
!exists(leveldb/README) {
	message("Checking out submodules")
	system("git submodule init")
	system("git submodule update")
}

LEVELDB_SOURCE=leveldb
LEVELDB_DIRS=$${LEVELDB_SOURCE}/db $${LEVELDB_SOURCE}/util $${LEVELDB_SOURCE}/table
LEVELDB_PRUNE_TEST="-name *test*.cc -prune"
LEVELDB_PRUNE_BENCH="-name *_bench.cc -prune"
LEVELDB_PRUNE_TOOL="-name leveldb_main.cc -prune"

# this is a bit tricky but allows finding files without relying on unix tools such as find
LEVELDB_SOURCES=$$files($${LEVELDB_SOURCE}/db/*.cc) $$files($${LEVELDB_SOURCE}/util/*.cc) $$files($${LEVELDB_SOURCE}/table/*.cc)
LEVELDB_SOURCES=$$find(LEVELDB_SOURCES,^((?!test).)*$)
LEVELDB_SOURCES=$$find(LEVELDB_SOURCES,^((?!_bench.cc).)*$)
LEVELDB_SOURCES=$$find(LEVELDB_SOURCES,^((?!leveldb_main.cc).)*$)
win32:LEVELDB_SOURCES=$$find(LEVELDB_SOURCES,^((?!posix).)*$)
LEVELDB_SOURCES = $$join(LEVELDB_SOURCES," src/","src/","")
LEVELDB_SOURCES = $$split(LEVELDB_SOURCES," ")

INCLUDEPATH += src/leveldb/include

LEVELDB_DEFINES += LEVELDB_ATOMIC_PRESENT

!win32:LEVELDB_DEFINES += LEVELDB_PLATFORM_POSIX
!win32:LEVELDB_SOURCES += src/leveldb/port/port_posix.cc
win32:LEVELDB_DEFINES += LEVELDB_PLATFORM_WIN32
win32:LEVELDB_SOURCES += src/leveldb_extra/port/port_win.cc
win32:LEVELDB_CXXFLAGS += -Isrc/leveldb_extra

x11:LEVELDB_DEFINES += OS_LINUX
osx:LEVELDB_DEFINES += OS_MACOSX
win32:LEVELDB_DEFINES += OS_WIN

CONFIG(release, debug|release) {
	LEVELDB_CXXFLAGS += -O2
	LEVELDB_DEFINES += NDEBUG
}

LEVELDB_CXXFLAGS += -pipe -std=c++0x -fPIC -Isrc/leveldb -Isrc/leveldb/include
LEVELDB_CXXFLAGS += -fno-builtin-memcmp -pthread
LEVELDB_CXXFLAGS += $$join(LEVELDB_DEFINES," -D","-D","")

leveldb.name = LevelDB
leveldb.input = LEVELDB_SOURCES
leveldb.dependency_type = TYPE_C
leveldb.variable_out = OBJECTS
leveldb.output = ${QMAKE_VAR_OBJECTS_DIR}leveldb_${QMAKE_FILE_IN_BASE}$${first(QMAKE_EXT_OBJ)}
leveldb.commands = $${QMAKE_CXX} $${LEVELDB_CXXFLAGS} -c ${QMAKE_FILE_IN} -o ${QMAKE_FILE_OUT}
silent:leveldb.commands = @echo compiling ${QMAKE_FILE_IN} && $${leveldb.commands}
QMAKE_EXTRA_COMPILERS += leveldb
