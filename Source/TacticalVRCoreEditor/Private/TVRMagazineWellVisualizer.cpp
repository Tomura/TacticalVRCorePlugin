#include "TVRMagazineWellVisualizer.h"
#include "ActorEditorUtils.h"

IMPLEMENT_HIT_PROXY(HMagWellHitProxy, HComponentVisProxy);

FTVRMagazineWellVisualizer::FTVRMagazineWellVisualizer()
{
	EditMode = ETVRMagWellEditMode::None;
}

FTVRMagazineWellVisualizer::~FTVRMagazineWellVisualizer()
{
}

void FTVRMagazineWellVisualizer::OnRegister()
{
	FComponentVisualizer::OnRegister();
}

void FTVRMagazineWellVisualizer::DrawVisualization(const UActorComponent* Component, const FSceneView* View,
	FPrimitiveDrawInterface* PDI)
{
	if(Component->GetOwner() && FActorEditorUtils::IsAPreviewOrInactiveActor(Component->GetOwner()))
	{
		if(const auto MagWell = Cast<UTVRMagazineWell>(Component))
		{
			const FVector StartLoc = MagWell->GetComponentLocation();
			const FVector EndLoc = MagWell->GetEjectLocation();
			PDI->DrawLine(StartLoc, EndLoc, FLinearColor::White, 100, 2.f, 0, true);
			PDI->DrawPoint(StartLoc, FLinearColor::White, 10.f, 101);
		
			PDI->SetHitProxy(new HMagWellHitProxy(MagWell, ETVRMagWellEditMode::EjectPoint));
			PDI->DrawPoint(EndLoc, FLinearColor::Red, 10.f, 101);
			PDI->SetHitProxy(nullptr);

			const FTransform BoxWorldTF = MagWell->GetCollisionTransform();
			const FVector Extent = MagWell->GetCollisionExtent();
			const FVector PPP = BoxWorldTF.TransformPositionNoScale(Extent * FVector(1.f, 1.f, 1.f));
			const FVector NPP = BoxWorldTF.TransformPositionNoScale(Extent * FVector(-1.f, 1.f, 1.f));
			const FVector PNP = BoxWorldTF.TransformPositionNoScale(Extent * FVector(1.f, -1.f, 1.f));		
			const FVector PPN = BoxWorldTF.TransformPositionNoScale(Extent * FVector(1.f, 1.f, -1.f));		
			const FVector NNP = BoxWorldTF.TransformPositionNoScale(Extent * FVector(-1.f, -1.f, 1.f));
			const FVector NPN = BoxWorldTF.TransformPositionNoScale(Extent * FVector(-1.f, 1.f, -1.f));
			const FVector PNN = BoxWorldTF.TransformPositionNoScale(Extent * FVector(1.f, -1.f, -1.f));
			const FVector NNN = BoxWorldTF.TransformPositionNoScale(Extent * FVector(-1.f, -1.f, -1.f));
		
			PDI->SetHitProxy(new HMagWellHitProxy(MagWell, ETVRMagWellEditMode::CollisionBox));
			PDI->DrawLine(PPP, NPP, FLinearColor::Green, 100, 0.f, 0, true);
			PDI->DrawLine(PPP, PNP, FLinearColor::Green, 100, 0.f, 0, true);
			PDI->DrawLine(PPP, PPN, FLinearColor::Green, 100, 0.f, 0, true);
		
			PDI->DrawLine(NPP, NNP, FLinearColor::Green, 100, 0.f, 0, true);
			PDI->DrawLine(NPP, NPN, FLinearColor::Green, 100, 0.f, 0, true);
		
			PDI->DrawLine(PNP, PNN, FLinearColor::Green, 100, 0.f, 0, true);
			PDI->DrawLine(PPN, PNN, FLinearColor::Green, 100, 0.f, 0, true);
			PDI->DrawLine(PNP, NNP, FLinearColor::Green, 100, 0.f, 0, true);
			PDI->DrawLine(PPN, NPN, FLinearColor::Green, 100, 0.f, 0, true);
		
			PDI->DrawLine(PNN, NNN, FLinearColor::Green, 100, 0.f, 0, true);
			PDI->DrawLine(NPN, NNN, FLinearColor::Green, 100, 0.f, 0, true);
			PDI->DrawLine(NNP, NNN, FLinearColor::Green, 100, 0.f, 0, true);
			PDI->SetHitProxy(nullptr);
		}
	}
}

bool FTVRMagazineWellVisualizer::VisProxyHandleClick(FEditorViewportClient* InViewportClient,
	HComponentVisProxy* VisProxy, const FViewportClick& Click)
{
	bool bEditing = false;
	if(VisProxy && VisProxy->Component.IsValid())
	{				
		if(VisProxy->IsA(HMagWellHitProxy::StaticGetType()))
		{
			const auto MagWellProxy = static_cast<HMagWellHitProxy*>(VisProxy);
			if(const auto MagWell = UpdateSelectedMagWell(VisProxy))
			{
				bEditing= true;	
				EditMode = MagWellProxy->DesiredEditMode;
				TargetViewport = InViewportClient->Viewport;
			}
		}
	}
	else
	{		
		EditMode = ETVRMagWellEditMode::None;
	}
	return bEditing;
}

void FTVRMagazineWellVisualizer::EndEditing()
{
	MagWellPropertyPath = FComponentPropertyPath();
	EditMode = ETVRMagWellEditMode::None;
	TargetViewport = nullptr;
}

bool FTVRMagazineWellVisualizer::GetWidgetLocation(const FEditorViewportClient* ViewportClient,
	FVector& OutLocation) const
{
	if (TargetViewport == nullptr || TargetViewport != ViewportClient->Viewport)
	{
		return false;
	}
	
	if(const auto MagWell = GetEditedMagWell())
	{
		switch(EditMode)
		{
		case ETVRMagWellEditMode::EjectPoint:
			OutLocation = MagWell->GetEjectLocation();
			return true;
		case ETVRMagWellEditMode::CollisionBox:
			OutLocation = MagWell->GetCollisionTransform().GetLocation();
			return true;
		default:
			break;
		}
	}
	return false;

}

bool FTVRMagazineWellVisualizer::GetCustomInputCoordinateSystem(const FEditorViewportClient* ViewportClient,
	FMatrix& OutMatrix) const
{
	if (TargetViewport == nullptr || TargetViewport != ViewportClient->Viewport)
	{
		return false;
	}
	if(const auto MagWell = GetEditedMagWell())
	{
		switch(EditMode)
		{
		case ETVRMagWellEditMode::EjectPoint:
			OutMatrix = FRotationMatrix(MagWell->GetComponentTransform().Rotator());
			return true;
		case ETVRMagWellEditMode::CollisionBox:
			OutMatrix = FRotationMatrix(MagWell->GetComponentTransform().Rotator());
			return true;
		default:
			break;
		}
	}
	return false;
}

bool FTVRMagazineWellVisualizer::HandleInputDelta(FEditorViewportClient* ViewportClient, FViewport* Viewport,
	FVector& DeltaTranslate, FRotator& DeltaRotate, FVector& DeltaScale)
{
	if (TargetViewport == nullptr || TargetViewport != ViewportClient->Viewport)
	{
		return false;
	}
	if(const auto MagWell = GetEditedMagWell())
	{
		switch(EditMode)
		{
		case ETVRMagWellEditMode::EjectPoint:
			UE_LOG(LogTemp, Log, TEXT("Delta [%s]"), *DeltaTranslate.ToString());
			MagWell->EjectRelativeLoc += MagWell->GetComponentTransform().InverseTransformVector(DeltaTranslate);
			NotifyPropertyModified(
				MagWell,
				FindFProperty<FProperty>(
					UTVRMagazineWell::StaticClass(),
					GET_MEMBER_NAME_CHECKED(UTVRMagazineWell, EjectRelativeLoc)
				)
			);
			return true;
		case ETVRMagWellEditMode::CollisionBox:
			{
				const FTransform OrigTransform = MagWell->GetCollisionTransform();
				const FTransform CompTransform = MagWell->GetComponentTransform();
				const auto NewRot = DeltaRotate.Quaternion() * OrigTransform.GetRotation();
				
				MagWell->CollisionRelativeTransform.SetRotation(
					CompTransform.InverseTransformRotation(NewRot)
				);
				MagWell->CollisionRelativeTransform.AddToTranslation(CompTransform.InverseTransformVector(DeltaTranslate));
				NotifyPropertyModified(
					MagWell,
					FindFProperty<FProperty>(
						UTVRMagazineWell::StaticClass(),
						GET_MEMBER_NAME_CHECKED(UTVRMagazineWell, CollisionRelativeTransform)
					)
				);
			}
			return true;
		default:
			break;
		}
	}	
	return FComponentVisualizer::HandleInputDelta(ViewportClient, Viewport, DeltaTranslate, DeltaRotate, DeltaScale);
}

bool FTVRMagazineWellVisualizer::HandleInputKey(FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key,
	EInputEvent Event)
{
	return FComponentVisualizer::HandleInputKey(ViewportClient, Viewport, Key, Event);
}

TSharedPtr<SWidget> FTVRMagazineWellVisualizer::GenerateContextMenu() const
{
	return FComponentVisualizer::GenerateContextMenu();
}

void FTVRMagazineWellVisualizer::DrawVisualizationHUD(const UActorComponent* Component, const FViewport* Viewport,
	const FSceneView* View, FCanvas* Canvas)
{
	FComponentVisualizer::DrawVisualizationHUD(Component, Viewport, View, Canvas);
}

bool FTVRMagazineWellVisualizer::IsVisualizingArchetype() const
{
	return (
		MagWellPropertyPath.IsValid() &&
		MagWellPropertyPath.GetParentOwningActor() &&
		FActorEditorUtils::IsAPreviewOrInactiveActor(MagWellPropertyPath.GetParentOwningActor())
	);
}

const UTVRMagazineWell* FTVRMagazineWellVisualizer::UpdateSelectedMagWell(HComponentVisProxy* VisProxy)
{
	const auto NewComp = CastChecked<const UTVRMagazineWell>(VisProxy->Component.Get());
	const auto OldHandComp = Cast<UTVRMagazineWell>(MagWellPropertyPath.GetComponent());
	const auto OldOwningActor = MagWellPropertyPath.GetParentOwningActor();
	MagWellPropertyPath = FComponentPropertyPath(NewComp);
	const auto NewOwningActor = MagWellPropertyPath.GetParentOwningActor();

	if (MagWellPropertyPath.IsValid())
	{
		return NewComp;
	}

	MagWellPropertyPath = FComponentPropertyPath();
	return nullptr;
}

UTVRMagazineWell* FTVRMagazineWellVisualizer::GetEditedMagWell() const
{
	return Cast<UTVRMagazineWell>(MagWellPropertyPath.GetComponent());
}
