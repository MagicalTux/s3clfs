
QMAKE=qmake
if [ -f /usr/lib/qt5/bin/qmake ]; then
	QMAKE=/usr/lib/qt5/bin/qmake
elif [ -f "${HOME}/Qt5.3.1/5.3/clang_64/bin/qmake" ]; then
	QMAKE="${HOME}/Qt5.3.1/5.3/clang_64/bin/qmake"
fi
