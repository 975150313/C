#-------------------------------------------------
#
# Project created by QtCreator 2018-12-04T20:04:08
#
#-------------------------------------------------

QT       -= gui

TARGET = BFactory
TEMPLATE = lib

DEFINES += BFACTORY_LIBRARY

DESTDIR     = $$PWD/../bin  #exe 或者dll 路径
MOC_DIR     = $$PWD/../temp #moc的产物放置何处
RCC_DIR     = $$PWD/../temp #rcc的产物放置何处
UI_DIR      = $$PWD/../temp #uic的产物放置何处
OBJECTS_DIR = $$PWD/../temp #编译器生成的.o（.obj）等文件放置
msvc:QMAKE_CXXFLAGS += /utf-8
# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        bfactory.cpp \
    bclass.cpp \
    ibclass.cpp

HEADERS += \
        bfactory.h \
        bfactory_global.h \ 
    ibclass.h \
    bclass.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
#=================================VLD 内存泄漏检测========================
#win32:CONFIG(debug, debug|release) {
#        INCLUDEPATH += $$quote(D:\Program Files (x86)\Visual Leak Detector\include)
#        LIBS += -L$$quote(D:\Program Files (x86)\Visual Leak Detector\lib\Win32)
#        LIBS += -lvld
#}
