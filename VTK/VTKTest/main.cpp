#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "QQuickVtk.h"

#include <vtkObject.h>
#include <vtkFileOutputWindow.h>
#include <vtkOpenGLRenderWindow.h>
#include <vtkAutoInit.h>
VTK_MODULE_INIT(vtkRenderingOpenGL2)
VTK_MODULE_INIT(vtkInteractionStyle)
VTK_MODULE_INIT(vtkRenderingContextOpenGL2)
VTK_MODULE_INIT(vtkRenderingFreeType)
int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);
    QSurfaceFormat::setDefaultFormat(QQuickVtk::defaultFormat());
    qmlRegisterType<QQuickVtk>("QtQuick.VtkItem", 1, 0, "VtkItem");
    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

//    QQmlContext* ctxt = engine.rootContext();
//    QQuickVtk quickVtk;
//    ctxt->setContextProperty("quickVtk", &quickVtk);
    return app.exec();
}
