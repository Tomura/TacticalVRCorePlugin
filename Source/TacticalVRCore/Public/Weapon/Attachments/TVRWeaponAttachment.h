// This file is covered by the LICENSE file in the root of this plugin.

#pragma once

#include "CoreMinimal.h"
#include "VRBPDatatypes.h"
#include "Grippables/GrippableActor.h"
#include "Engine/StaticMeshActor.h"
#include "TVRWeaponAttachment.generated.h"

UCLASS()
class TACTICALVRCORE_API ATVRWeaponAttachment : public AActor
{
	GENERATED_BODY()

	UPROPERTY(Category = "Weapon Attachment", BlueprintReadOnly, EditDefaultsOnly, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* Mesh;
	
	UPROPERTY(Category = "Weapon Attachment", BlueprintReadOnly, EditDefaultsOnly, meta = (AllowPrivateAccess = "true"))
	class USceneComponent* RootScene;

	
public:
	static FName StaticMeshComponentName;
	static FName NAME_GripOverride;

public:
	// Sets default values for this actor's properties
	ATVRWeaponAttachment(const FObjectInitializer& OI);

	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	UFUNCTION(Category = "Weapon Attachment", BlueprintCallable)
    virtual void AttachToWeapon(class UTVRAttachmentPoint* AttachPoint);


	UFUNCTION(Category= "Weapon Attachment", BlueprintCallable)
    virtual class ATVRGunBase* GetGunOwner() const;

	class UStaticMeshComponent* GetStaticMeshComponent() const { return Mesh;}

	virtual void SetCollisionProfile(FName NewProfile);

	virtual FName GetGripSlotOverride() const {return GripSlotOverride;}
	FName GripSlotOverride;

	virtual bool GetGripSlot(
		const FVector& WorldLocation,
		class UGripMotionControllerComponent* CallingController,
		FTransform& OutTransform,
		FName& OutSlotName
	) const {return false;}


protected:
	TSubclassOf<UStaticMeshComponent> StaticMeshClass;
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnOwnerGripped(class UGripMotionControllerComponent* GrippingHand, const struct FBPActorGripInformation& GripInfo);
	virtual void OnOwnerGripped_Implementation(class UGripMotionControllerComponent* GrippingHand, const struct FBPActorGripInformation& GripInfo) {}
	UFUNCTION()
	void OnOwnerDropped(class UGripMotionControllerComponent* GrippingHand, const struct FBPActorGripInformation& GripInfo, bool bSocketed);
	virtual void OnOwnerDropped_Implementation(class UGripMotionControllerComponent* GrippingHand, const struct FBPActorGripInformation& GripInfo, bool bSocketed) {}

protected:
	UPROPERTY()
	TArray<class UStaticMeshComponent*> AttachmentMeshes;
	
};

