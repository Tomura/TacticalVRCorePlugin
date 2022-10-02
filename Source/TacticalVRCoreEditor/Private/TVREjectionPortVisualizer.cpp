#include "TVREjectionPortVisualizer.h"
#include "ActorEditorUtils.h"

FTVREjectionPortVisualizer::FTVREjectionPortVisualizer()
{
    EjectionPortPropertyPath = FComponentPropertyPath();
    TargetViewport = nullptr;
}

 FTVREjectionPortVisualizer::~FTVREjectionPortVisualizer()
{
}

void FTVREjectionPortVisualizer::OnRegister()
{
	FComponentVisualizer::OnRegister();
}

void FTVREjectionPortVisualizer::DrawVisualization(const UActorComponent* Component, const FSceneView* View,
	FPrimitiveDrawInterface* PDI)
{
	if(Component->GetOwner() && FActorEditorUtils::IsAPreviewOrInactiveActor(Component->GetOwner()))
	{
		if(const auto EjectionPort = Cast<UTVREjectionPort>(Component))
		{
			DrawDirectionalArrow(
				PDI,
				EjectionPort->GetEjectionDir().ToMatrixNoScale(),
				FLinearColor::Red,
				7.f,
				1.f,
				100,
				0.25f
			);
			PDI->DrawPoint(EjectionPort->GetEjectionDir().GetLocation(), FLinearColor::White, 10.f, 101);
		}
	}
}

bool FTVREjectionPortVisualizer::VisProxyHandleClick(FEditorViewportClient* InViewportClient,
	HComponentVisProxy* VisProxy, const FViewportClick& Click)
{
	return FComponentVisualizer::VisProxyHandleClick(InViewportClient, VisProxy, Click);
}

void FTVREjectionPortVisualizer::EndEditing()
{
	FComponentVisualizer::EndEditing();
}

bool FTVREjectionPortVisualizer::GetWidgetLocation(const FEditorViewportClient* ViewportClient,
	FVector& OutLocation) const
{
	return FComponentVisualizer::GetWidgetLocation(ViewportClient, OutLocation);
}

bool FTVREjectionPortVisualizer::GetCustomInputCoordinateSystem(const FEditorViewportClient* ViewportClient,
	FMatrix& OutMatrix) const
{
	return FComponentVisualizer::GetCustomInputCoordinateSystem(ViewportClient, OutMatrix);
}

bool FTVREjectionPortVisualizer::HandleInputDelta(FEditorViewportClient* ViewportClient, FViewport* Viewport,
	FVector& DeltaTranslate, FRotator& DeltaRotate, FVector& DeltaScale)
{
	return FComponentVisualizer::HandleInputDelta(ViewportClient, Viewport, DeltaTranslate, DeltaRotate, DeltaScale);
}

TSharedPtr<SWidget> FTVREjectionPortVisualizer::GenerateContextMenu() const
{
	return FComponentVisualizer::GenerateContextMenu();
}

void FTVREjectionPortVisualizer::DrawVisualizationHUD(const UActorComponent* Component,
	const FViewport* Viewport, const FSceneView* View, FCanvas* Canvas)
{
	FComponentVisualizer::DrawVisualizationHUD(Component, Viewport, View, Canvas);
}

bool FTVREjectionPortVisualizer::IsVisualizingArchetype() const
{
    return (EjectionPortPropertyPath.IsValid() && EjectionPortPropertyPath.GetParentOwningActor() && FActorEditorUtils::IsAPreviewOrInactiveActor(EjectionPortPropertyPath.GetParentOwningActor()));
}

UTVREjectionPort* FTVREjectionPortVisualizer::GetEditedMagWell() const
{
	return Cast<UTVREjectionPort>(EjectionPortPropertyPath.GetComponent());
}

const UTVREjectionPort* FTVREjectionPortVisualizer::UpdateSelectedMagWell(HComponentVisProxy* VisProxy)
{
	return nullptr;
}