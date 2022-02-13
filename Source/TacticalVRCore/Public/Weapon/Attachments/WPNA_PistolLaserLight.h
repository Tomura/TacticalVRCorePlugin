// This file is covered by the LICENSE file in the root of this plugin.

#pragma once

#include "CoreMinimal.h"

#include "WPNA_PistolLight.h"
#include "WPNA_PistolLaserLight.generated.h"

UCLASS()
class TACTICALVRCORE_API AWPNA_PistolLaserLight : public AWPNA_PistolLight
{
	GENERATED_BODY()

	UPROPERTY(Category = "Laser", EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess=true))
	class UStaticMeshComponent* LaserBeam;
	
	UPROPERTY(Category = "Laser", EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess=true))
	class UStaticMeshComponent* LaserImpactMesh;
		
protected:
	bool bIsLaserOn;
	
	UPROPERTY(Category = "Laser", EditDefaultsOnly)
	float Spread;
	UPROPERTY(Category = "Laser", EditDefaultsOnly)
	float BaseThickness;
	
public:
	// Sets default values for this actor's properties
	AWPNA_PistolLaserLight(const FObjectInitializer& OI);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	virtual bool IsLaserOn() const {return bIsLaserOn;}
	
	virtual void ToggleLaser(class UGripMotionControllerComponent* UsingHand) override;


};
