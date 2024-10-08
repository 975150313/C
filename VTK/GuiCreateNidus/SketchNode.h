#ifndef SKETCHNODE_H
#define SKETCHNODE_H

#include <vtkPolyData.h>
#include <vtkBillboardTextActor3D.h>
#include <vtkPoints.h>
#include <vtkActor.h>
#include <QString>
class SketchNode
{
public:
    int modelid;
    int sliceNo;
    int index;
    QString uid;
    QString name;
    QString modeltpye;
    vtkSmartPointer<vtkPoints> linePoints;
    vtkSmartPointer<vtkActor> lineActor;
    vtkSmartPointer<vtkBillboardTextActor3D> lineboardText;
};
typedef std::vector<SketchNode> SketchNodeList;
#endif // SKETCHNODE_H
