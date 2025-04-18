QT -= gui

CONFIG += c++11 console
CONFIG -= app_bundle

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
#DEFINES -= UNICODE
DEFINES += fcl_EXPORTS
DEFINES += CCD_EXPORT
DEFINES += CCD_STATIC_DEFINE
msvc:QMAKE_CXXFLAGS += /utf-8
SOURCES += \
        main.cpp
#fcl_EXPORTS
# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


INCLUDEPATH += D:\Library\Eigen3\3.4.0\vc142\include\eigen3 \

INCLUDEPATH += D:\Library\libccd\2.1.0\msvc16\include \

LIBS +=-LD:\Library\libccd\2.1.0\msvc16\lib -lccd

INCLUDEPATH += D:\Library\fcl\0.7.0\msvc16\include \

LIBS +=-LD:\Library\fcl\0.7.0\msvc16\lib -lfcl

INCLUDEPATH += D:\Library\Assimp\include \

LIBS +=D:\Library\Assimp\lib\x64\assimp-vc143-mt.lib


INCLUDEPATH += D:\Library\VTK\9.4.1\msvc16\include\vtk-9.4 \

LIBS +=D:\Library\VTK\9.4.1\msvc16\lib\*.lib
