#ifndef THPNVTK_H
#define THPNVTK_H
#include <vtkPolyData.h>
#include <vtkImageData.h>
#include <vtkSmartPointer.h>
#include <vtkRenderer.h>
#include <vtkActor.h>
#include <vtkAlgorithmOutput.h>
#include <vtkDICOMReader.h>
#include <vtkProp3D.h>
#include <vtkBillboardTextActor3D.h>
class vtkMatrix4x4;
class vtkPoints;
namespace vtk
{
vtkSmartPointer<vtkDICOMReader> GetDICOMReader(const char* path,
                                               const int memoryRowOrder=2,
                                               const int scanDepth=4,
                                               const int dataType=VTK_SHORT);
vtkSmartPointer<vtkProp3D> AddActor(vtkRenderer* renderer, vtkDataObject* input);
void PrintMatrix(const vtkMatrix4x4* matri,const char* name);

vtkSmartPointer<vtkActor> GetLineActor(vtkPolyData* polyData);
vtkSmartPointer<vtkActor> GetLineActor(vtkPoints* points);
vtkSmartPointer<vtkBillboardTextActor3D> getBillBoardActor(const int text, vtkPoints* linePoints);

int saveSketchNode(const char* name,vtkImageData* imageData,vtkPoints* points, vtkMatrix4x4* matrix);
}



#endif // THPNVTK_H
