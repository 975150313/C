/*=========================================================================

Program:   Visualization Toolkit
Module:    vtkImageTracerWidgetNew.cxx

Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
All rights reserved.
See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkImageTracerWidgetNew.h"

#include "vtkAbstractPicker.h"
#include "vtkActor.h"
#include "vtkAssemblyPath.h"
#include "vtkAssemblyNode.h"
#include "vtkCallbackCommand.h"
#include "vtkCamera.h"
#include "vtkCellArray.h"
#include "vtkCellPicker.h"
#include "vtkFloatArray.h"
#include "vtkGlyphSource2D.h"
#include "vtkImageData.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkPickingManager.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkPolyLine.h"
#include "vtkProperty.h"
#include "vtkPropPicker.h"
#include "vtkRenderer.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkTransform.h"

#include "vtkTextProperty.h"
#include "vtkParametricSpline.h"
#include "vtkParametricFunctionSource.h"

#include "createnidus.h"

#include <QDebug>

vtkStandardNewMacro(vtkImageTracerWidgetNew);

vtkCxxSetObjectMacro(vtkImageTracerWidgetNew, HandleProperty, vtkProperty);
vtkCxxSetObjectMacro(vtkImageTracerWidgetNew, SelectedHandleProperty, vtkProperty);
vtkCxxSetObjectMacro(vtkImageTracerWidgetNew, LineProperty, vtkProperty);
vtkCxxSetObjectMacro(vtkImageTracerWidgetNew, SelectedLineProperty, vtkProperty);

vtkImageTracerWidgetNew::vtkImageTracerWidgetNew()
{
    this->HandleLeftMouseButton = true;
    this->HandleMiddleMouseButton = true;
    this->HandleRightMouseButton = true;

    this->State = vtkImageTracerWidgetNew::Start;
    this->EventCallbackCommand->SetCallback(vtkImageTracerWidgetNew::ProcessEvents);

    this->Interaction = 1;
    this->ViewProp = nullptr;
    this->PickCount = 0;
    this->SnapToImage = 0;
    this->AutoClose = 0;
    this->CaptureRadius = 1.0;
    this->IsSnapping = 0;
    this->ImageSnapType = VTK_ITW_SNAP_CELLS;
    this->CurrentPicker = nullptr;
    this->CurrentHandle = nullptr;
    this->CurrentHandleIndex = -1;
    this->ProjectionNormal = VTK_ITW_PROJECTION_XY;
    this->ProjectionPosition = 0.0;
    this->ProjectToPlane = 0;
    this->NumberOfHandles = 0;
    this->LastX = 0;
    this->LastY = 0;

    this->PropPicker = vtkPropPicker::New();
    this->PropPicker->PickFromListOn();

    // Build the representation of the widget
    this->HandleGenerator = vtkGlyphSource2D::New();
    this->HandleGenerator->SetGlyphTypeToCross();
    this->HandleGenerator->FilledOff();
    this->HandleGenerator->SetCenter(0, 0, 0);

    this->TransformFilter = vtkTransformPolyDataFilter::New();
    this->Transform = vtkTransform::New();
    this->TransformFilter->SetTransform(this->Transform);
    this->Transform->Identity();
    this->TransformFilter->SetInputConnection(
        this->HandleGenerator->GetOutputPort());
    this->TransformFilter->Update();

    this->TemporaryHandlePoints = vtkFloatArray::New();
    this->TemporaryHandlePoints->SetNumberOfComponents(3);

    this->LinePoints = vtkPoints::New();
    this->LinePoints->Allocate(1001);
    this->LineCells = vtkCellArray::New();
    this->LineCells->Allocate(this->LineCells->EstimateSize(1000, 2));
    this->LineActor = vtkActor::New();
    vtkPolyDataMapper* lineMapper = vtkPolyDataMapper::New();
    this->LineData = vtkPolyData::New();

    lineMapper->SetInputData(this->LineData);
    lineMapper->SetResolveCoincidentTopologyToPolygonOffset();
    lineMapper->ScalarVisibilityOff();
    this->LineActor->SetMapper(lineMapper);
    this->LineActor->PickableOff();
    this->LineActor->VisibilityOff();
    lineMapper->Delete();

    // Manage the picking stuff
    this->HandlePicker = vtkCellPicker::New();
    this->HandlePicker->SetTolerance(0.005);
    this->HandlePicker->PickFromListOn();

    this->LinePicker = vtkCellPicker::New();
    this->LinePicker->SetTolerance(0.005);
    this->LinePicker->PickFromListOn();

    // Set up the initial properties
    this->HandleProperty = nullptr;
    this->SelectedHandleProperty = nullptr;
    this->LineProperty = nullptr;
    this->SelectedLineProperty = nullptr;
    this->CreateDefaultProperties();

    // Initialize ivars
    this->Handle = nullptr;
    this->HandleGeometry = nullptr;

    // Create one handle
    this->AllocateHandles(1);
    this->AdjustHandlePosition(0, this->HandleGenerator->GetCenter());

    // Initial creation of the widget, serves to initialize it
    // using default bounds to get started
    double bounds[6];
    vtkMath::UninitializeBounds(bounds);

    this->PlaceFactor = 1.0;
    this->PlaceWidget(bounds);
}

vtkImageTracerWidgetNew::~vtkImageTracerWidgetNew()
{
    for (int i = 0; i < this->NumberOfHandles; ++i)
    {
        this->HandleGeometry[i]->Delete();
        this->Handle[i]->Delete();
    }
    delete[] this->Handle;
    this->Handle = nullptr;

    delete[] this->HandleGeometry;
    this->HandleGeometry = nullptr;

    if (this->HandleProperty)
    {
        this->HandleProperty->Delete();
    }
    if (this->SelectedHandleProperty)
    {
        this->SelectedHandleProperty->Delete();
    }
    if (this->LineProperty)
    {
        this->LineProperty->Delete();
    }
    if (this->SelectedLineProperty)
    {
        this->SelectedLineProperty->Delete();
    }
    if (this->ViewProp)
    {
        this->ViewProp->UnRegister(this);
    }

    this->LinePoints->Delete();
    this->LineCells->Delete();
    this->LineActor->Delete();
    this->LineData->Delete();

    this->LinePicker->Delete();
    this->HandlePicker->Delete();
    this->CurrentPicker = nullptr;
    this->CurrentHandle = nullptr;

    this->PropPicker->Delete();
    this->TransformFilter->Delete();
    this->Transform->Delete();
    this->TemporaryHandlePoints->Delete();
    this->HandleGenerator->Delete();
}

//----------------------------------------------------------------------------
void vtkImageTracerWidgetNew::SetViewProp(vtkProp* prop)
{
    if (this->ViewProp != prop)
    {
        // Avoid destructor recursion
        vtkProp *temp = this->ViewProp;
        this->ViewProp = prop;
        if (temp)
        {
            temp->UnRegister(this);
        }
        if (this->ViewProp)
        {
            this->ViewProp->Register(this);
            this->PropPicker->InitializePickList();
            this->PropPicker->AddPickList(this->ViewProp);
        }
    }
}

//------------------------------------------------------------------------------
void vtkImageTracerWidgetNew::RegisterPickers()
{
    vtkPickingManager* pm = this->GetPickingManager();
    if (!pm)
    {
        return;
    }
    pm->AddPicker(this->PropPicker, this);
    pm->AddPicker(this->HandlePicker, this);
    pm->AddPicker(this->LinePicker, this);
}

void vtkImageTracerWidgetNew::SetEnabled(int enabling)
{
    if (!this->Interactor)
    {
        vtkErrorMacro(<< "The interactor must be set prior to enabling/disabling widget");
        return;
    }

    if (!this->ViewProp)
    {
        vtkErrorMacro(<< "The external prop must be set prior to enabling/disabling widget");
        return;
    }

    if (enabling)
    {
        vtkDebugMacro(<< "Enabling line widget");

        if (this->Enabled) //already enabled, just return
        {
            return;
        }

        if (!this->CurrentRenderer)
        {
            this->SetCurrentRenderer(this->Interactor->FindPokedRenderer(
                this->Interactor->GetLastEventPosition()[0],
                this->Interactor->GetLastEventPosition()[1]));
            if (!this->CurrentRenderer)
            {
                return;
            }
        }

        this->Enabled = 1;

        this->AddObservers();

        // Turn on the handles
        for (int i = 0; i < this->NumberOfHandles; ++i)
        {
            this->CurrentRenderer->AddViewProp(this->Handle[i]);
            this->Handle[i]->SetProperty(this->HandleProperty);
            this->Handle[i]->PickableOff();
        }

        this->SizeHandles();

        this->CurrentRenderer->AddViewProp(this->LineActor);
        this->LineActor->SetProperty(this->LineProperty);
        this->LineActor->PickableOff();
        this->RegisterPickers();

        this->InvokeEvent(vtkCommand::EnableEvent, nullptr);
    }

    else // disabling
    {
        vtkDebugMacro(<< "Disabling tracer widget");

        if (!this->Enabled) //already disabled, just return
        {
            return;
        }

        // if disabling occurs without finishing an activity, cleanup states
        if (this->State == vtkImageTracerWidgetNew::Tracing)
        {
            this->OnLeftButtonUp();
        }
        else if (this->State == vtkImageTracerWidgetNew::Snapping)
        {
            this->Interactor->SetControlKey(1);
            this->OnMiddleButtonUp();
        }

        this->Enabled = 0;

        // Don't listen for events any more
        this->Interactor->RemoveObserver(this->EventCallbackCommand);

        // Turn off the handles
        for (int i = 0; i < this->NumberOfHandles; ++i)
        {
            this->CurrentRenderer->RemoveViewProp(this->Handle[i]);
        }

        this->CurrentRenderer->RemoveViewProp(this->LineActor);

        this->CurrentHandle = nullptr;
        this->InvokeEvent(vtkCommand::DisableEvent, nullptr);
        this->SetCurrentRenderer(nullptr);
        this->UnRegisterPickers();
    }

    this->Interactor->Render();
}

void vtkImageTracerWidgetNew::ProcessEvents(vtkObject* vtkNotUsed(object),
    unsigned long event,
    void* clientdata,
    void* vtkNotUsed(calldata))
{
    vtkImageTracerWidgetNew* self = reinterpret_cast<vtkImageTracerWidgetNew *>(clientdata);

    switch (event)
    {
    case vtkCommand::LeftButtonPressEvent:
        self->OnLeftButtonDown();
        break;
    case vtkCommand::LeftButtonReleaseEvent:
        self->OnLeftButtonUp();
        break;
    case vtkCommand::MiddleButtonPressEvent:
        self->OnMiddleButtonDown();
        break;
    case vtkCommand::MiddleButtonReleaseEvent:
        self->OnMiddleButtonUp();
        break;
    case vtkCommand::RightButtonPressEvent:
        self->OnRightButtonDown();
        break;
    case vtkCommand::RightButtonReleaseEvent:
        self->OnRightButtonUp();
        break;
    case vtkCommand::MouseMoveEvent:
        self->OnMouseMove();
        break;
    case vtkCommand::MouseWheelForwardEvent:
        self->OnMouseWheelForward();
        break;
    case vtkCommand::MouseWheelBackwardEvent:
        self->OnMouseWheelBackward();
        break;
    }
}

void vtkImageTracerWidgetNew::AddObservers(void)
{
    // Listen for the following events
    vtkRenderWindowInteractor *i = this->Interactor;
    if (!i)
    {
        return;
    }

    i->AddObserver(vtkCommand::MouseMoveEvent, this->EventCallbackCommand,
        this->Priority);
    if (this->HandleLeftMouseButton)
    {
        i->AddObserver(vtkCommand::LeftButtonPressEvent,
            this->EventCallbackCommand, this->Priority);
        i->AddObserver(vtkCommand::LeftButtonReleaseEvent,
            this->EventCallbackCommand, this->Priority);
    }
    if (this->HandleMiddleMouseButton)
    {
        i->AddObserver(vtkCommand::MiddleButtonPressEvent,
            this->EventCallbackCommand, this->Priority);
        i->AddObserver(vtkCommand::MiddleButtonReleaseEvent,
            this->EventCallbackCommand, this->Priority);
    }
    if (this->HandleRightMouseButton)
    {
        i->AddObserver(vtkCommand::RightButtonPressEvent,
            this->EventCallbackCommand, this->Priority);
        i->AddObserver(vtkCommand::RightButtonReleaseEvent,
            this->EventCallbackCommand, this->Priority);
    }
}

void vtkImageTracerWidgetNew::SetInteraction(vtkTypeBool interact)
{
    if (this->Interactor && this->Enabled)
    {
        if (this->Interaction == interact)
        {
            return;
        }
        if (interact == 0)
        {
            this->Interactor->RemoveObserver(this->EventCallbackCommand);
        }
        else
        {
            this->AddObservers();
        }
        this->Interaction = interact;
    }
    else
    {
        vtkGenericWarningMacro(<< "Set interactor and Enabled before changing interaction...");
    }
}

void vtkImageTracerWidgetNew::PrintSelf(ostream& os, vtkIndent indent)
{
    this->Superclass::PrintSelf(os, indent);

    if (this->HandleProperty)
    {
        os << indent << "Handle Property: " << this->HandleProperty << "\n";
    }
    else
    {
        os << indent << "Handle Property: (none)\n";
    }

    if (this->SelectedHandleProperty)
    {
        os << indent << "Selected Handle Property: "
            << this->SelectedHandleProperty << "\n";
    }
    else
    {
        os << indent << "Selected Handle Property: (none)\n";
    }

    if (this->LineProperty)
    {
        os << indent << "Line Property: " << this->LineProperty << "\n";
    }
    else
    {
        os << indent << "Line Property: (none)\n";
    }

    if (this->SelectedLineProperty)
    {
        os << indent << "Selected Line Property: "
            << this->SelectedLineProperty << "\n";
    }
    else
    {
        os << indent << "Selected Line Property: (none)\n";
    }

    if (this->ViewProp)
    {
        os << indent << "ViewProp: " << this->ViewProp << "\n";
    }
    else
    {
        os << indent << "ViewProp: (none)\n";
    }

    os << indent << "Interaction: "
        << (this->Interaction ? "On\n" : "Off\n");
    os << indent << "ProjectionNormal: " << this->ProjectionNormal << "\n";
    os << indent << "ProjectionPosition: " << this->ProjectionPosition << "\n";
    os << indent << "ProjectToPlane: "
        << (this->ProjectToPlane ? "On\n" : "Off\n");
    os << indent << "ImageSnapType: " << this->ImageSnapType << "\n";
    os << indent << "SnapToImage: "
        << (this->SnapToImage ? "On\n" : "Off\n");
    os << indent << "CaptureRadius: " << this->CaptureRadius << "\n";
    os << indent << "NumberOfHandles: " << this->NumberOfHandles << "\n";
    os << indent << "HandleLeftMouseButton: " << this->HandleLeftMouseButton << "\n";
    os << indent << "HandleMiddleMouseButton: " << this->HandleMiddleMouseButton << "\n";
    os << indent << "HandleRightMouseButton: " << this->HandleRightMouseButton << "\n";
    os << indent << "AutoClose: "
        << (this->AutoClose ? "On\n" : "Off\n");
}

int vtkImageTracerWidgetNew::HighlightHandle(vtkProp* prop)
{
    // First unhighlight anything picked
    if (this->CurrentHandle)
    {
        this->CurrentHandle->SetProperty(this->HandleProperty);
        this->Interactor->Render();
    }

    this->CurrentHandle = static_cast<vtkActor *>(prop);

    if (this->CurrentHandle)
    {
        this->ValidPick = 1;
        this->CurrentPicker->GetPickPosition(this->LastPickPosition);
        this->CurrentHandle->SetProperty(this->SelectedHandleProperty);
        for (int i = 0; i < this->NumberOfHandles; ++i) // find handle
        {
            if (this->CurrentHandle == this->Handle[i])
            {
                return i;
            }
        }
    }
    return -1;
}

void vtkImageTracerWidgetNew::HighlightLine(const int& highlight)
{
    if (highlight)
    {
        this->ValidPick = 1;
        this->CurrentPicker->GetPickPosition(this->LastPickPosition);
        this->LineActor->SetProperty(this->SelectedLineProperty);
    }
    else
    {
        this->LineActor->SetProperty(this->LineProperty);
    }
}

void vtkImageTracerWidgetNew::AdjustHandlePosition(const int& handle, double pos[3])
{
    if (handle < 0 || handle >= this->NumberOfHandles) { return; }

    if (this->ProjectToPlane)
    {
        pos[this->ProjectionNormal] = this->ProjectionPosition;
    }

    this->HandleGenerator->SetCenter(0.0, 0.0, 0.0);
    this->Transform->Identity();
    this->Transform->PostMultiply();

    if (this->ProjectionNormal == VTK_ITW_PROJECTION_YZ)
    {
        this->Transform->RotateY(90.0);
    }
    else if (this->ProjectionNormal == VTK_ITW_PROJECTION_XZ)
    {
        this->Transform->RotateX(90.0);
    }

    this->Transform->Translate(pos);
    this->TransformFilter->Update();

    this->HandleGeometry[handle]->CopyStructure(this->TransformFilter->GetOutput());
    this->HandleGeometry[handle]->Modified();
}

void vtkImageTracerWidgetNew::SetProjectionPosition(double position)
{
    this->ProjectionPosition = position;

    int i;
    for (i = 0; i < this->NumberOfHandles; ++i)
    {
        this->AdjustHandlePosition(i, this->HandleGeometry[i]->GetCenter());
    }

    double pt[3];
    for (i = 0; i < this->NumberOfHandles; ++i)
    {
        this->LinePoints->GetPoint(i, pt);
        pt[this->ProjectionNormal] = this->ProjectionPosition;
        this->LinePoints->SetPoint(i, pt);
    }

    this->LinePoints->GetData()->Modified();
    this->LineData->Modified();
}

void vtkImageTracerWidgetNew::SetHandlePosition(int handle, double xyz[3])
{
    this->AdjustHandlePosition(handle, xyz);
}

void vtkImageTracerWidgetNew::SetHandlePosition(int handle, double x, double y, double z)
{
    double xyz[3] = { x,y,z };
    this->AdjustHandlePosition(handle, xyz);
}

void vtkImageTracerWidgetNew::GetHandlePosition(int handle, double xyz[3])
{
    if (handle < 0 || handle >= this->NumberOfHandles) { return; }
    this->HandleGeometry[handle]->GetCenter(xyz);
}

double* vtkImageTracerWidgetNew::GetHandlePosition(int handle)
{
    if (handle < 0 || handle >= this->NumberOfHandles) { return nullptr; }
    return this->HandleGeometry[handle]->GetCenter();
}

void vtkImageTracerWidgetNew::OnLeftButtonDown()
{
//    qDebug()<<"OnLeftButtonDown";
    // If the user is snap defining a line by middle mouse button,
    // ignore this button
    if (this->State == vtkImageTracerWidgetNew::Snapping) { return; }

    int X = this->Interactor->GetEventPosition()[0];
    int Y = this->Interactor->GetEventPosition()[1];

    // Okay, make sure that the pick is in the current renderer
    if (!this->CurrentRenderer || !this->CurrentRenderer->IsInViewport(X, Y))
    {
        this->State = vtkImageTracerWidgetNew::Outside;
        return;
    }

    int found = 0;
    if (this->PropPicker->PickProp(X, Y, this->CurrentRenderer))
    {
        if (this->ViewProp == this->PropPicker->GetViewProp())
        {
            found = 1;
            this->State = vtkImageTracerWidgetNew::Tracing;
        }
    }

    if (!found)
    {
        this->State = vtkImageTracerWidgetNew::Outside;
        return;
    }

    // first erase any handles if there any
    if (this->NumberOfHandles > 1)
    {
        this->AllocateHandles(1);
    }

    this->CurrentPicker = this->PropPicker;  //collect the pick position from the prop picker
    this->CurrentHandleIndex = this->HighlightHandle(this->Handle[0]);

    if (this->CurrentHandleIndex == -1)    //this should never happen
    {
        this->State = vtkImageTracerWidgetNew::Outside;
        return;
    }

    // set the handle to the picked position
    this->AdjustHandlePosition(this->CurrentHandleIndex, this->LastPickPosition);

    // erase the line and initialize it
    this->ResetLine(this->LastPickPosition);

    this->LastX = X;
    this->LastY = Y;

    this->EventCallbackCommand->SetAbortFlag(1);
    this->StartInteraction();
    this->InvokeEvent(vtkCommand::StartInteractionEvent, nullptr);
    this->Interactor->Render();
}

void vtkImageTracerWidgetNew::OnLeftButtonUp()
{
    if (this->State == vtkImageTracerWidgetNew::Outside ||
        this->State == vtkImageTracerWidgetNew::Start ||
        this->State == vtkImageTracerWidgetNew::Snapping)
    {
        return;
    }

    this->State = vtkImageTracerWidgetNew::Start;
    this->CurrentHandleIndex = this->HighlightHandle(nullptr);

    if (this->AutoClose)  // attempt to close by tolerance
    {
        this->ClosePath();
        if (this->IsClosed()) // if successful, remove the overlapping handle
        {
            this->EraseHandle(this->NumberOfHandles - 1);
        }
    }

    this->SizeHandles();

    this->EventCallbackCommand->SetAbortFlag(1);
    this->EndInteraction();
    this->InvokeEvent(vtkCommand::EndInteractionEvent, nullptr);
    this->Interactor->Render();
    this->CurrentPicker = nullptr;


    if(nullptr != this->reateNidus)
    {
        int npts = this->LinePoints->GetNumberOfPoints();
        if (npts < 4) { return; }
        double p0[3];
        this->LinePoints->GetPoint(0, p0);
        double p1[3];
        this->LinePoints->GetPoint(npts - 1, p1);
//        qDebug()<<"POS "<<p1[0]<<p1[1]<<p1[2]<<this->ProjectionPosition;
        this->reateNidus->AddLine(this->LinePoints,
                                  this->LineActor,
                                  this->LineData,
                                  this->LineCells);
    }
}

void vtkImageTracerWidgetNew::OnMiddleButtonDown()
{
    int X = this->Interactor->GetEventPosition()[0];
    int Y = this->Interactor->GetEventPosition()[1];

    if (!this->CurrentRenderer || !this->CurrentRenderer->IsInViewport(X, Y))
    {
        this->State = vtkImageTracerWidgetNew::Outside;
        return;
    }

    int found = 0;
    if (this->PropPicker->PickProp(X, Y, this->CurrentRenderer))
    {
        if (this->ViewProp == this->PropPicker->GetViewProp())
        {
            found = 1;
            this->State = vtkImageTracerWidgetNew::Snapping; // do snap tracing
        }
    }

    if (!found)
    {
        this->State = vtkImageTracerWidgetNew::Outside;
        return;
    }

    if (!this->IsSnapping)  // this is the first time so reset the handles
    {
        if (this->NumberOfHandles > 1)
        {
            this->AllocateHandles(1);
        }
    }

    this->CurrentPicker = this->PropPicker;         // highlight the last handle
    this->CurrentHandleIndex = this->HighlightHandle(this->Handle[this->NumberOfHandles - 1]);

    if (this->CurrentHandleIndex == -1)  // sanity check: this should never happen
    {
        this->State = vtkImageTracerWidgetNew::Outside;
        return;
    }

    this->AdjustHandlePosition(this->CurrentHandleIndex, this->LastPickPosition);

    if (!this->IsSnapping)  // this is the first time so initialize the line
    {
        this->ResetLine(this->GetHandlePosition(this->CurrentHandleIndex));
    }

    this->IsSnapping = this->NumberOfHandles;

    this->EventCallbackCommand->SetAbortFlag(1);
    this->StartInteraction();
    this->InvokeEvent(vtkCommand::StartInteractionEvent, nullptr);
    this->Interactor->Render();
}

void vtkImageTracerWidgetNew::OnMiddleButtonUp()
{
    if (this->State == vtkImageTracerWidgetNew::Outside ||
        this->State == vtkImageTracerWidgetNew::Start)
    {
        return;
    }

    if (this->Interactor->GetControlKey()) // finished the snapping
    {
        this->IsSnapping = 0;
    }
    else // continue snap drawing
    {
        return;
    }

    this->State = vtkImageTracerWidgetNew::Start;
    this->CurrentHandleIndex = this->HighlightHandle(nullptr);

    if (this->AutoClose)
    {
        this->ClosePath();
        if (this->IsClosed()) // if successful, remove the last overlapping handle
        {
            this->EraseHandle(this->NumberOfHandles - 1);
        }
    }

    this->SizeHandles();

    this->EventCallbackCommand->SetAbortFlag(1);
    this->EndInteraction();
    this->InvokeEvent(vtkCommand::EndInteractionEvent, nullptr);
    this->Interactor->Render();
    this->CurrentPicker = nullptr;
}

void vtkImageTracerWidgetNew::OnRightButtonDown()
{
    if (this->State == vtkImageTracerWidgetNew::Snapping) { return; }

    int X = this->Interactor->GetEventPosition()[0];
    int Y = this->Interactor->GetEventPosition()[1];

    if (!this->CurrentRenderer || !this->CurrentRenderer->IsInViewport(X, Y))
    {
        this->State = vtkImageTracerWidgetNew::Outside;
        return;
    }

    if (this->Interactor->GetControlKey() && (this->NumberOfHandles > 1))
    {
        this->State = vtkImageTracerWidgetNew::Erasing; // pick a handle to delete
        for (int i = 0; i < this->NumberOfHandles; ++i)
        {
            this->Handle[i]->PickableOn();
        }
        this->CurrentPicker = this->HandlePicker;
    }
    else if (this->Interactor->GetShiftKey() && (this->NumberOfHandles > 1))
    {
        this->State = vtkImageTracerWidgetNew::Inserting; // pick a line to insert on
        this->LineActor->PickableOn();
        this->LinePicker->AddPickList(this->LineActor);
        this->CurrentPicker = this->LinePicker;
    }
    else
    {
        if (this->NumberOfHandles < 3 && this->LinePoints->GetNumberOfPoints() > this->NumberOfHandles)
        {
            this->State = vtkImageTracerWidgetNew::Translating;
        }
        else
        {
            this->State = vtkImageTracerWidgetNew::Moving;
        }
        for (int i = 0; i < this->NumberOfHandles; ++i)
        {
            this->Handle[i]->PickableOn();
        }
        this->CurrentPicker = this->HandlePicker;
    }

    if (this->ViewProp)  // don't pick the prop
    {
        this->ViewProp->PickableOff();
    }

    int found = 0;
    vtkAssemblyPath* path = this->GetAssemblyPath(X, Y, 0., this->CurrentPicker);

    if (path)
    {
        found = 1;
        if (this->State == vtkImageTracerWidgetNew::Erasing ||
            this->State == vtkImageTracerWidgetNew::Moving ||
            this->State == vtkImageTracerWidgetNew::Translating)
        {
            this->CurrentHandleIndex = this->HighlightHandle(path->GetFirstNode()->GetViewProp());
            if (this->CurrentHandleIndex == -1)
            {
                found = 0;  // we didn't hit a handle
                for (int i = 0; i < this->NumberOfHandles; ++i)
                {
                    this->Handle[i]->PickableOff();
                }
            }
        }
        else if (this->State == vtkImageTracerWidgetNew::Inserting)
        {
            if (static_cast<vtkActor*>(path->GetFirstNode()->GetViewProp()) == this->LineActor)
            {
                this->HighlightLine(1);
            }
            else
            {
                found = 0;
                this->LineActor->PickableOff();
            }
        }
    }

    if (!found)
    {
        this->State = vtkImageTracerWidgetNew::Outside;
        if (this->ViewProp)
        {
            this->ViewProp->PickableOn();
        }
        this->CurrentPicker = nullptr;
        return;
    }

    this->EventCallbackCommand->SetAbortFlag(1);
    this->StartInteraction();
    this->InvokeEvent(vtkCommand::StartInteractionEvent, nullptr);
    this->Interactor->Render();
}

void vtkImageTracerWidgetNew::OnRightButtonUp()
{
    if (this->State == vtkImageTracerWidgetNew::Outside ||
        this->State == vtkImageTracerWidgetNew::Start ||
        this->State == vtkImageTracerWidgetNew::Snapping)
    {
        return;
    }

    if (this->State == vtkImageTracerWidgetNew::Erasing)
    {
        int index = this->CurrentHandleIndex;
        this->CurrentHandleIndex = this->HighlightHandle(nullptr);
        int closed = this->IsClosed();
        this->EraseHandle(index);
        this->BuildLinesFromHandles();
        if (closed && this->NumberOfHandles > 2)
        {
            this->AppendLine(this->HandleGeometry[0]->GetCenter());
        }
    }
    else if (this->State == vtkImageTracerWidgetNew::Inserting)
    {
        this->HighlightLine(0);
        int closed = this->IsClosed();
        this->InsertHandleOnLine(this->LastPickPosition);
        this->BuildLinesFromHandles();
        if (closed)
        {
            this->AppendLine(this->HandleGeometry[0]->GetCenter());
        }
    }
    else if (this->State == vtkImageTracerWidgetNew::Moving)
    {
        this->CurrentHandleIndex = this->HighlightHandle(nullptr);
        if (this->AutoClose && !this->IsClosed())
        {
            this->ClosePath();
            if (this->IsClosed()) // if successful, remove the last overlapping handle
            {
                this->EraseHandle(this->NumberOfHandles - 1);
            }
        }
    }
    else if (this->State == vtkImageTracerWidgetNew::Translating)
    {
        this->CurrentHandleIndex = this->HighlightHandle(nullptr);
    }

    this->State = vtkImageTracerWidgetNew::Start;

    this->SizeHandles();

    if (this->ViewProp)
    {
        this->ViewProp->PickableOn();
    }

    this->EventCallbackCommand->SetAbortFlag(1);
    this->EndInteraction();
    this->InvokeEvent(vtkCommand::EndInteractionEvent, nullptr);
    this->Interactor->Render();
    this->CurrentPicker = nullptr;
}

void vtkImageTracerWidgetNew::OnMouseMove()
{
    // See whether we're active
    if (this->State == vtkImageTracerWidgetNew::Outside ||
        this->State == vtkImageTracerWidgetNew::Start)
    {
        return;
    }

    int X = this->Interactor->GetEventPosition()[0];
    int Y = this->Interactor->GetEventPosition()[1];
    double z;

    // Process the motion
    if (this->CurrentHandle)
    {
        if (this->State == vtkImageTracerWidgetNew::Tracing ||
            this->State == vtkImageTracerWidgetNew::Snapping)
        {
            this->Trace(X, Y);
        }
        else if (this->State == vtkImageTracerWidgetNew::Moving ||
            this->State == vtkImageTracerWidgetNew::Translating)
        {
            double focalPoint[4], pickPoint[4], prevPickPoint[4];

            vtkCamera *camera = this->CurrentRenderer->GetActiveCamera();
            if (!camera) { return; }

            // Compute the two points defining the motion vector
            this->ComputeWorldToDisplay(this->LastPickPosition[0],
                this->LastPickPosition[1],
                this->LastPickPosition[2], focalPoint);
            z = focalPoint[2];
            this->ComputeDisplayToWorld(
                double(this->Interactor->GetLastEventPosition()[0]),
                double(this->Interactor->GetLastEventPosition()[1]),
                z, prevPickPoint);
            this->ComputeDisplayToWorld(double(X), double(Y), z, pickPoint);

            if (this->State == vtkImageTracerWidgetNew::Moving)
            {
                this->MovePoint(prevPickPoint, pickPoint);
            }
            else
            {
                this->Translate(prevPickPoint, pickPoint);
            }
        }
    }

    // Interact, if desired
    this->EventCallbackCommand->SetAbortFlag(1);
    this->InvokeEvent(vtkCommand::InteractionEvent, nullptr);
    this->Interactor->Render();
}

void vtkImageTracerWidgetNew::OnMouseWheelForward()
{
    qDebug()<<"OnMouseWheelForward";

}

void vtkImageTracerWidgetNew::OnMouseWheelBackward()
{
    //==========================SHOW==========================
//    qDebug()<<"POS "<<pos[0]<<pos[1]<<pos[2]<<this->ProjectionPosition;
//    if(this->ProjectionPosition == pos[2])
//    {
//        this->LineActor->SetVisibility(1);
//    }
//    else
//    {
//        this->LineActor->SetVisibility(0);
//    }
}

void vtkImageTracerWidgetNew::Trace(int X, int Y)
{
    if (!this->PropPicker->PickProp(X, Y, this->CurrentRenderer)) { return; }
    if (this->ViewProp != this->PropPicker->GetViewProp()) { return; }

    double pos[3];
    this->PropPicker->GetPickPosition(pos);
//    qDebug()<<pos[0]<<pos[1]<<pos[2];
    if (this->SnapToImage)
    {
        this->Snap(pos);
    }

    if (this->ProjectToPlane)
    {
        pos[this->ProjectionNormal] = this->ProjectionPosition;
    }
//    qDebug()<<pos[0]<<pos[1]<<pos[2];
    if (this->LastX != X || this->LastY != Y)
    {
        if (this->State == vtkImageTracerWidgetNew::Tracing)
        {
            if (this->NumberOfHandles == 1)
            {
                this->AppendHandles(pos);
            }
            else
            {
                this->AdjustHandlePosition(this->CurrentHandleIndex, pos);
            }
            this->AppendLine(pos);
        }
        else if (this->State == vtkImageTracerWidgetNew::Snapping)
        {
            if (this->IsSnapping != this->CurrentHandleIndex)
            {
                this->AppendHandles(pos);
                this->AppendLine(pos);
                this->IsSnapping = this->CurrentHandleIndex;
            }
            else
            {
                this->AdjustHandlePosition(this->CurrentHandleIndex, pos);
                this->LinePoints->SetPoint(this->PickCount, pos);
                this->LinePoints->GetData()->Modified();
                this->LineData->Modified();
            }
        }
    }

    this->LastX = X;
    this->LastY = Y;
}

void vtkImageTracerWidgetNew::MovePoint(const double *p1, const double *p2)
{
    // Get the motion vector
    double v[3];
    v[0] = p2[0] - p1[0];
    v[1] = p2[1] - p1[1];
    v[2] = p2[2] - p1[2];

    double *ctr = this->HandleGeometry[this->CurrentHandleIndex]->GetCenter();

    double newCtr[3];
    newCtr[0] = ctr[0] + v[0];
    newCtr[1] = ctr[1] + v[1];
    newCtr[2] = ctr[2] + v[2];

    // Move the widget handle
    this->AdjustHandlePosition(this->CurrentHandleIndex, newCtr);

    // Enforce consistency with the line
    int closed = this->IsClosed();

    this->LinePoints->SetPoint(this->CurrentHandleIndex,
        this->HandleGeometry[this->CurrentHandleIndex]->GetCenter());

    // Special case when moving the first point
    if (closed && (this->CurrentHandleIndex == 0))
    {
        this->LinePoints->SetPoint(this->LinePoints->GetNumberOfPoints() - 1,
            this->HandleGeometry[0]->GetCenter());
    }

    this->LinePoints->GetData()->Modified();
    this->LineData->Modified();
}

void vtkImageTracerWidgetNew::Translate(const double *p1, const double *p2)
{
    // Get the motion vector
    double v[3];
    v[0] = p2[0] - p1[0];
    v[1] = p2[1] - p1[1];
    v[2] = p2[2] - p1[2];

    double newCtr[3];
    int i;
    for (i = 0; i < this->NumberOfHandles; ++i)
    {
        double *ctr = this->HandleGeometry[i]->GetCenter();
        newCtr[0] = ctr[0] + v[0];
        newCtr[1] = ctr[1] + v[1];
        newCtr[2] = ctr[2] + v[2];
        this->AdjustHandlePosition(i, newCtr);
    }

    for (i = 0; i < this->LinePoints->GetNumberOfPoints(); ++i)
    {
        double *ctr = this->LinePoints->GetPoint(i);
        newCtr[0] = ctr[0] + v[0];
        newCtr[1] = ctr[1] + v[1];
        newCtr[2] = ctr[2] + v[2];
        if (this->ProjectToPlane)
        {
            newCtr[this->ProjectionNormal] = this->ProjectionPosition;
        }
        this->LinePoints->SetPoint(i, newCtr);
    }

    this->LinePoints->GetData()->Modified();
    this->LineData->Modified();
}

void vtkImageTracerWidgetNew::ResetHandles(void)
{
    if (this->NumberOfHandles == 0) { return; }

    if (this->CurrentHandle)
    {
        this->CurrentHandle = nullptr;
    }

    this->HandlePicker->InitializePickList();

    int i;
    if (this->CurrentRenderer)
    {
        for (i = 0; i < this->NumberOfHandles; ++i)
        {
            this->CurrentRenderer->RemoveViewProp(this->Handle[i]);
        }
    }

    for (i = 0; i < this->NumberOfHandles; ++i)
    {
        this->HandleGeometry[i]->Delete();
        this->Handle[i]->Delete();
    }

    this->NumberOfHandles = 0;

    delete[] this->Handle;
    this->Handle = nullptr;

    delete[] this->HandleGeometry;
    this->HandleGeometry = nullptr;
}

void vtkImageTracerWidgetNew::AllocateHandles(const int& nhandles)
{
    if ((this->NumberOfHandles == nhandles) || (nhandles < 1)) { return; }

    // De-allocate the handles
    this->ResetHandles();
    this->NumberOfHandles = nhandles;

    // Create the handles
    this->Handle = new vtkActor*[this->NumberOfHandles];
    this->HandleGeometry = new vtkPolyData*[this->NumberOfHandles];

    int i;
    for (i = 0; i < this->NumberOfHandles; ++i)
    {
        this->HandleGeometry[i] = vtkPolyData::New();
        vtkPolyDataMapper* handleMapper = vtkPolyDataMapper::New();
        handleMapper->SetInputData(this->HandleGeometry[i]);
        this->Handle[i] = vtkActor::New();
        this->Handle[i]->SetMapper(handleMapper);
        handleMapper->Delete();
        this->Handle[i]->SetProperty(this->HandleProperty);
        this->Handle[i]->PickableOff();
        this->HandlePicker->AddPickList(this->Handle[i]);
    }

    if (this->CurrentRenderer && this->Enabled)
    {
        for (i = 0; i < this->NumberOfHandles; ++i)
        {
            this->CurrentRenderer->AddViewProp(this->Handle[i]);
        }
    }
}

void vtkImageTracerWidgetNew::AppendHandles(double* pos)
{
    this->TemporaryHandlePoints->Reset();
    this->TemporaryHandlePoints->SetNumberOfTuples(this->NumberOfHandles + 1);
    int i;
    for (i = 0; i < this->NumberOfHandles; ++i)
    {
        this->TemporaryHandlePoints->SetTuple(i, this->HandleGeometry[i]->GetCenter());
    }

    this->TemporaryHandlePoints->SetTuple(this->NumberOfHandles, pos);

    this->AllocateHandles(this->TemporaryHandlePoints->GetNumberOfTuples());

    for (i = 0; i < this->NumberOfHandles; ++i)
    {
        this->AdjustHandlePosition(i, this->TemporaryHandlePoints->GetTuple(i));
    }

    if (this->CurrentHandleIndex != -1)
    {
        this->CurrentHandleIndex = this->NumberOfHandles - 1;
        this->CurrentHandle = this->Handle[this->CurrentHandleIndex];
        this->CurrentHandle->SetProperty(this->SelectedHandleProperty);
    }
}

void vtkImageTracerWidgetNew::InsertHandleOnLine(double* pos)
{
    if (this->NumberOfHandles < 3 &&
        this->LinePoints->GetNumberOfPoints() > 2)
    {
        return; // don't insert on a continuously traced line
    }

    int id = this->LinePicker->GetCellId();
    if (id == -1) { return; }

    this->TemporaryHandlePoints->Reset();
    this->TemporaryHandlePoints->SetNumberOfTuples(this->NumberOfHandles + 1);
    int i;
    for (i = 0; i <= id; i++)
    {
        this->TemporaryHandlePoints->SetTuple(i, this->HandleGeometry[i]->GetCenter());
    }

    this->TemporaryHandlePoints->SetTuple(id + 1, pos);

    for (i = id + 1; i < this->NumberOfHandles; ++i)
    {
        this->TemporaryHandlePoints->SetTuple(i + 1, this->HandleGeometry[i]->GetCenter());
    }

    this->AllocateHandles(this->TemporaryHandlePoints->GetNumberOfTuples());

    for (i = 0; i < this->NumberOfHandles; ++i)
    {
        this->AdjustHandlePosition(i, this->TemporaryHandlePoints->GetTuple(i));
    }
}

void vtkImageTracerWidgetNew::InitializeHandles(vtkPoints* points)
{
    if (!points) { return; }

    int npts = points->GetNumberOfPoints();
    if (npts == 0) { return; }

    this->AllocateHandles(npts);

    for (int i = 0; i < npts; ++i)
    {
        this->AdjustHandlePosition(i, points->GetPoint(i));
    }

    if (npts > 1)
    {
        this->BuildLinesFromHandles();
        if (this->AutoClose)
        {
            this->ClosePath();
            if (this->IsClosed()) // if successful, remove the overlapping handle
            {
                this->EraseHandle(this->NumberOfHandles - 1);
            }
        }
    }
}

void vtkImageTracerWidgetNew::EraseHandle(const int& index)
{
    if (this->NumberOfHandles == 1) { return; }

    this->TemporaryHandlePoints->Reset();
    this->TemporaryHandlePoints->SetNumberOfTuples(this->NumberOfHandles - 1);
    int i;
    int count = 0;
    for (i = 0; i < this->NumberOfHandles; ++i)
    {
        if (i != index)
        {
            this->TemporaryHandlePoints->SetTuple(count++, this->HandleGeometry[i]->GetCenter());
        }
    }

    this->AllocateHandles(this->TemporaryHandlePoints->GetNumberOfTuples());

    for (i = 0; i < this->NumberOfHandles; ++i)
    {
        this->AdjustHandlePosition(i, this->TemporaryHandlePoints->GetTuple(i));
    }
}

void vtkImageTracerWidgetNew::ResetLine(double* pos)
{
    this->LinePicker->DeletePickList(this->LineActor);
    this->LineActor->VisibilityOff();
    this->LineActor->PickableOff();

    this->LinePoints->Delete();
    this->LineCells->Delete();

    this->LineData->Initialize();
    this->LineData->Squeeze();

    this->LinePoints = vtkPoints::New();
    this->LineCells = vtkCellArray::New();

    this->LineData->SetPoints(this->LinePoints);
    this->LineData->SetLines(this->LineCells);

    this->PickCount = 0;

    this->LinePoints->InsertPoint(this->PickCount, pos);
}

void vtkImageTracerWidgetNew::CleanLine()
{
    this->LinePicker->DeletePickList(this->LineActor);
    this->LineActor->VisibilityOff();
    this->LineActor->PickableOff();

    this->LinePoints->Delete();
    this->LineCells->Delete();

    this->LineData->Initialize();
    this->LineData->Squeeze();

    this->LinePoints = vtkPoints::New();
    this->LineCells = vtkCellArray::New();

    this->LineData->SetPoints(this->LinePoints);
    this->LineData->SetLines(this->LineCells);

    this->PickCount = 0;
    //this->ResetLine(this->HandleGeometry[0]->GetCenter());
    //this->LinePoints->InsertPoint(this->PickCount, this->LastPickPosition);
}

void vtkImageTracerWidgetNew::AppendLine(double* pos)
{
    this->CurrentPoints[0] = this->PickCount++;
    this->CurrentPoints[1] = this->PickCount;

    this->LinePoints->InsertPoint(this->PickCount, pos);
    this->LineCells->InsertNextCell(2, this->CurrentPoints);
    this->LineCells->Modified();

    this->LinePoints->GetData()->Modified();
    this->LineData->SetPoints(this->LinePoints);
    this->LineData->SetLines(this->LineCells);
    this->LineData->Modified();

    this->LineActor->SetVisibility(1);
}

void vtkImageTracerWidgetNew::BuildLinesFromHandles()
{
    this->ResetLine(this->HandleGeometry[0]->GetCenter());

    for (int i = 1; i < this->NumberOfHandles; ++i)
    {
        this->AppendLine(this->HandleGeometry[i]->GetCenter());
    }
}

void vtkImageTracerWidgetNew::ClosePath()
{

    int npts = this->LinePoints->GetNumberOfPoints();
    if (npts < 4) { return; }

    double p0[3];
    this->LinePoints->GetPoint(0, p0);
    double p1[3];
    this->LinePoints->GetPoint(npts - 1, p1);

    if (sqrt(vtkMath::Distance2BetweenPoints(p0, p1)) <= this->CaptureRadius)
    {
        this->LinePoints->SetPoint(npts - 1, p0);
        this->LinePoints->GetData()->Modified();
        this->LineData->Modified();
    }
}

int vtkImageTracerWidgetNew::IsClosed() // can only be based on line data
{
    int npts = this->LinePoints->GetNumberOfPoints();
    if (npts < 4) { return 0; }

    double p0[3];
    this->LinePoints->GetPoint(0, p0);
    double p1[3];
    this->LinePoints->GetPoint(npts - 1, p1);

    return (p0[0] == p1[0] && p0[1] == p1[1] && p0[2] == p1[2]);
}

void vtkImageTracerWidgetNew::GetPath(vtkPolyData *pd)
{
    pd->ShallowCopy(this->LineData);
}

void vtkImageTracerWidgetNew::SetSnapToImage(vtkTypeBool snap)
{
    if (this->GetInput())
    {
        if (this->GetInput()->GetDataObjectType() != VTK_IMAGE_DATA)
        {
            vtkErrorMacro(<< "Input data must be of type vtkImageData");
            return;
        }
        else
        {
            this->SnapToImage = snap;
        }
    }
    else
    {
        vtkGenericWarningMacro(<< "SetInput with type vtkImageData first");
        return;
    }
}

void vtkImageTracerWidgetNew::Snap(double* pos) // overwrites pos
{
    vtkImageData* ptr = vtkImageData::SafeDownCast(this->GetInput());
    if (!ptr) { return; }

    if (this->ImageSnapType == VTK_ITW_SNAP_CELLS)  // snap to cell center
    {
        double bounds[6];
        double weights[8];
        double pcoords[3];
        int subId;
        vtkIdType cellId = ptr->FindCell(pos, nullptr, -1, 0.0, subId, pcoords, weights);
        if (cellId != -1)
        {
            ptr->GetCellBounds(cellId, bounds);
            for (int i = 0; i < 3; ++i)
            {
                pos[i] = bounds[i * 2] + 0.5*(bounds[i * 2 + 1] - bounds[i * 2]);
            }
        }
    }
    else // snap to nearest point
    {
        vtkIdType ptId = ptr->FindPoint(pos);
        if (ptId != -1)
        {
            ptr->GetPoint(ptId, pos);
        }
    }
}

void vtkImageTracerWidgetNew::CreateDefaultProperties()
{
    if (!this->HandleProperty)
    {
        this->HandleProperty = vtkProperty::New();
        this->HandleProperty->SetAmbient(1.0);
        this->HandleProperty->SetDiffuse(0.0);
        this->HandleProperty->SetColor(1, 0, 1);
        this->HandleProperty->SetLineWidth(2);
        this->HandleProperty->SetRepresentationToWireframe();
        this->HandleProperty->SetInterpolationToFlat();
    }
    if (!this->SelectedHandleProperty)
    {
        this->SelectedHandleProperty = vtkProperty::New();
        this->SelectedHandleProperty->SetAmbient(1.0);
        this->SelectedHandleProperty->SetDiffuse(0.0);
        this->SelectedHandleProperty->SetColor(0, 1, 0);
        this->SelectedHandleProperty->SetLineWidth(2);
        this->SelectedHandleProperty->SetRepresentationToWireframe();
        this->SelectedHandleProperty->SetInterpolationToFlat();
    }
    if (!this->LineProperty)
    {
        this->LineProperty = vtkProperty::New();
        this->LineProperty->SetAmbient(1.0);
        this->LineProperty->SetDiffuse(0.0);
        this->LineProperty->SetColor(0, 1, 0);
        this->LineProperty->SetLineWidth(2);
        this->LineProperty->SetRepresentationToWireframe();
        this->LineProperty->SetInterpolationToFlat();
    }
    if (!this->SelectedLineProperty)
    {
        this->SelectedLineProperty = vtkProperty::New();
        this->SelectedLineProperty->SetAmbient(1.0);
        this->SelectedLineProperty->SetDiffuse(0.0);
        this->SelectedLineProperty->SetColor(0, 1, 1);
        this->SelectedLineProperty->SetLineWidth(2);
        this->SelectedLineProperty->SetRepresentationToWireframe();
        this->SelectedLineProperty->SetInterpolationToFlat();
    }
}

void vtkImageTracerWidgetNew::PlaceWidget(double bds[6])
{
    double bounds[6], center[3];
    this->AdjustBounds(bds, bounds, center);

    // create a default handle within the data bounds
    double x0 = bounds[0];
    double x1 = bounds[1];
    double y0 = bounds[2];
    double y1 = bounds[3];
    double z0 = bounds[4];
    double z1 = bounds[5];
    double xyz[3];
    double position = 0.5;
    xyz[0] = (1.0 - position)*x0 + position*x1;
    xyz[1] = (1.0 - position)*y0 + position*y1;
    xyz[2] = (1.0 - position)*z0 + position*z1;

    this->AdjustHandlePosition(0, xyz);

    for (int i = 0; i < 6; ++i)
    {
        this->InitialBounds[i] = bounds[i];
    }
    this->InitialLength = sqrt((bounds[1] - bounds[0])*(bounds[1] - bounds[0]) +
        (bounds[3] - bounds[2])*(bounds[3] - bounds[2]) +
        (bounds[5] - bounds[4])*(bounds[5] - bounds[4]));
    this->SizeHandles();
}

void vtkImageTracerWidgetNew::SizeHandles()
{
    // TODO...
}

void vtkImageTracerWidgetNew::SetOpenGLWidget(CreateNidus* widget)
{
    this->reateNidus = widget;
}

void vtkImageTracerWidgetNew::SetLineNumber(int number)
{
    this->LineNumber = number;
}
