// This file is covered by the LICENSE file in the root of this plugin.

#pragma once

#include "CoreMinimal.h"
#include "TVRAttachmentPoint.h"
#include "TVRAttachPoint_Laser.generated.h"

/**
 * 
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), BlueprintType )
class TACTICALVRCORE_API UTVRAttachPoint_Laser : public UTVRAttachmentPoint
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UTVRAttachPoint_Laser(const FObjectInitializer& OI);

	virtual bool SetCurrentAttachmentClass(TSubclassOf<ATVRWeaponAttachment> NewClass) override;
	virtual TSubclassOf<class ATVRWeaponAttachment> GetCurrentAttachmentClass_Internal() const override;
	virtual void GetAllowedAttachments(TArray<TSubclassOf<class ATVRWeaponAttachment>>& OutAllowedAttachments) const override;
	virtual bool ToggleLaser() override;

	UFUNCTION(Category = "Attach Point", BlueprintCallable)
	virtual bool  SetCurrentLaser(TSubclassOf<class AWPNA_Laser> NewAttachment, bool bForce = false);

protected:	
	UPROPERTY(Category = "Attach Point", EditAnywhere)
	TSubclassOf<class AWPNA_Laser> CurrentLaserClass;
	
	UPROPERTY(Category = "Attach Point", EditDefaultsOnly)
	TArray<TSubclassOf<class AWPNA_Laser>> AllowedLasers;
};
