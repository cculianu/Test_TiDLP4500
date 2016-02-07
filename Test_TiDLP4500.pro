#-------------------------------------------------
#
# Project created by QtCreator 2016-02-06T10:03:32
#
#-------------------------------------------------

QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Test_TiDLP4500
TEMPLATE = app


SOURCES += main.cpp\
        dialog.cpp \
    RenderWindow.cpp \
    Util.cpp

HEADERS  += dialog.h \
    RenderWindow.h \
    Util.h

FORMS    += dialog.ui

RESOURCES += \
    shaders.qrc \
    misc.qrc

win32 {
    RC_FILE += WinResources.rc
    QMAKE_CFLAGS_RELEASE -= /O2 /O1 -O1 -O2
    QMAKE_CXXFLAGS_RELEASE -= /O2 /O1 -O1 -O2
    QMAKE_CFLAGS_RELEASE += -Ox
    QMAKE_CXXFLAGS_RELEASE += -Ox
}

macx {
    ICON = icon.icns
    DISTFILES += icon.icns
}
