#include "qquickvtkrenderer.h"
#include <vtkCamera.h>
#include <vtkProperty.h>
#include <vtkActor.h>
#include <vtkCylinderSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkInteractorStyleTrackballCamera.h>

#include <QVTKInteractor.h>
#include <QQuickWindow>
#include "QQuickVtk.h"
#include <QEvent>
#include <QThread>

#include <QOpenGLWindow>
#include <QVTKOpenGLWindow.h>
#include <QVTKInteractorAdapter.h>
#include <QWindow>
#include <QQuickWindow>
vtkSmartPointer<vtkActor> get_polydata_actor()
{
    vtkSmartPointer<vtkCylinderSource> cylinder = vtkSmartPointer<vtkCylinderSource>::New();
    vtkSmartPointer<vtkPolyDataMapper> cylinderMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    cylinder->SetResolution(8);
    cylinderMapper->SetInputConnection(cylinder->GetOutputPort());
    vtkSmartPointer<vtkActor> cylinderActor = vtkSmartPointer<vtkActor>::New();
    cylinderActor->SetMapper(cylinderMapper);
    //    cylinderActor->GetProperty()->SetColor(.1,.2,.3);
    cylinderActor->RotateX(30.0);
    cylinderActor->RotateY(-45.0);
    return cylinderActor;
}
QQuickVtkRenderer::QQuickVtkRenderer(const QQuickVtk* parent)
{
    p = const_cast<QQuickVtk*>(parent);
    renderer = vtkSmartPointer<vtkRenderer>::New();
    renderWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
//    interactor = vtkSmartPointer<vtkGenericRenderWindowInteractor>::New();
    interactor = vtkSmartPointer<QVTKInteractor>::New();
    qtSlotConnect = vtkSmartPointer<vtkEventQtSlotConnect>::New();
    renderWindow->OpenGLInit();
    renderWindow->OpenGLInitState();
    renderWindow->OpenGLInitContext();
    renderWindow->AddRenderer(renderer);

    interactor->Initialize();
    interactor->SetRenderWindow(renderWindow);
    interactor->SetEnableRender(false);

    this->InteractorAdapter = new QVTKInteractorAdapter(this);
    this->InteractorAdapter->SetDevicePixelRatio(parent->window()->devicePixelRatio(),interactor);

    vtkSmartPointer<vtkInteractorStyleTrackballCamera> style = vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
    style->SetDefaultRenderer(renderer);
    style->SetInteractor(interactor);
    interactor->SetInteractorStyle(style);

    resetCamera();
    auto cylinderActor = get_polydata_actor();
    renderer->AddActor(cylinderActor);

    connect(parent,&QQuickVtk::qevent,this,&QQuickVtkRenderer::event,Qt::DirectConnection);

}

bool QQuickVtkRenderer::event(QEvent* e)
{
    switch (e->type())
    {
      case QEvent::MouseMove:
      case QEvent::MouseButtonPress:
      case QEvent::MouseButtonRelease:
      case QEvent::MouseButtonDblClick:
        // skip events that are explicitly handled by overrides to avoid duplicate
        // calls to InteractorAdapter->ProcessEvent().
        m_mouseEvent = QSharedPointer<QMouseEvent>::create(*static_cast<QMouseEvent*>(e));
        m_mouseEvent->ignore();
        break;
      case QEvent::Resize:
        // we don't let QVTKInteractorAdapter process resize since we handle it
        // in this->recreateFBO().
        break;
      default:
        if (this->renderWindow && this->renderWindow->GetInteractor())
        {
          return this->InteractorAdapter->ProcessEvent(e, this->renderWindow->GetInteractor());
        }
    }

    return true;
}



bool QQuickVtkRenderer::ProcessEvent(QEvent* e)
{
  if (this->renderWindow && this->renderWindow->GetReadyForRendering())
  {
    return this->InteractorAdapter->ProcessEvent(e, this->renderWindow->GetInteractor());
  }
  return false;
}
void QQuickVtkRenderer::synchronize(QQuickFramebufferObject* qfbitem)
{
    auto fboItem = qobject_cast<QQuickVtk *>(qfbitem);
    if (m_mouseEvent && !m_mouseEvent->isAccepted())
    {
        ProcessEvent(m_mouseEvent.data());
        m_mouseEvent->accept();
    }

}
QQuickVtkRenderer::~QQuickVtkRenderer()
{
}


void QQuickVtkRenderer::render()
{
    renderWindow->Render();
}

QOpenGLFramebufferObject *QQuickVtkRenderer::createFramebufferObject(const QSize &size)
{
    QOpenGLFramebufferObjectFormat format;
    format.setAttachment(QOpenGLFramebufferObject::Depth);
    format.setSamples(0);
    renderWindow->SetSize(size.width(), size.height());
    return new QOpenGLFramebufferObject(size, format);
}

void QQuickVtkRenderer::resetCamera()
{
//    QOpenGLContext * context = QOpenGLContext::currentContext();
    renderWindow->OpenGLInitState();
    renderWindow->MakeCurrent();
    openGLFunctions.initializeOpenGLFunctions();
    openGLFunctions.glUseProgram(0);
    openGLFunctions.glGetString(GL_VERSION);
    qDebug()<<"OpenGL:"<< reinterpret_cast<const char*>(openGLFunctions.glGetString(GL_VERSION));

//    renderer->ResetCameraClippingRange();
////    renderer->GetActiveCamera()->SetPosition(0, 0, 1000);
////    renderer->GetActiveCamera()->SetFocalPoint(0.0, 0.0, 0.0);
////    renderer->GetActiveCamera()->SetViewUp(0.0, 1, 0.0);
////    renderer->SetBackground(.8,.8,.8);
    renderer->SetBackground(.1, .2, .3);
    renderer->SetBackground2(.1, .2, .3);
    renderer->GradientBackgroundOn();
}
void QQuickVtkRenderer::openGLInitState()
{
    qDebug()<<"openGLInitState";
    this->renderWindow->OpenGLInitState();
    this->renderWindow->MakeCurrent();
    this->openGLFunctions.initializeOpenGLFunctions();
    this->openGLFunctions.glUseProgram(0);
}
