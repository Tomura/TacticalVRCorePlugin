// This file is covered by the LICENSE file in the root of this plugin.

#pragma once

#include "CoreMinimal.h"
#include "TVRAttachmentPoint.h"
#include "TVRAttachPoint_Underbarrel.generated.h"

/**
 * 
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), BlueprintType )
class TACTICALVRCORE_API UTVRAttachPoint_Underbarrel : public UTVRAttachmentPoint
{
	GENERATED_BODY()
	
public:
	UTVRAttachPoint_Underbarrel(const FObjectInitializer& OI);

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;
#endif
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	public:	
	virtual TSubclassOf<class ATVRWeaponAttachment> GetCurrentAttachmentClass() const override;
	virtual void GetAllowedAttachments(TArray<TSubclassOf<class ATVRWeaponAttachment>>& AllowedAttachments) const override;

	protected:
	
	UPROPERTY(Category = "Attach Point", EditAnywhere)
	TSubclassOf<class AWPNA_ForeGrip> CurrentAttachmentClass;
	
	UPROPERTY(Category = "Attach Point", EditDefaultsOnly)
	TArray<TSubclassOf<class AWPNA_ForeGrip>> AllowedAttachments;
};
