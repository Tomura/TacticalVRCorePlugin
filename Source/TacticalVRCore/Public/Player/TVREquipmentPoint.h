// This file is covered by the LICENSE file in the root of this plugin.

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "Weapon/TVRGunBase.h"
#include "TVREquipmentPoint.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TACTICALVRCORE_API UTVREquipmentPoint : public UStaticMeshComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UTVREquipmentPoint();

	virtual void OnChildAttached(USceneComponent* ChildComponent) override;
	virtual void OnChildDetached(USceneComponent* ChildComponent) override;
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(Category="Equip", EditDefaultsOnly)
	TSubclassOf<class ATVRGunBase> AllowedGunClass;
	UPROPERTY(Category="Equip", EditDefaultsOnly)
	float AttachRange;
	UPROPERTY(Category="Equip", EditDefaultsOnly)
	float LerpSpeed;
	UPROPERTY(Category="Equip", EditDefaultsOnly)
	float LerpRotSpeed;

	class AActor* AttachedActor;
	
public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	virtual bool HasAttachment() const;

	virtual bool ShouldLerpAttachment() const;

	virtual void AttachGun(class ATVRGunBase* ActorToAttach);

	virtual bool IsPointInRange(FVector WorldLocation) const;

	virtual bool CanAcceptGun(class ATVRGunBase* TestGun);
};
