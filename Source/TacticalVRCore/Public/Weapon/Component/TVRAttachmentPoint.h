// This file is covered by the LICENSE file in the root of this plugin.

#pragma once

#include "CoreMinimal.h"
#include "Components/ChildActorComponent.h"
#include "TVRAttachmentPoint.generated.h"

enum class ETVRRailType: uint8;

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
	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
							   FActorComponentTickFunction* ThisTickFunction) override;
	
	// virtual void CreateChildActor() override;
	virtual void CreateChildActor() override;
	
	virtual void OnWeaponAttachmentAttached(class ATVRWeaponAttachment* NewAttachment);
	// void AttachWeaponAttachment()

	UFUNCTION(Category = "WeaponAttachment", BlueprintCallable)
	TSubclassOf<class ATVRWeaponAttachment> GetCurrentAttachmentClass() const;
	
	virtual TSubclassOf<class ATVRWeaponAttachment> GetCurrentAttachmentClass_Internal() const {return nullptr;}
	
	UFUNCTION(Category = "WeaponAttachment", BlueprintCallable)
	virtual void GetAllowedAttachments(TArray<TSubclassOf<class ATVRWeaponAttachment>>& OutAllowedAttachments) const {}

	virtual bool SetCurrentAttachmentClass(TSubclassOf<class ATVRWeaponAttachment> NewClass);
	
	UFUNCTION(Category = "WeaponAttachment", BlueprintCallable)
	class ATVRWeaponAttachment* GetCurrentAttachment() const;


	UFUNCTION(Category = "WeaponAttachment", BlueprintCallable)
	virtual class ATVRGunBase* GetGunOwner() const;
	
	UFUNCTION(Category = "WeaponAttachment", BlueprintCallable)
	void SetPreferredColorVariant(uint8 NewVariant);
	
	UPROPERTY(Category = "WeaponAttachment", EditDefaultsOnly)
	bool bSpawnAttachmentOnBeginPlay;

	uint8 GetRequestedVariant() const;
	uint8 GetRequestedColorVariant() const;
	
	UPROPERTY(Category = "WeaponAttachment", EditAnywhere)
	uint8 PreferredVariant;
	UPROPERTY(Category = "WeaponAttachment", EditAnywhere)
	uint8 ColorVariant;
	
	UPROPERTY(Category = "WeaponAttachment", EditAnywhere)
	bool bOverrideVariant;
	UPROPERTY(Category = "WeaponAttachment", EditAnywhere, meta=(EditCondition="bOverrideVariant"))
	uint8 VariantOverride;
	
	UPROPERTY(Category = "WeaponAttachment", EditAnywhere)
	bool bOverrideColorVariant;
	UPROPERTY(Category = "WeaponAttachment", EditAnywhere, meta=(EditCondition="bOverrideColorVariant"))
	uint8 ColorVariantOverride;
	
	UPROPERTY(Category = "WeaponAttachment", EditAnywhere)
	ETVRRailType RailType;
	UPROPERTY(Category = "WeaponAttachment", EditAnywhere)
	uint8 CustomRailType;

	UPROPERTY(Category = "WeaponAttachment", BlueprintAssignable)
	FOnWeaponAttachmentAttachedDelegate EventOnWeaponAttachmentAttached;

	virtual bool ToggleLight() { return false; }
	virtual bool ToggleLaser() { return false; }
	

protected:
	UPROPERTY()
	class ATVRWeaponAttachment* CachedCurrentAttachment;
};
