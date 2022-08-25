// Copyright (c) 2020 Tammo Beil. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/Component/TVRAttachmentPoint.h"
#include "TVRAttachPoint_Barrel.generated.h"

/**
 * 
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), BlueprintType)
class TACTICALVRCORE_API UTVRAttachPoint_Barrel : public UTVRAttachmentPoint
{
	GENERATED_BODY()

public:
	UTVRAttachPoint_Barrel(const FObjectInitializer& OI);
	virtual bool SetCurrentAttachmentClass(TSubclassOf<ATVRWeaponAttachment> NewClass) override;
	virtual TSubclassOf<class ATVRWeaponAttachment> GetCurrentAttachmentClass_Internal() const override;
	virtual void GetAllowedAttachments(TArray<TSubclassOf<class ATVRWeaponAttachment>>& OutAllowedAttachments) const override;

	virtual void OnConstruction() override;
protected:
	UPROPERTY(Category = "Attach Point", EditAnywhere)
	TSubclassOf<class AWPNA_Barrel> CurrentAttachmentClass;
	
	UPROPERTY(Category = "Attach Point", EditDefaultsOnly)
	TArray<TSubclassOf<class AWPNA_Barrel>> AllowedAttachments;
};
