#include "qquickvtk.h"

#include <QDebug>
#include <QQuickWindow>
#include <QThread>
#include "QQuickVtkRenderer.h"
QQuickVtk::QQuickVtk(QQuickItem* parent):
    QQuickFramebufferObject(parent)
{
    // 以下3行是C++处理鼠标事件的前提，否则所有(C++)鼠标事件直接忽略
    setAcceptHoverEvents(true);//返回此项目是否接受悬停事件
    setAcceptedMouseButtons(Qt::AllButtons); //将此项目接受的鼠标按钮设置为按钮
    setFlag(ItemAcceptsInputMethod, true);
    this->setMirrorVertically(true); // QtQuick and OpenGL have opposite Y-Axis directions
}

QQuickFramebufferObject::Renderer *QQuickVtk::createRenderer() const
{
    return new QQuickVtkRenderer(this);
}
bool QQuickVtk::event(QEvent *e)
{
    emit qevent(e);
    update();
    return true;
}

QSurfaceFormat QQuickVtk::defaultFormat()
{
  QSurfaceFormat fmt;
  fmt.setRenderableType(QSurfaceFormat::OpenGL);
  fmt.setVersion(3, 2);
  fmt.setProfile(QSurfaceFormat::CoreProfile);
  fmt.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
  fmt.setRedBufferSize(1);
  fmt.setGreenBufferSize(1);
  fmt.setBlueBufferSize(1);
  fmt.setDepthBufferSize(1);
  fmt.setStencilBufferSize(0);
  fmt.setAlphaBufferSize(1);
  fmt.setStereo(false);
  fmt.setSamples(vtkOpenGLRenderWindow::GetGlobalMaximumNumberOfMultiSamples());
  return fmt;
}
