// Copyright (c) 2020 Tammo Beil. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/Component/TVRAttachmentPoint.h"
#include "TVRAttachPoint_Stock.generated.h"

/**
 * 
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), BlueprintType)
class TACTICALVRCORE_API UTVRAttachPoint_Stock : public UTVRAttachmentPoint
{
	GENERATED_BODY()
public:
	// Sets default values for this component's properties
	UTVRAttachPoint_Stock(const FObjectInitializer& OI);

	virtual TSubclassOf<class ATVRWeaponAttachment> GetCurrentAttachmentClass() const override;
	virtual void GetAllowedAttachments(TArray<TSubclassOf<class ATVRWeaponAttachment>>& OutAllowedAttachments) const override;
	
protected:
	UPROPERTY(Category = "Attach Point", EditAnywhere)
	TSubclassOf<class AWPNA_Stock> CurrentAttachmentClass;
	
	UPROPERTY(Category = "Attach Point", EditDefaultsOnly)
	TArray<TSubclassOf<class AWPNA_Stock>> AllowedAttachments;
};
