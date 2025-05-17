// Copyright (c) 2020 Tammo Beil. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "TVRAmmoSlot.generated.h"

/**
 * 
 */
UCLASS()
class TACTICALVRCORE_API UTVRAmmoSlot : public UStaticMeshComponent
{
	GENERATED_BODY()

public:
	UTVRAmmoSlot();

	UPROPERTY(Category="Ammo Slot", EditAnywhere)
	uint8 Capacity;
	
	UPROPERTY(Category="Ammo Slot", EditAnywhere)
	TSubclassOf<class ATVRMagazine> AllowedMagazineClass;

	UPROPERTY()
	TArray<class ATVRMagazine*> Magazines;

	bool CanAcceptMagazine(class ATVRMagazine* Mag) const;

	void AddMagazine(class ATVRMagazine* Mag);
	void RemoveMagazine(class ATVRMagazine* Mag);

	UFUNCTION()
	void OnMagDestroyed(AActor* DestroyedActor);

	virtual void OnChildAttached(USceneComponent* ChildComponent) override;
	virtual void OnChildDetached(USceneComponent* ChildComponent) override;
};
