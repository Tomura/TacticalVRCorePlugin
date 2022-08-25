// This file is covered by the LICENSE file in the root of this plugin.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/TVRCartridge.h"
#include "TVRSpentCartridge.generated.h"

/**
 * 
 */
UCLASS()
class TACTICALVRCORE_API ATVRSpentCartridge : public ATVRCartridge
{
	GENERATED_BODY()

public:
	ATVRSpentCartridge(const FObjectInitializer& OI);

	virtual void SetLifeSpan(float InLifespan) override;

	void Deactivate();
	
	void Activate();
	
	bool IsActive() const {return bActive;}

	
	virtual void ApplySpentCartridgeSettings(TSubclassOf<ATVRCartridge> TemplateType);
	
protected:
	virtual void BeginPlay() override;

protected:
	bool bActive;
	float Lifespan;
	FTimerHandle LifespanTimer;
};
