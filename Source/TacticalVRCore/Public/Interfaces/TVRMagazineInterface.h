// Copyright (c) 2020 Tammo Beil. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "TVRMagazineInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UTVRMagazineInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class TACTICALVRCORE_API ITVRMagazineInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	UFUNCTION(Category="Magazine Interface", BlueprintCallable, BlueprintNativeEvent)
	bool CanFeedAmmo() const;
	virtual bool CanFeedAmmo_Implementation() const {return false;}
	
	UFUNCTION(Category="Magazine Interface", BlueprintCallable, BlueprintNativeEvent)
	TSubclassOf<class ATVRCartridge> TryFeedAmmo();
	virtual TSubclassOf<class ATVRCartridge> TryFeedAmmo_Implementation() {return nullptr;}
	
	UFUNCTION(Category="Magazine Interface", BlueprintCallable, BlueprintNativeEvent)
	void SetMagazineCollisionProfile(FName NewProfile);
	virtual void SetMagazineCollisionProfile_Implementation(FName NewProfile) {}
	
	UFUNCTION(Category="Magazine Interface", BlueprintCallable, BlueprintNativeEvent)
	bool CanBoltLock() const;
	virtual bool CanBoltLock_Implementation() const {return false;}

	UFUNCTION(Category="Magazine Interface", BlueprintCallable, BlueprintNativeEvent)
	bool IsEmpty() const;
	virtual bool IsEmpty_Implementation() const {return true;}
	
	UFUNCTION(Category="Magazine Interface", BlueprintCallable, BlueprintNativeEvent)
	void OnMagReleasePressed(bool bAlternatePress = false);
	virtual void OnMagReleasePressed_Implementation(bool bAlternatePress = false) {}
	
	UFUNCTION(Category="Magazine Interface", BlueprintCallable, BlueprintNativeEvent)
	void OnMagReleaseReleased(bool bAlternatePress = false);
	virtual void OnMagReleaseReleased_Implementation(bool bAlternatePress = false) {}
	
	UFUNCTION(Category="Magazine Interface", BlueprintCallable, BlueprintNativeEvent)
	bool IsMagReleasePressed() const;
	virtual bool IsMagReleasePressed_Implementation() const {return false;}
	
	UFUNCTION(Category="Magazine Interface", BlueprintCallable, BlueprintNativeEvent)
	float GetAmmoInsertProgress() const;
	virtual float GetAmmoInsertProgress_Implementation() const {return 0.f;}

	UFUNCTION(Category="Magazine Interface", BlueprintCallable, BlueprintNativeEvent)
	void OnOwnerGripReleased(class ATVRCharacter* OwningChar, class UGripMotionControllerComponent* Controller);
	virtual void OnOwnerGripReleased_Implementation(class ATVRCharacter* OwningChar, class UGripMotionControllerComponent*) {}

	UFUNCTION(Category="Magazine Interface", BlueprintCallable, BlueprintNativeEvent)
	void GetAllowedCatridges(TArray<TSubclassOf<class ATVRCartridge>>& OutCartridges) const;
	virtual void GetAllowedCatridges_Implementation(TArray<TSubclassOf<class ATVRCartridge>>& OutCartridges) const {}
};
