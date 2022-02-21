// This file is covered by the LICENSE file in the root of this plugin.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/Component/TVRAttachmentPoint.h"
#include "TVRAttachPoint_Muzzle.generated.h"

/**
 * 
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), BlueprintType)
class TACTICALVRCORE_API UTVRAttachPoint_Muzzle : public UTVRAttachmentPoint
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UTVRAttachPoint_Muzzle(const FObjectInitializer& OI);

	virtual bool SetCurrentAttachmentClass(TSubclassOf<ATVRWeaponAttachment> NewClass) override;

	virtual TSubclassOf<class ATVRWeaponAttachment> GetCurrentAttachmentClass_Internal() const override;
	virtual void GetAllowedAttachments(TArray<TSubclassOf<class ATVRWeaponAttachment>>& OutAllowedAttachments) const override;

protected:
	UPROPERTY(Category = "Attach Point", EditAnywhere)
	TSubclassOf<class AWPNA_Muzzle> CurrentAttachmentClass;
	
	UPROPERTY(Category = "Attach Point", EditDefaultsOnly)
	TArray<TSubclassOf<class AWPNA_Muzzle>> AllowedMuzzles;
};
