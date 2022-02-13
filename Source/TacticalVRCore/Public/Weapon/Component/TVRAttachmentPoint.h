// This file is covered by the LICENSE file in the root of this plugin.

#pragma once

#include "CoreMinimal.h"
#include "Components/ChildActorComponent.h"
#include "TVRAttachmentPoint.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeaponAttachmentAttachedDelegate, class UTVRAttachmentPoint*, AttachmentPoint, class ATVRWeaponAttachment*, NewWeaponAttachment);

UCLASS(Abstract, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), HideCategories=(ChildActorComponent) )
class TACTICALVRCORE_API UTVRAttachmentPoint : public UChildActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UTVRAttachmentPoint(const FObjectInitializer& OI);
	
	virtual void BeginPlay() override;
	virtual void OnRegister() override;


	virtual void OnConstruction();
	
	virtual void OnWeaponAttachmentAttached(class ATVRWeaponAttachment* NewAttachment);
	// void AttachWeaponAttachment()

	UFUNCTION(Category = "WeaponAttachment", BlueprintCallable)
	virtual TSubclassOf<class ATVRWeaponAttachment> GetCurrentAttachmentClass() const {return nullptr;}
	
	UFUNCTION(Category = "WeaponAttachment", BlueprintCallable)
	virtual void GetAllowedAttachments(TArray<TSubclassOf<class ATVRWeaponAttachment>>& OutAllowedAttachments) const {}

	UFUNCTION(Category = "WeaponAttachment", BlueprintCallable)
	FORCEINLINE class ATVRWeaponAttachment* GetCurrentAttachment() const  {return CurrentAttachment;}
public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	virtual class ATVRGunBase* GetGunOwner() const;
	
	UPROPERTY(Category = "WeaponAttachment", EditDefaultsOnly)
	bool bSpawnAttachmentOnBeginPlay;

	UPROPERTY(Category = "WeaponAttachment", BlueprintAssignable)
	FOnWeaponAttachmentAttachedDelegate EventOnWeaponAttachmentAttached;

	virtual bool ToggleLight() { return false; }
	virtual bool ToggleLaser() { return false; }
	

protected:
	UPROPERTY()
	class ATVRWeaponAttachment* CurrentAttachment;
};
