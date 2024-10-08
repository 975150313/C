#-------------------------------------------------
#
# Project created by QtCreator 2020-10-29T14:57:27
#
#-------------------------------------------------

QT       += core gui xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = GuiCreateNidus
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
msvc:QMAKE_CXXFLAGS += /utf-8
# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        GuiCreateNidus.cpp \
    createnidus.cpp \
    SketchNodee.cpp \
    thpnvtk.cpp \
    vtkimagetracerwidgetnew.cpp \
    vtkinteractorstyleimagenew.cpp \
    XMLTool.cpp

HEADERS += \
        GuiCreateNidus.h \
    createnidus.h \
    SketchNode.h \
    thpnvtk.h \
    vtkImageTracerWidgetNew.h \
    vtkInteractorStyleImageNew.h \
    XMLTool.h

FORMS += \
        GuiCreateNidus.ui

RESOURCES += \
    Resources/resource.qrc

CONFIG+=debug_and_release

CONFIG(release, debug|release){
QMAKE_CXXFLAGS_RELEASE = $$QMAKE_CFLAGS_RELEASE_WITH_DEBUGINFO
QMAKE_LFLAGS_RELEASE = $$QMAKE_LFLAGS_RELEASE_WITH_DEBUGINFO

#====================================Log4Qt=====================================
#INCLUDEPATH += $$(Library)\Log4Qt\include
#LIBS += $$(Library)\Log4Qt\lib\log4qt.lib

#=====================================VTK_ITK===================================
DEPENDPATH += D:\Library\VTK\8.2.0\release\bin


INCLUDEPATH += D:\Library\VTK\8.2.0\release\include\vtk-8.2 \

LIBS +=D:\Library\VTK\8.2.0\release\lib\*.lib
}
