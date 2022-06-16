// This file is covered by the LICENSE file in the root of this plugin.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TVRTypes.h"
#include "TVRGunHapticsComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Abstract, Blueprintable, BlueprintType)
class TACTICALVRCORE_API UTVRGunHapticsComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UTVRGunHapticsComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
	// A function that is only run locally during Begin Play for the Owning Client of the Player Controller
	UFUNCTION(Category = "Gun Haptics",  BlueprintNativeEvent)
	void LocalBeginPlay();
	virtual void LocalBeginPlay_Implementation();

public:	

	UFUNCTION(Category = "Gun Haptics", BlueprintCallable, Unreliable, Client)
	void ClientButtstockKick(uint8 Strength, float Duration);

	UFUNCTION(Category = "Gun Haptics", BlueprintCallable, Unreliable, Client)
	void ClientPistolKick(uint8 Strength, float Duration, ETVRLeftRight Type);

	UFUNCTION(Category = "Gun Haptics", BlueprintCallable)
	virtual class APlayerController* GetOwnerPlayerController() const;
	
protected:
	UFUNCTION(Category = "Gun Hapctics", BlueprintNativeEvent)
	void ButtstockKick(uint8 Strength, float Duration);
	virtual void ButtstockKick_Implementation(uint8 Strength, float Duration);
	
	UFUNCTION(Category = "Gun Hapctics", BlueprintNativeEvent)
	void PistolKick(uint8 Strength, float Duration, ETVRLeftRight Type);
	virtual void PistolKick_Implementation(uint8 Strength, float Duration, ETVRLeftRight Type);
};
