#ifndef CREATENIDUS_H
#define CREATENIDUS_H

#include <QtWidgets>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QEvent>

#include <QVTKOpenGLNativeWidget.h>
#include <QVTKOpenGLWidget.h>
#include <vtkImageTracerWidget.h>
#include <vtkRenderer.h>
#include <vtkImageActor.h>
#include <vtkImageSliceMapper.h>
#include <vtkImageReslice.h>
#include <vtkEventQtSlotConnect.h>
#include <vtkDICOMReader.h>
#include <vtkCornerAnnotation.h>
#include <vtkCamera.h>

#include "vtkImageTracerWidgetNew.h"
#include "vtkInteractorStyleImageNew.h"
#include "SketchNode.h"
class vtkRenderWindow;
class QVTKInteractor;
class vtkRenderWindowInteractor;
class vtkCamera;
class vtkBillboardTextActor3D;
class QTreeWidgetItem;
class QString;
class CreateNidus : public QVTKOpenGLWidget
{
    Q_OBJECT
public:
    explicit CreateNidus(QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
    int SetDirectoryName(const char* path);
    void SetEnabledTracer();
    void Init();
    void SetWindowLevel(int ww, int wl);
    void AddLine(vtkPoints* points,
                 vtkActor* actor,
                 vtkPolyData* data,
                 vtkCellArray* cells);
    QString CreateNodeName();
    QString CreateUUID();

    int AddSketchNodeXml(SketchNode node);
    int DeleteSketchNodeXml(SketchNode node);
    bool AddSketchNode(QString name);
    bool findSketchNode(QString name);
    int getSketchNodeSize();
    QString getSketchNodeName();
    int getSketchNodeSliceNo();
    int insertSketchNode(QString uid, QString name);
    int saveSketchNode(bool savePatientInfoXml=false);
    int deleteSTL(QString name, QString uid);
    int deleteSketchChildNode(QString name, QString uid, bool isTopNode);
    int updateSketchNodeIndex(QString name, QString uid, int index);
    int getSliceNumber();
    int setSliceNumber(int value);

    void showSketchNode(const QString& key);
    void changeSketchNodeName(const QString& key);
    void addPatientXml(QString path, int modelid, QString name);
    void readSketchNodeXML();
signals:
    QTreeWidgetItem* addItem(const SketchNode& node);
    void setSliceNo(int value);
    void setRange(int min, int max);
public slots:
    void Render();
    void Execute(vtkObject* caller, unsigned long eventId, void* clientData, void* callData);
protected:
    void wheelEvent(QWheelEvent*event) override;//滚轮
private:
    vtkCamera* camera;
    vtkNew<vtkRenderer> renderer;
    vtkNew<vtkImageActor> imageActor;
    vtkNew<vtkImageReslice> imageReslice;
    vtkNew<vtkImageTracerWidgetNew> tracer;
    vtkNew<vtkCornerAnnotation> cornerAnnotation;

    vtkRenderWindow* renderWindow;
    vtkRenderWindowInteractor* interactor;
//    QVTKInteractor* interactor;

    vtkNew<vtkInteractorStyleImageNew> style;
    vtkNew<vtkEventQtSlotConnect> qtSlotConnect;
    vtkSmartPointer<vtkDICOMReader> reader;
    int sliceNumber =0;
    int min =0;
    int max =0;
    double point[3] = {256, 256, 0};
    double range[2] = {0, 0};
    int extent[6] = {0, 0, 0, 0, 0, 0};
    double bounds[6] = {0, 0, 0, 0, 0, 0};
    double spacing[3] = {0, 0, 0};
    double origin[3] = {0, 0, 0};
    std::map<std::string, SketchNodeList> m_sketchDataMap;
    QMap<QString,SketchNodeList> sketchNodeMap;
    QString sketchNodeName;
    QString directoryName;

    QString m_SketchNodeXmlPath;
    QString m_PatientInfoXmlPath;
};

#endif // CREATENIDUS_H
