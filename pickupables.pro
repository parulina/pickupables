OBJECTS_DIR =		.obj
MOC_DIR =		.obj/moc
DESTDIR =		bin

TARGET = 		pickupables

QT = 			core gui widgets
QMAKE_CXXFLAGS +=	-Wfatal-errors
CONFIG += 		c++11 debug_and_release
RESOURCES +=		resources.qrc

# Windows: disable console in Makefile.Release
win32:CONFIG(release, debug|release) {
	CONFIG += static windows
}
win32:CONFIG(debug, debug|release) {
	CONFIG += console
}

HEADERS += 	$$files(src/*.h)
SOURCES += 	$$files(src/*.cpp)

VERSION_MAJOR =	0
VERSION_MINOR =	0
VERSION_PATCH =	1
VERSION = 	$${VERSION_MAJOR}.$${VERSION_MINOR}.$${VERSION_PATCH}
DEFINES +=	VERSION=\\\"$$VERSION\\\"
