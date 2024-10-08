#include "createnidus.h"
#include <QUuid>
#include <QGridLayout>

#include <vtkRenderWindow.h>
#include <vtkInteractorStyleImage.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkProperty.h>
#include <vtkGlyphSource2D.h>
#include <vtkTextProperty.h>
#include <vtkCallbackCommand.h>
#include <vtkCamera.h>
#include <vtkTransform.h>
#include <vtkBillboardTextActor3D.h>
#include <vtkImageProperty.h>
#include <vtkInformation.h>
#include <vtkDICOMMetaData.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkDICOMApplyPalette.h>
#include <vtkImageMapToColors.h>
#include <vtkDICOMCTRectifier.h>
#include <vtkLookupTable.h>
#include <vtkImageResliceMapper.h>

#include "thpnvtk.h"
#include "XMLTool.h"
CreateNidus::CreateNidus(QWidget *parent, Qt::WindowFlags flags) : QVTKOpenGLWidget(parent, flags)
{
    Init();
}

void CreateNidus::Init()
{
    renderWindow = this->GetRenderWindow();//创建 RenderWindow
    interactor = this->GetInteractor();    //获取 Interactor
    renderWindow->AddRenderer(renderer);
    interactor->SetRenderWindow(renderWindow);
    interactor->SetInteractorStyle(style);
    style->SetInteractionModeToImage2D();
    camera = renderer->GetActiveCamera();
    renderer->ResetCamera();
    //
    tracer->SetCurrentRenderer(renderer);
    tracer->SetDefaultRenderer(renderer);
    tracer->GetGlyphSource()->SetColor(1, 0, 0);
    tracer->GetGlyphSource()->SetScale(0);
    tracer->GetGlyphSource()->SetRotationAngle(0);
    tracer->GetGlyphSource()->Modified();
    tracer->GetLineProperty()->SetColor(1, 0, 0);
    tracer->GetHandleProperty()->SetColor(1, 0, 0);
    tracer->GetHandleProperty()->SetOpacity(0);
    tracer->GetHandleProperty()->SetPointSize(0.1);
    tracer->GetHandleProperty()->SetEdgeVisibility(0);
    tracer->GetHandleProperty()->SetShading(0);
    tracer->GetHandleProperty()->Modified();
    tracer->SetHandleMiddleMouseButton(false);
    tracer->SetHandleRightMouseButton(false);
    tracer->SetCaptureRadius(100);
    tracer->SetProjectToPlane(1);//强制手柄位于特定的正交平面上。
    tracer->SetProjectionNormalToZAxes();
    tracer->SetAutoClose(1);
    tracer->SetInteractor(interactor);//关键代码
    tracer->SetViewProp(imageActor);//关键代码
    tracer->Off();
    tracer->SetOpenGLWidget(this);
    tracer->Modified();
    //    tracer->SnapToImageOn();
    //
    cornerAnnotation->GetTextProperty()->SetFontFamilyToCourier();
    cornerAnnotation->GetTextProperty()->SetFontSize(10);
    cornerAnnotation->GetTextProperty()->SetColor(1, 1, 0);
    int WW = 256;
    int WL = 128;

    QString msg = QString("Window: %1 Level:%2").arg(2000).arg(-600);
    cornerAnnotation->SetText(0, qPrintable(msg));
    //    cornerAnnotation->SetText(4, "P");
    //    cornerAnnotation->SetText(5, "L");
    //    cornerAnnotation->SetText(6, "R");
    //    cornerAnnotation->SetText(7, "A");

    renderer->AddViewProp(imageActor);
    renderer->AddViewProp(cornerAnnotation);

    double leftToRight[3] = {1, 0, 0};// 正负号左右交换
    double viewUp[3] = {0, 1, 0};// 切换相机上下
    style->SetInteractor(interactor);
    style->SetCurrentRenderer(renderer);
    style->SetDefaultRenderer(renderer);
    style->SetImageOrientation(leftToRight, viewUp);
    style->SetOpenGLWidget(this);
    style->SetEnabled(1);

    renderer->SetBackground(0.2, 0.2, 0.2);
    renderWindow->Render();

    //    qtSlotConnect->Connect(interactor,
    //                           vtkCommand::StartInteractionEvent,
    //                           this,SLOT(Execute(vtkObject*, unsigned long, void*, void*)));
    //    qtSlotConnect->Connect(interactor,
    //                           vtkCommand::EndInteractionEvent,
    //                           this,SLOT(Execute(vtkObject*, unsigned long, void*, void*)));

}

void CreateNidus::showSketchNode(const QString& key)
{
    for(auto iterator =sketchNodeMap.begin(); iterator != sketchNodeMap.end(); ++iterator)
    {
        QString name = iterator.key();
        SketchNodeList& list = iterator.value();
        for(auto it = list.begin(); it != list.end(); ++it)
        {
            auto node = (*it);
            if(node.sliceNo == (sliceNumber + 1) && name ==key)
            {
                node.lineActor->SetVisibility(1);
                node.lineboardText->SetVisibility(1);
            }
            else
            {
                node.lineActor->SetVisibility(0);
                node.lineboardText->SetVisibility(0);
            }
        }
    }
}

void CreateNidus::changeSketchNodeName(const QString& key)
{
    this->sketchNodeName = key;
    this->Render();
}

int CreateNidus::getSliceNumber()
{
    return this->sliceNumber;
}

void CreateNidus::Render()
{
    showSketchNode(sketchNodeName);
    renderWindow->Render();
}

int CreateNidus::setSliceNumber(int value)
{
    auto deltaY = value - this->sliceNumber;
    this->sliceNumber = value;
    QString msg = QString("Slice No.  %1/%2").arg(sliceNumber + 1).arg(max + 1);
    cornerAnnotation->SetText(2, qPrintable(msg));
    //
    imageReslice->Update();
    auto matrix = vtkSmartPointer<vtkMatrix4x4>::New();
    matrix->Identity();
    matrix->SetElement(2, 3, spacing[2] * sliceNumber);
    imageReslice->SetResliceAxes(matrix);
    imageReslice->Update();
    interactor->Render();
    Render();
    return this->sliceNumber;
}

void CreateNidus::wheelEvent(QWheelEvent* event)
{
    if(event->delta() > 0)// 当滚轮远离使用者时
    {
        if(sliceNumber>min)
        {
            emit setSliceNo(sliceNumber - 1);
        }
    }
    else//当滚轮向使用者方向旋转时
    {
        if(sliceNumber < max)
        {
            emit setSliceNo(sliceNumber + 1);
        }

    }
}

int CreateNidus::SetDirectoryName(const char* path)
{
    this->m_SketchNodeXmlPath = QString("%1/xml/SketchNode.xml").arg(path);
    this->m_PatientInfoXmlPath = QString("%1/xml/patient_info.xml").arg(path);
    this->directoryName = QString(path);
    this->reader = vtk::GetDICOMReader(path, 2);

    reader->GetOutput()->GetScalarRange(range);
    reader->GetOutput()->GetExtent(extent);
    reader->GetOutput()->GetSpacing(spacing);
    reader->GetOutput()->GetOrigin(origin);
    vtkDICOMMetaData *meta = reader->GetMetaData();
    bool hasPalette = false;
    if (meta->Get(DC::PhotometricInterpretation).Matches("PALETTE?COLOR") ||
            meta->Get(DC::PixelPresentation).Matches("COLOR") ||
            meta->Get(DC::PixelPresentation).Matches("MIXED") ||
            meta->Get(DC::PixelPresentation).Matches("TRUE_COLOR"))
    {
        hasPalette = true;
        // palette maps stored values, not slope/intercept rescaled values
        reader->AutoRescaleOff();
    }
    double pos[6];
    auto pv = meta->Get(DC::ImageOrientationPatient);
    if (pv.IsValid() && pv.GetNumberOfValues() == 6)
    {
        pv.GetValues(pos, 6);
    }

    auto imageData = vtkSmartPointer<vtkImageData>::New();
    imageData->DeepCopy(reader->GetOutput());
//    vtkSmartPointer<vtkDICOMApplyPalette> palette;
//    if (hasPalette)
//    {
//        palette = vtkSmartPointer<vtkDICOMApplyPalette>::New();
//        palette->SetInputConnection(reader->GetOutputPort());
//        palette->Update();
//        palette->GetOutput()->GetScalarRange(range);
//        portToDisplay = palette->GetOutputPort();
//    }

//    vtkSmartPointer<vtkDICOMCTRectifier> rect;
//    if (meta->Get(DC::Modality).Matches("CT"))
//    {
//        rect = vtkSmartPointer<vtkDICOMCTRectifier>::New();
//        rect->SetVolumeMatrix(reader->GetPatientMatrix());
//        rect->SetInputConnection(portToDisplay);
//        rect->Update();
//        portToDisplay = rect->GetOutputPort();
//    }

    imageReslice->SetInputData(imageData);
    imageReslice->SetOutputDimensionality(2);
    imageReslice->Update();
    imageData= imageReslice->GetOutput();
    imageActor->GetMapper()->SetInputData(imageData);
    imageActor->GetProperty()->SetColorWindow(range[1] - range[0]);
    imageActor->GetProperty()->SetColorLevel(0.5*(range[0] + range[1]));
    imageActor->GetProperty()->SetInterpolationTypeToNearest();
    tracer->SetInputData(imageData);
    renderer->ResetCamera();
    renderWindow->Render();

    min = extent[4];
    max = extent[5];
//    sliceNumber = dimz;
    sliceNumber = 0;
    emit setRange(min, max);
    emit setSliceNo(sliceNumber);
    style->SetCurrentImageNumber(0);
    style->SetWindowLevel(2000, -600);

    // Read XML;
    readSketchNodeXML();

    return max;
}

int CreateNidus::insertSketchNode(QString uid, QString name)
{
    QString path = QString("%1/xml/SketchNode.xml").arg(directoryName);

    QMap<QString,QVariant> attr;
    attr["@FileName"] = name;
    attr["@UID"] = uid;
    QMap<QString,QVariant> slice;
    slice["Slice"] = attr;

    QMap<QString,QVariant> where;
    xml::Append(path, "Sketch", slice, where);//添加点集合
    return EXIT_SUCCESS;
}

int CreateNidus::saveSketchNode(bool savePatientInfoXml)
{
    QString xmlPath = QString("%1/xml/SketchNode.xml").arg(directoryName);
    QString patient_info = QString("%1/xml/patient_info.xml").arg(directoryName);
    QString stlPath = QString("%1/stl").arg(directoryName);

//    QFile::remove(xmlPath);//删除文件 重写文件

    auto imageData = reader->GetOutput();
    auto spacing = imageData->GetSpacing();
    auto origin = imageData->GetOrigin();
    auto dims = imageData->GetDimensions();
    if(0 == sketchNodeMap.size() && true == savePatientInfoXml)
    {
        QMap<QString, QVariant> where;
        where["Category"] = "-1";
        where["Version"] = "Paint_1.0.0";
        xml::Delete(patient_info,"STL/Field",where);
    }

    for(auto it = sketchNodeMap.constBegin(); it != sketchNodeMap.constEnd(); ++it)
    {
        auto sketchNodeList = it.value();
        if(sketchNodeList.size()==0)
        {
            return 0;
        }
        auto name = it.key();
        auto path = QString("%1/%2.stl").arg(stlPath).arg(name);
        vtkNew<vtkPoints> points;
        for (auto i=0; i<sketchNodeList.size(); ++i)
        {
            auto node = sketchNodeList.at(i);
            auto polyData = vtkSmartPointer<vtkPolyData>::New();
            auto sliceNo = node.sliceNo;
            auto count = node.linePoints->GetNumberOfPoints();
            for(auto n=0; n<count; ++n)
            {
                auto pos = node.linePoints->GetPoint(n);
                double point[3] =
                {
                    pos[0],
                    pos[1],
                    (sliceNo-1) * spacing[2],
                };
                points->InsertNextPoint(point);
            }

//            AddSketchNodeXml(node);
        }

        auto res = vtk::saveSketchNode(qPrintable(path),
                                       reader->GetOutput(),
                                       points.Get(),
                                       reader->GetPatientMatrix());
        if(1 == res && true == savePatientInfoXml)
        {
            addPatientXml(patient_info, -1, name);
        }
    }
    return EXIT_SUCCESS;
}

void CreateNidus::Execute(vtkObject* caller, unsigned long eventId, void* clientData, void* callData)
{

}

void CreateNidus::readSketchNodeXML()
{
    sketchNodeMap.clear();
    auto variant = xml::Read(this->m_SketchNodeXmlPath, "Sketch/Slice");
    for(auto v: variant.toList())
    {
        auto Slice = v.toMap();

        auto FileName = Slice["@FileName"].toString();
        if(false == sketchNodeMap.contains(FileName))
        {
            sketchNodeMap[FileName] = SketchNodeList();
        }
        auto PolyLines = Slice["Slice"].toList();
//        if(PolyLines.size()==0)
        {
            //创建顶级节点
            sketchNodeMap[FileName] = SketchNodeList();
            SketchNode node;
            node.index = 1;
            node.sliceNo = 0;
            node.name = FileName;
            node.uid = Slice["@UID"].toString();
            node.modelid = 1;
            this->sketchNodeName = FileName;
            emit addItem(node);
        }

        for(auto i=0; i<PolyLines.size(); ++i)
        {
            auto PolyLine = PolyLines.at(i).toMap();
            SketchNode node;
            node.name = FileName;
            node.sliceNo = PolyLine["@SliceNo"].toInt();
            node.modelid = -1;
            node.uid = PolyLine["@UID"].toString();
            node.index = PolyLine["@Index"].toInt();
            node.linePoints = vtkSmartPointer<vtkPoints>::New();

            auto Points = PolyLine["PolyLine"].toList();
            for(auto j=0; j<Points.size(); ++j)
            {
                auto Point = Points.at(j).toMap();
                double pos[3] ={
                    Point["@X"].toDouble(),
                    Point["@Y"].toDouble(),
                    Point["@Z"].toDouble()};
                node.linePoints->InsertNextPoint(pos);
            }
            node.lineActor = vtk::GetLineActor(node.linePoints);
            node.lineboardText = vtk::getBillBoardActor(node.index,node.linePoints);
            sketchNodeMap[FileName].push_back(node);
            renderer->AddViewProp(node.lineActor);
            renderer->AddViewProp(node.lineboardText);
            renderWindow->Render();
            this->sketchNodeName = FileName;
            emit addItem(node);
        }

    }

}

void CreateNidus::SetEnabledTracer()
{
    tracer->SetEnabled(!tracer->GetEnabled());
}

void CreateNidus::addPatientXml(QString path, int modelid, QString name)
{
    QMap<QString, QString> where;
    where["FileName"] = QString("%1.stl").arg(name);
    where["Version"] = "Paint_1.0.0";

    QMap<QString, QString> attr;
    attr["Category"] = QString::number(modelid);
    attr["FileName"] = QString("%1.stl").arg(name);
    attr["ShowName"] = name;
    attr["Color"] = "#FF0000";
    attr["Opacity"] = "0.45";
    attr["Visible"] = "1";
    attr["Version"] = "Paint_1.0.0";
    xml::updateXml(path, "STL/Field", attr, where);
}

void CreateNidus::SetWindowLevel(int ww, int wl)
{
    QString msg = QString("Window: %1 Level:%2").arg(ww).arg(wl);
    cornerAnnotation->SetText(0, qPrintable(msg));
}

QString CreateNidus::CreateNodeName()
{
    auto index = sketchNodeMap.size() + 1;
    QString Name = QString("结节_%1").arg(index);
    while (true == sketchNodeMap.contains(Name)) {
        Name = QString("结节_%1").arg(index++);
    }
    return Name;
}

QString CreateNidus::CreateUUID()
{
    return QUuid::createUuid().toString().remove("{").remove("}").remove("-");
}

void CreateNidus::AddLine(vtkPoints* linePoints,
                          vtkActor* lineActor,
                          vtkPolyData* lineData,
                          vtkCellArray* lineCells)
{
    QString name = this->sketchNodeName;
    if(true == name.isEmpty())
    {
        name = CreateNodeName();
        sketchNodeMap[name] = SketchNodeList();
        this->sketchNodeName = name;
        SketchNode node;
        node.index = sketchNodeMap[name].size() + 1;
        node.name = name;
        node.sliceNo = 0;
        node.uid = CreateUUID();
        emit addItem(node);//添加顶级节点
        insertSketchNode(node.uid, name);//添加到XML
    }

    SketchNode node;
    node.linePoints = vtkSmartPointer<vtkPoints>::New();
    node.linePoints->DeepCopy(linePoints);
    node.modelid = 1;
    node.uid = CreateUUID();
    node.modeltpye = "Nidus";
    node.lineActor = vtk::GetLineActor(node.linePoints);
    node.sliceNo = this->sliceNumber + 1;
    node.index = sketchNodeMap[name].size() + 1;
    node.name = name;
    node.lineboardText = vtk::getBillBoardActor(node.index,linePoints);
    sketchNodeMap[name].push_back(node);
    renderer->AddViewProp(node.lineActor);
    renderer->AddViewProp(node.lineboardText);
    lineActor->SetVisibility(0);
    renderWindow->Render();
    //    AddSketchNodeXml(node);
    emit addItem(node);//添加子节点
    AddSketchNodeXml(node);
}

int CreateNidus::AddSketchNodeXml(SketchNode node)
{
    QString xmlPath = QString("%1/xml/SketchNode.xml").arg(directoryName);
    QString stlPath = QString("%1/stl/");

    QVariantList points;
    auto numberOfPoints = node.linePoints->GetNumberOfPoints();
    for(auto i=0; i<numberOfPoints; ++i)
    {
        double pos[3] = {0, 0, 0};
        node.linePoints->GetPoint(i, pos);
        QMap<QString,QVariant> point;
        point["@X"] = QString::number(pos[0],'f',3);
        point["@Y"] = QString::number(pos[1],'f',3);
        point["@Z"] = QString::number(pos[2],'f',3);
        points.push_back(point);
    }
    QMap<QString,QVariant> polyLine;
    polyLine["@SliceNo"] = QString::number(node.sliceNo);
    polyLine["@modelid"] = QString::number(-1);
    polyLine["@UID"] = node.uid;
    polyLine["@Index"] = QString::number(node.index);//Index 没有意义
    polyLine["Point"] = points;

    QMap<QString,QVariant> slice;
    slice["@FileName"] = node.name;
    slice["PolyLine"] = polyLine;

    QMap<QString,QVariant> where;
    where["FileName"] = node.name;
    xml::Append(xmlPath, "Sketch/Slice", slice, where);//添加点集合

    return EXIT_SUCCESS;
}

int CreateNidus::DeleteSketchNodeXml(SketchNode node)
{
    return EXIT_SUCCESS;
}

bool CreateNidus::AddSketchNode(QString name)
{
    if(false == sketchNodeMap.contains(name))
    {
        sketchNodeMap[name] = SketchNodeList();
    }
    this->sketchNodeName = name;//切换当前name
    return true;
}

bool CreateNidus::findSketchNode(QString name)
{
    return sketchNodeMap.contains(name);
}

int CreateNidus::getSketchNodeSize()
{
    return sketchNodeMap.size();
}

QString CreateNidus::getSketchNodeName()
{
    return this->sketchNodeName;
}

int CreateNidus::getSketchNodeSliceNo()
{
    return this->sliceNumber + 1;
}

int CreateNidus::deleteSTL(QString name, QString uid)
{
    QString path = QString("%1/xml/patient_info.xml").arg(directoryName);

    QMap<QString,QVariant> where;
    where["ShowName"] = name;
    where["Version"] = "Paint_1.0.0";
    where["Category"] = -1;
    xml::Delete(path, "STL/Field", where);
    return EXIT_SUCCESS;
}

int CreateNidus::deleteSketchChildNode(QString name, QString uid, bool isTopNode)
{
    //当UID为空,删除整个map中的key 当uid不为空 删除SketchNodeList中的一个节点
    if(false == sketchNodeMap.contains(name))
    {
        return EXIT_SUCCESS;
    }
    QMap<QString,QVariant> where;
    QString path = QString("%1/xml/SketchNode.xml").arg(directoryName);
    if(true == isTopNode)
    {
//        where["FileName"] = name;
        where["UID"] = uid;
        qDebug()<<uid;
        xml::Delete(path, "Sketch/Slice", where);

        SketchNodeList& list = sketchNodeMap[name];
        for(auto it = list.begin(); it != list.end(); ++it)
        {
            auto node = (*it);
            renderer->RemoveViewProp(node.lineActor);
            renderer->RemoveViewProp(node.lineboardText);
            renderWindow->Render();
        }

        sketchNodeMap.remove(name);
        if(this->sketchNodeName == name)
        {
            this->sketchNodeName = QString();
        }
        this->deleteSTL(name, uid);//删除STL
    }
    else
    {
        SketchNodeList& list = sketchNodeMap[name];
        for(auto it = list.begin(); it != list.end(); ++it)
        {
            auto node = (*it);
            if(node.uid == uid)
            {
                renderer->RemoveViewProp(node.lineActor);
                renderer->RemoveViewProp(node.lineboardText);
                renderWindow->Render();
                //删除node
                sketchNodeMap[name].erase(it);
                break;
            }
        }
        where["UID"] = uid;
        xml::Delete(path, "Sketch/PolyLine", where);
    }
    return EXIT_SUCCESS;
}

int CreateNidus::updateSketchNodeIndex(QString name, QString uid, int index)
{
    if(false == sketchNodeMap.contains(name))
    {
        EXIT_SUCCESS;
    }

    SketchNodeList& list = sketchNodeMap[name];
    for(auto &node :list)
    {
        if(node.uid == uid)
        {
            node.index = index;
            node.lineboardText->SetInput(std::to_string(index).c_str());
            //            qDebug()<<node.index<<node.name;
        }
    }
    renderWindow->Render();
    return EXIT_SUCCESS;
}
