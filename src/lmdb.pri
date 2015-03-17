# pro file for lmdb compilation through Qt

# This method of finding lmdb files is based on source of lmdb/build_detect_platform
# We modify this a bit to use what qmake already provides us, and avoid having to use lmdb's
# own thing :)

# ensure lmdb is there
!exists(lmdb/libraries/liblmdb/CHANGES) {
	message("Checking out submodules")
	system("git submodule init")
	system("git submodule update")
}

LMDB_SOURCE=src/lmdb

LMDB_SOURCES=$${LMDB_SOURCE}/libraries/liblmdb/mdb.c $${LMDB_SOURCE}/libraries/liblmdb/midl.c

INCLUDEPATH += src/lmdb/libraries/liblmdb

LMDB_DEFINES += LMDB_ATOMIC_PRESENT

CONFIG(release, debug|release) {
	LMDB_CXXFLAGS += -O2
} else {
	LMDB_CXXFLAGS += -DMDB_DEBUG=1
}

LMDB_CXXFLAGS += -pipe -fPIC -Isrc/lmdb/libraries/liblmdb
#LMDB_CXXFLAGS += $$join(LMDB_DEFINES," -D","-D","")

lmdb.name = LMDB
lmdb.input = LMDB_SOURCES
lmdb.dependency_type = TYPE_C
lmdb.variable_out = OBJECTS
lmdb.output = ${QMAKE_VAR_OBJECTS_DIR}lmdb_${QMAKE_FILE_IN_BASE}$${first(QMAKE_EXT_OBJ)}
lmdb.commands = $${QMAKE_CC} $${LMDB_CXXFLAGS} -c ${QMAKE_FILE_IN} -o ${QMAKE_FILE_OUT}
silent:lmdb.commands = @echo compiling ${QMAKE_FILE_IN} && $${lmdb.commands}
QMAKE_EXTRA_COMPILERS += lmdb
