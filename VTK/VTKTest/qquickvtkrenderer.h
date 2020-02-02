#ifndef QQUICKVTKRENDERER_H
#define QQUICKVTKRENDERER_H

#include <QOpenGLFunctions>
#include <QQuickFramebufferObject>
#include <QOpenGLFramebufferObject>

#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkGenericRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <QVTKInteractor.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkEventQtSlotConnect.h>
class QQuickVtk;
class QVTKInteractorAdapter;
class QQuickVtkRenderer : public QObject,
        public QQuickFramebufferObject::Renderer
{
    Q_OBJECT
public:
    QQuickVtkRenderer(const QQuickVtk* parent = nullptr);
    ~QQuickVtkRenderer();
    void render() override;
    QOpenGLFramebufferObject *createFramebufferObject(const QSize &size) override;
    void synchronize(QQuickFramebufferObject *) override;
    void openGLInitState();

    void resetCamera();
signals:

public slots:
    bool ProcessEvent(QEvent* e);
    bool event(QEvent* e);
private:
    QQuickVtk* p;

    QOpenGLFunctions openGLFunctions;
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> renderWindow;
//    vtkSmartPointer<vtkGenericRenderWindowInteractor> interactor;
    vtkSmartPointer<QVTKInteractor> interactor;
    vtkSmartPointer<vtkRenderer> renderer;
    vtkSmartPointer<vtkEventQtSlotConnect> qtSlotConnect;
    QVTKInteractorAdapter* InteractorAdapter;
    QSharedPointer<QMouseEvent> m_mouseEvent;
};

#endif // QQUICKVTKRENDERER_H
