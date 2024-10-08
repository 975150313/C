#include "thpnvtk.h"
#include <vtkStripper.h>
#include <vtkStringArray.h>
#include <vtkDICOMReader.h>
#include <vtkCellLocator.h>
#include <vtkContourFilter.h>
#include <vtkCleanPolyData.h>
#include <vtkDICOMDirectory.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderer.h>
#include <vtkProperty.h>
#include <vtkAlgorithmOutput.h>
#include <vtkSTLWriter.h>
#include <vtkPolyDataConnectivityFilter.h>
#include <vtkPointSet.h>
#include <vtkDataSet.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkTriangleFilter.h>

#include <vtkSphere.h>
#include <vtkClipPolyData.h>
#include <vtkExtractVOI.h>
#include <vtkSphereSource.h>
#include <iostream>
#include <ostream>
#include <strstream>
#include <vtkMatrix4x4.h>


#include <vtkImageData.h>
#include <vtkSmartPointer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkImageMapper.h>
#include <vtkActor2D.h>
#include <vtkImageViewer2.h>
#include <vtkImageSliceMapper.h>
#include <vtkImageMapToWindowLevelColors.h>
#include <vtkImageSlice.h>
#include <vtkImageProperty.h>
#include <vtkCornerAnnotation.h>
#include <vtkAssemblyPath.h>
#include <vtkTextProperty.h>
#include <vtkTransformFilter.h>
#include <vtkInteractorStyleImage.h>
#include <vtkDataSet.h>
#include <vtkImageReslice.h>
#include <vtkImageActor.h>
#include <vtkActor.h>
#include <vtkCylinderSource.h>
#include <vtkMath.h>
#include <vtkMinimalStandardRandomSequence.h>
#include <vtkNamedColors.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>
#include <vtkSphereSource.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkDICOMToRAS.h>
#include <vtkMatrix4x4.h>
#include <vtkImageChangeInformation.h>
#include <vtkDICOMCTRectifier.h>
#include <vtkFillHolesFilter.h>
#include <vtkSTLReader.h>
#include <vtkImageReslice.h>
#include <vtkImageResliceMapper.h>
#include <vtkImageSlice.h>
#include <vtkImageSliceMapper.h>
#include <vtkParametricSpline.h>
#include <vtkParametricFunctionSource.h>
#include <vtkBillboardTextActor3D.h>
#include <vtkDICOMMetaData.h>
#include <vtkErrorCode.h>

#include <vtkSmoothPolyDataFilter.h>
#include <vtkDiscreteMarchingCubes.h>
#include <vtkImageStencil.h>
#include <vtkImageTracerWidget.h>
#include <vtkPolyDataToImageStencil.h>

#include <QDebug>

vtkSmartPointer<vtkDICOMReader> vtk::GetDICOMReader(const char* path,const int memoryRowOrder,const int scanDepth, const int dataType)
{
    vtkNew<vtkStringArray> arr;
    vtkNew<vtkDICOMDirectory> directory;
    directory->SetDirectoryName(path);  //UTF-8
    directory->SetScanDepth(scanDepth); //检索文件夹深度
    directory->IgnoreDicomdirOn();      //忽略索引文件
    directory->RequirePixelDataOn();    //忽略没有像素数据的文件
    directory->ShowHiddenOff();         //忽略隐藏文件
    directory->Update();

    int numberOfSeries = directory->GetNumberOfSeries();

    if(0 == numberOfSeries)
    {
        return 0;
    }

    auto fileNames = directory->GetFileNamesForSeries(0);
    auto reader =vtkSmartPointer<vtkDICOMReader>::New();
    reader->SetFileNames(fileNames);
    reader->SortingOn();                        //图像排序
    if(memoryRowOrder>=0 && memoryRowOrder<3)
    {
        reader->SetMemoryRowOrder(memoryRowOrder);//调整图像方向
    }
    reader->SetOutputScalarType(dataType); //设置输出图像像素类型
    reader->UpdateInformation();
    reader->SetDataByteOrderToLittleEndian();
//    reader->SetDataByteOrderToBigEndian();
    reader->Update();
    return reader;
}

vtkSmartPointer<vtkProp3D> vtk::AddActor(vtkRenderer* renderer, vtkDataObject* input)
{
    auto imageSliceMapper = vtkSmartPointer<vtkImageSliceMapper>::New();
    //    imageSliceMapper->SetInputData(reslice1->GetOutput());
    imageSliceMapper->SetInputData(vtkImageData::SafeDownCast(input));
    imageSliceMapper->SetSliceNumber(0);

    auto imageSlice = vtkSmartPointer<vtkImageSlice>::New();
    imageSlice->SetMapper(imageSliceMapper);
    renderer->AddViewProp(imageSlice);
//    renderer->ResetCamera();
//    renderer->Render();
    return imageSlice;
}
void vtk::PrintMatrix(const vtkMatrix4x4* matri,const char* name)
{
    qDebug()<<"--------------------"<<name<<"==nullptr:"<<(matri==nullptr)<<"--";
    if(matri != nullptr)
    {
        qDebug()<<matri->GetElement(0,0)<<matri->GetElement(0,1)<<matri->GetElement(0,2)<<matri->GetElement(0,3);
        qDebug()<<matri->GetElement(1,0)<<matri->GetElement(1,1)<<matri->GetElement(1,2)<<matri->GetElement(1,3);
        qDebug()<<matri->GetElement(2,0)<<matri->GetElement(2,1)<<matri->GetElement(2,2)<<matri->GetElement(2,3);
        qDebug()<<matri->GetElement(3,0)<<matri->GetElement(3,1)<<matri->GetElement(3,2)<<matri->GetElement(3,3);
    }

}



vtkSmartPointer<vtkActor> vtk::GetLineActor(vtkPolyData* polyData)
{
    vtkNew<vtkParametricSpline> spline;
    spline->SetPoints(polyData->GetPoints());

    vtkNew<vtkParametricFunctionSource> source;
    source->SetParametricFunction(spline);
    source->Update();

    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputConnection(source->GetOutputPort());

    auto actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    actor->GetProperty()->SetColor(1, 0, 0);
    actor->SetVisibility(1);
    return actor;
}

vtkSmartPointer<vtkActor> vtk::GetLineActor(vtkPoints* points)
{
    vtkNew<vtkImageTracerWidget> tracer;

    vtkNew<vtkPolyData> polyData;
    tracer->InitializeHandles(points);
    tracer->GetPath(polyData);

    vtkNew<vtkParametricSpline> spline;
    spline->SetPoints(polyData->GetPoints());

    vtkNew<vtkParametricFunctionSource> source;
    source->SetParametricFunction(spline);
    source->Update();

    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputConnection(source->GetOutputPort());

    auto actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    actor->GetProperty()->SetColor(1, 0, 0);
    actor->SetVisibility(1);
    return actor;
}
vtkSmartPointer<vtkBillboardTextActor3D> vtk::getBillBoardActor(const int text, vtkPoints* linePoints)
{
    double pos[3] = {0, 0, 0};
    linePoints->GetPoint(0, pos);
    auto actor = vtkSmartPointer<vtkBillboardTextActor3D>::New();
    actor->GetTextProperty()->SetColor(1, 0, 0);
    actor->GetTextProperty()->SetFontSize(10);
    actor->GetTextProperty()->SetJustificationToCentered();
    actor->SetPosition(pos[0] - 2, pos[1] + 2, pos[2]);
    actor->SetInput(std::to_string(text).c_str());
    return actor;
}
int vtk::saveSketchNode(const char* name,vtkImageData* data,vtkPoints* points, vtkMatrix4x4* matrix)
{
    vtkNew<vtkImageData> imageData;
    imageData->DeepCopy(data);
    auto spacing = imageData->GetSpacing();
    auto origin = imageData->GetOrigin();
    auto dims = imageData->GetDimensions();
    auto polyData = vtkSmartPointer<vtkPolyData>::New();

    auto pdata = static_cast<short*>(imageData->GetScalarPointer());
//    auto pdata = static_cast<float*>(imageData->GetScalarPointer());
    auto numofPix = imageData->GetNumberOfPoints();
    std::fill(pdata, pdata + numofPix, 0);//像素全部重置为0

    vtkNew<vtkImageTracerWidget> imageTracerWidget;
    imageTracerWidget->InitializeHandles(points);
    imageTracerWidget->GetPath(polyData);

    auto pol2stenc = vtkSmartPointer<vtkPolyDataToImageStencil>::New();
    pol2stenc->SetInputData(polyData);
    pol2stenc->SetOutputOrigin(origin);
    pol2stenc->SetOutputSpacing(spacing);
    pol2stenc->SetOutputWholeExtent(imageData->GetExtent());
    pol2stenc->Update();

    auto imgstenc = vtkSmartPointer<vtkImageStencil>::New();
    imgstenc->SetInputData(imageData);
    imgstenc->SetStencilConnection(pol2stenc->GetOutputPort());
    imgstenc->ReverseStencilOn();
    imgstenc->SetBackgroundValue(1);
    imgstenc->Update();

    vtkNew<vtkDiscreteMarchingCubes> contourFilter;
    contourFilter->SetInputData(imgstenc->GetOutput());
    contourFilter->ReleaseDataFlagOn();
    contourFilter->ComputeScalarsOn();
    contourFilter->ComputeGradientsOn();
    contourFilter->ComputeNormalsOn();
    contourFilter->SetValue(0, 1);
    contourFilter->Update();
    auto output = contourFilter->GetOutputPort();

    auto filter = vtkSmartPointer<vtkFillHolesFilter>::New();
    filter->SetInputConnection(output);
    filter->SetHoleSize(10000000.0);
    filter->Update();
    output = filter->GetOutputPort();
    // 网格平滑
    vtkNew<vtkSmoothPolyDataFilter> smoothFilter;
    smoothFilter->SetInputConnection(output);
    smoothFilter->SetNumberOfIterations(25);
    smoothFilter->SetRelaxationFactor(0.1);
    smoothFilter->FeatureEdgeSmoothingOff();
    smoothFilter->BoundarySmoothingOn();
    smoothFilter->Update();
    output = smoothFilter->GetOutputPort();

    vtkNew<vtkTransform> transform;
    transform->SetMatrix(matrix);             //坐标转换矩阵

    vtkNew<vtkTransformPolyDataFilter> transformFilter;
    transformFilter->SetInputConnection(output);
    transformFilter->SetTransform(transform);
    transformFilter->Update();
    output = transformFilter->GetOutputPort();

    vtkNew<vtkSTLWriter> writer;
    writer->SetInputConnection(output);
    writer->SetFileName(name);
    writer->SetFileTypeToBinary();
    return writer->Write();
}
