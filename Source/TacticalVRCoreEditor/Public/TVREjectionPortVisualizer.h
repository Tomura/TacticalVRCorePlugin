#pragma once
#include "ComponentVisualizer.h"
#include "Weapon/Component/TVREjectionPort.h"

class FTVREjectionPortVisualizer : public FComponentVisualizer
{
public:
	FTVREjectionPortVisualizer();
	virtual ~FTVREjectionPortVisualizer();

	/** Makes a new instance of this detail layout class for a specific detail view requesting it */
	static TSharedRef<FTVREjectionPortVisualizer> MakeInstance()
	{
		return MakeShareable(new FTVREjectionPortVisualizer());
	}

	// Begin FComponentVisualizer interface
	virtual void OnRegister() override;
	virtual void DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI) override;
	virtual bool VisProxyHandleClick(FEditorViewportClient* InViewportClient, HComponentVisProxy* VisProxy, const FViewportClick& Click) override;
	virtual void EndEditing() override;
	virtual bool GetWidgetLocation(const FEditorViewportClient* ViewportClient, FVector& OutLocation) const override; 
	virtual bool GetCustomInputCoordinateSystem(const FEditorViewportClient* ViewportClient, FMatrix& OutMatrix) const override;
	virtual bool HandleInputDelta(FEditorViewportClient* ViewportClient, FViewport* Viewport, FVector& DeltaTranslate, FRotator& DeltaRotate, FVector& DeltaScale) override;
	virtual TSharedPtr<SWidget> GenerateContextMenu() const override;
	virtual void DrawVisualizationHUD(const UActorComponent* Component, const FViewport* Viewport, const FSceneView* View, FCanvas* Canvas) override;
	virtual bool IsVisualizingArchetype() const override;
	// End FComponentVisualizer interface
	
	/** Get the target component we are currently editing */
	UTVREjectionPort* GetEditedMagWell() const;

	const class UTVREjectionPort* UpdateSelectedMagWell(HComponentVisProxy* VisProxy);
	
	UPROPERTY()
	FComponentPropertyPath EjectionPortPropertyPath;

	UPROPERTY()
	FViewport* TargetViewport;
};

