// This file is covered by the LICENSE file in the root of this plugin.

#pragma once

#include "CoreMinimal.h"
#include "TVRTypes.h"
#include "UObject/Interface.h"
#include "TVRChargingHandleInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(BlueprintType)
class UTVRChargingHandleInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class TACTICALVRCORE_API ITVRChargingHandleInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	/**
	 * @returns charging handle progress as a number between 0.0 and 1.0, where 1.0 means it is fully deflected
	 */
	UFUNCTION(Category="Charging Handle Interface", BlueprintCallable, BlueprintNativeEvent)
	float GetProgress() const;
	virtual float GetProgress_Implementation() const {return 0.f;}
	
	UFUNCTION(Category="Charging Handle Interface", BlueprintCallable, BlueprintNativeEvent)
	float GetMaxTavel() const;
	virtual float GetMaxTavel_Implementation() const {return 0.f;}
	
	UFUNCTION(Category="Charging Handle Interface", BlueprintCallable, BlueprintNativeEvent)
	float GetStiffness() const;
	virtual float GetStiffness_Implementation() const {return 0.f;}

	/**
	 * @returns true if the charging handle is locked (cannot cycle back to 0)
	 */
	UFUNCTION(Category="Charging Handle Interface", BlueprintCallable, BlueprintNativeEvent)
	bool IsLocked() const;
	virtual bool IsLocked_Implementation() const { return false; }
	
	UFUNCTION(Category="Charging Handle Interface", BlueprintCallable, BlueprintNativeEvent)
	bool IsInUse() const;
	virtual bool IsInUse_Implementation() const {return false;}

	UFUNCTION(Category="Charging Handle Interface", BlueprintCallable, BlueprintNativeEvent)
	bool IsReciprocating() const;
	virtual bool IsReciprocating_Implementation() const {return false;}

	
	UFUNCTION(Category="Charging Handle Interface", BlueprintCallable, BlueprintNativeEvent)
	ETVRLeftRight GetGrabLocation() const;
	virtual ETVRLeftRight GetGrabLocation_Implementation() const {return ETVRLeftRight::None;}
	
	UFUNCTION(Category="Charging Handle Interface", BlueprintCallable, BlueprintNativeEvent)
	void SetAudioComponent(UAudioComponent* NewComp);
	virtual void SetAudioComponent_Implementation(UAudioComponent* NewComp) {}
	
	UFUNCTION(Category="Charging Handle Interface", BlueprintCallable, BlueprintNativeEvent)
	void SetStiffness(float NewStiffness);
	virtual void SetStiffness_Implementation(float NewStiffness) {}
	
	UFUNCTION(Category="Charging Handle Interface", BlueprintCallable, BlueprintNativeEvent)
	void SetMaxTravel(float NewTravel);
	virtual void SetMaxTravel_Implementation(float NewTravel) {}
	
	UFUNCTION(Category="Charging Handle Interface", BlueprintCallable, BlueprintNativeEvent)
	void SetProgress(float NewProgress);
	virtual void SetProgress_Implementation(float NewProgress) {}

	UFUNCTION(Category="Charging Handle Interface", BlueprintCallable, BlueprintNativeEvent)
	void LockChargingHandle(float LockProgress = -1.f);
	virtual void LockChargingHandle_Implementation(float LockProgress = -1.f) {}
	
	UFUNCTION(Category="Charging Handle Interface", BlueprintCallable, BlueprintNativeEvent)
	void UnlockChargingHandle();
	virtual void UnlockChargingHandle_Implementation() {}

	
	UFUNCTION(Category="Charging Handle Interface", BlueprintCallable, BlueprintNativeEvent)	
	bool CanInitiateFire();
	virtual bool CanInitiateFire_Implementation() {return true;}

	UFUNCTION(Category="Charging Handle Interface", BlueprintCallable, BlueprintNativeEvent)	
	bool CanSuccessfullyFire();
	virtual bool CanSuccessfullyFire_Implementation() {return true;}

	
	UFUNCTION(Category="Charging Handle Interface", BlueprintCallable, BlueprintNativeEvent)
	void SetBoltProgress(float NewProgress);
	virtual void SetBoltProgress_Implementation(float NewProgress) {}

	UFUNCTION(Category="Charging Handle Interface", BlueprintCallable, BlueprintNativeEvent)
	float GetBoltProgress() const;
	virtual float GetBoltProgress_Implementation() const {return 0.f;}
	
	UFUNCTION(Category="Charging Handle Interface", BlueprintCallable, BlueprintNativeEvent)
	void OnBoltClosed() const;
	virtual void OnBoltClosed_Implementation() const {}
};
