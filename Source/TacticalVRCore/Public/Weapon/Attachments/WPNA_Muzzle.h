// This file is covered by the LICENSE file in the root of this plugin.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/Attachments/TVRWeaponAttachment.h"
#include "WPNA_Muzzle.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class TACTICALVRCORE_API AWPNA_Muzzle : public ATVRWeaponAttachment
{
	GENERATED_BODY()
	
	UPROPERTY(Category = "Laser", EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess=true))
	class USceneComponent* MuzzleLocation;

public:
	AWPNA_Muzzle(const FObjectInitializer& OI);
	
	virtual void AttachToWeapon(UTVRAttachmentPoint* AttachPoint) override;
	virtual void BeginPlay() override;
	virtual void Destroyed() override;

	virtual bool IsSuppressor() const { return bIsSuppressor; }

	virtual void ModifyMuzzleLocation();

protected:
	UPROPERTY(Category = "Muzzle", EditDefaultsOnly)
	bool bIsSuppressor;
	
	UPROPERTY(Category = "Muzzle", EditDefaultsOnly)
	float SoundModifier;
	
	UPROPERTY(Category = "Muzzle", EditDefaultsOnly)
	float DamageModifier;
	
	UPROPERTY(Category = "Muzzle", EditDefaultsOnly)
	float RecoilModifier;

	UPROPERTY(Category = "Muzzle", EditDefaultsOnly)
	UParticleSystemComponent* MuzzleFlashOverride;
	
};
