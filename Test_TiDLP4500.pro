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
