#include "GuiCreateNidus.h"
#include <QApplication>
#include <QFile>
#include <QDebug>

#include <vtkObject.h>
#include <QSurfaceFormat>
#include <QVTKOpenGLWidget.h>
#include <QVTKOpenGLNativeWidget.h>
#include <vtkOpenGLRenderWindow.h>
#include <vtkAutoInit.h>
VTK_MODULE_INIT(vtkRenderingOpenGL2)
VTK_MODULE_INIT(vtkInteractionStyle)
VTK_MODULE_INIT(vtkRenderingContextOpenGL2)
VTK_MODULE_INIT(vtkRenderingFreeType)

//#include "log4qt/logger.h"
//#include "log4qt/propertyconfigurator.h"
//#include "log4qt/logmanager.h"

int main(int argc, char *argv[])
{
    QSurfaceFormat::setDefaultFormat(QVTKOpenGLWidget::defaultFormat());
//    vtkObject::GlobalWarningDisplayOff();
    QApplication a(argc, argv);

//    Log4Qt::PropertyConfigurator::configureAndWatch(a.applicationDirPath() + "/log4qt.properties");
//    Log4Qt::LogManager::setHandleQtMessages(true);

    QStringList arguments = QCoreApplication::arguments();
    if(1 >= arguments.size())
    {
        qDebug() << "Arguments:" << arguments<<arguments.size();
        return 0;
    }

    QFile fs(":/main.qss");
    if (fs.open(QIODevice::ReadOnly))
    {
        a.setStyleSheet(fs.readAll());
    }
    GuiCreateNidus w;
    w.setDirectoryName(arguments.last().replace("\\","/"));
//    w.showMaximized();
    w.show();
    return a.exec();
}
