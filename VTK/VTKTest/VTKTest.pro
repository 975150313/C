QT += quick quickcontrols2  qml
CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
msvc:QMAKE_CXXFLAGS += /utf-8
# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

HEADERS += \
    qquickvtk.h \
    qquickvtkrenderer.h

SOURCES += main.cpp \
    qquickvtk.cpp \
    qquickvtkrenderer.cpp

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


#=====================================VTK_ITK===================================
INCLUDEPATH += $(VTK_DIR)\Release\include \

INCLUDEPATH += $(VTK_DIR)\Release\include\vtk-8.2 \

DEPENDPATH += $$(VTK_DIR)\Release\bin \

DEPENDPATH += E:\Qt5.10.1\5.10.1\msvc2015_64\bin \

DEPENDPATH += E:\Qt5.10.1\5.10.1\msvc2015_64\qml \


CONFIG += debug_and_release

CONFIG(release,debug|release){

LIBS += $(VTK_DIR)\release\lib\vtkCommonCore-8.2.lib
LIBS += $(VTK_DIR)\release\lib\vtkCommonDataModel-8.2.lib
LIBS += $(VTK_DIR)\release\lib\vtkCommonExecutionModel-8.2.lib
LIBS += $(VTK_DIR)\release\lib\vtkCommonMisc-8.2.lib
LIBS += $(VTK_DIR)\release\lib\vtkCommonSystem-8.2.lib
LIBS += $(VTK_DIR)\release\lib\vtkDICOMParser-8.2.lib
LIBS += $(VTK_DIR)\release\lib\vtkFiltersCore-8.2.lib
LIBS += $(VTK_DIR)\release\lib\vtkFiltersGeneral-8.2.lib
LIBS += $(VTK_DIR)\release\lib\vtkFiltersSources-8.2.lib
LIBS += $(VTK_DIR)\release\lib\vtkGUISupportQt-8.2.lib
LIBS += $(VTK_DIR)\release\lib\vtkImagingCore-8.2.lib
LIBS += $(VTK_DIR)\release\lib\vtkImagingGeneral-8.2.lib
LIBS += $(VTK_DIR)\release\lib\vtkImagingStencil-8.2.lib
LIBS += $(VTK_DIR)\release\lib\vtkInteractionImage-8.2.lib
LIBS += $(VTK_DIR)\release\lib\vtkInteractionStyle-8.2.lib
LIBS += $(VTK_DIR)\release\lib\vtkInteractionWidgets-8.2.lib
LIBS += $(VTK_DIR)\release\lib\vtkIOGeometry-8.2.lib
LIBS += $(VTK_DIR)\release\lib\vtkIOImage-8.2.lib
LIBS += $(VTK_DIR)\release\lib\vtkRenderingContextOpenGL2-8.2.lib
LIBS += $(VTK_DIR)\release\lib\vtkRenderingCore-8.2.lib
LIBS += $(VTK_DIR)\release\lib\vtkRenderingFreeType-8.2.lib
LIBS += $(VTK_DIR)\release\lib\vtkRenderingVolumeOpenGL2-8.2.lib
LIBS += $(VTK_DIR)\release\lib\vtksys-8.2.lib
LIBS += $(VTK_DIR)\release\lib\vtkRenderingOpenGL2-8.2.lib
LIBS += $(VTK_DIR)\release\lib\vtkRenderingAnnotation-8.2.lib
LIBS += $(VTK_DIR)\release\lib\vtkCommonComputationalGeometry-8.2.lib
LIBS += $(VTK_DIR)\release\lib\vtkCommonTransforms-8.2.lib

LIBS += $(VTK_DIR)\release\lib\vtkDICOM-8.2.lib
}
#=====================================VTK_ITK===================================

