#-------------------------------------------------
#
# Project created by QtCreator 2017-04-29T15:44:51
#
#-------------------------------------------------

QT       += core gui
CONFIG += c++11 console

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SpaceMouseDriver
TEMPLATE = app


SOURCES += main.cpp\
    gui/BaseSingleton.cpp \
    gui/BaseThread.cpp \
    gui/GeometryEngine.cpp \
    gui/MainWidget.cpp \
    gui/ServiceSpaceMouse.cpp

HEADERS  += \
    gui/BaseSingleton.h \
    gui/BaseThread.h \
    gui/GeometryEngine.h \
    gui/GlobSignal.h \
    gui/MainWidget.h \
    gui/ServiceSpaceMouse.h

RESOURCES += \
    gui/resources/shaders.qrc \
    gui/resources/textures.qrc

win32 {
!contains(QMAKE_TARGET.arch, x86_64) {
        message("Windows - x86 build")
        LIBS += -L"$$PWD/lib/windows/x86/"
    } else {
        message("Windows - x86_64 build")
        LIBS += -L"$$PWD/lib/windows/x64/"
    }
}

linux {
    message("Linux build")
    LIBS += -L"$$PWD/lib/linux" -lhidapi-hidraw
}


INCLUDEPATH += $$PWD\hidapi\include\hidapi \

LIBS += $$PWD\hidapi\lib\hidapi.lib



