#pragma once
#include "ComponentVisualizer.h"
#include "Weapon/Component/TVRMagazineWell.h"

enum class ETVRMagWellEditMode : uint8
{
	None,
	EjectPoint,
	CollisionBox
};


struct HMagWellHitProxy : public HComponentVisProxy {
	DECLARE_HIT_PROXY();

	HMagWellHitProxy(const UActorComponent* InComp, ETVRMagWellEditMode Mode) : HComponentVisProxy(InComp, HPP_Wireframe)
	{
		DesiredEditMode = Mode;
	}

	ETVRMagWellEditMode DesiredEditMode;
};


class FTVRMagazineWellVisualizer : public FComponentVisualizer
{
public:
	FTVRMagazineWellVisualizer();
	virtual ~FTVRMagazineWellVisualizer();

	/** Makes a new instance of this detail layout class for a specific detail view requesting it */
	static TSharedRef<FTVRMagazineWellVisualizer> MakeInstance()
	{
		return MakeShareable(new FTVRMagazineWellVisualizer());
	}

	// Begin FComponentVisualizer interface
	virtual void OnRegister() override;
	virtual void DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI) override;
	virtual bool VisProxyHandleClick(FEditorViewportClient* InViewportClient, HComponentVisProxy* VisProxy, const FViewportClick& Click) override;
	virtual void EndEditing() override;
	virtual bool GetWidgetLocation(const FEditorViewportClient* ViewportClient, FVector& OutLocation) const override; 
	virtual bool GetCustomInputCoordinateSystem(const FEditorViewportClient* ViewportClient, FMatrix& OutMatrix) const override;
	virtual bool HandleInputDelta(FEditorViewportClient* ViewportClient, FViewport* Viewport, FVector& DeltaTranslate, FRotator& DeltaRotate, FVector& DeltaScale) override;
	virtual bool HandleInputKey(FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event) override;
	virtual TSharedPtr<SWidget> GenerateContextMenu() const override;
	virtual void DrawVisualizationHUD(const UActorComponent* Component, const FViewport* Viewport, const FSceneView* View, FCanvas* Canvas) override;
	virtual bool IsVisualizingArchetype() const override;
	// End FComponentVisualizer interface
	
	/** Get the target component we are currently editing */
	UTVRMagazineWell* GetEditedMagWell() const;

	const class UTVRMagazineWell* UpdateSelectedMagWell(HComponentVisProxy* VisProxy);
	
	UPROPERTY()
	FComponentPropertyPath MagWellPropertyPath;

	UPROPERTY()
	FViewport* TargetViewport;
	
	private:
	ETVRMagWellEditMode EditMode;
};
