// This file is covered by the LICENSE file in the root of this plugin.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/Component/TVRMagazineCompInterface.h"
#include "TVRInternalMagazineComponent.generated.h"


/**
 * For any kind of internal magazine, like shotgun magtubes, etc.
 */
UCLASS(Blueprintable, BlueprintType,
	meta = (BlueprintSpawnableComponent),
	ClassGroup = (TacticalVR)
)
class TACTICALVRCORE_API UTVRInternalMagazineComponent : public UTVRMagazineCompInterface
{
	GENERATED_BODY()
	
	UPROPERTY()
	class UAudioComponent* MagAudioComp;
	
public:
	UTVRInternalMagazineComponent(const FObjectInitializer& OI);

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	// ================================================
	// Start MagazineComponentInterface
	// ================================================
	
	/**
	* Overlap Event. Bound to the BeginOverlap delegate.
	* @param OverlappedComponent Component that is overlapping (should be this component)
	* @param OtherActor The actors that is overlapping with the magwell
	* @param OtherComp The component of OtherActor that is overlapping with this
	* @param OtherBodyIndex Body Index of the overlapping body
	* @param bFromSweep Whether the overlap was triggered by a sweep
	* @param SweepResult Sweep result
	*/
    virtual void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult) override;

	virtual bool IsEmpty() const override;
	virtual bool CanFeedAmmo() const override;
	virtual TSubclassOf<class ATVRCartridge> TryFeedAmmo() override;
	virtual bool CanBoltLock() const override;
	
	// ================================================
	// Start MagazineComponentInterface
	// ================================================

	/**
	* Checks a given class is one of the allowed Magazine Types.
	* @param TestClass Class to test
	* @returns True of the class is an allowed magazine type
	*/
	virtual bool IsAllowedAmmo(UClass* TestClass) const;
	virtual bool CanInsertAmmo() const;
	virtual bool IsFull() const;
	
protected:
	
	virtual void BeginPlay() override;

	void BeginInsertCartridge(class ATVRCartridge* Cartridge);

	void FullyInsertCartridge();

	void SetCurrentInsertCartridge(ATVRCartridge* Cartridge);

	virtual void AttachCartridge(ATVRCartridge* Cartridge);

	float CartridgeSpeed = 0.f;
	
	UPROPERTY()
	class ATVRCartridge* CurrentInsertingCartridge;

	/** NOT YET IMPLEMENTED */
	UPROPERTY(Category="Magazine", EditDefaultsOnly)
	bool bUseComplexInsertion;
	
	UPROPERTY(Category="Magazine", EditDefaultsOnly)
	USoundBase* AmmoInsertSound;
	
	UPROPERTY(Category = "Magazine", EditDefaultsOnly)	
	TArray<TSubclassOf<class ATVRCartridge>> CompatibleAmmo;

	UPROPERTY(Category= "Magazine", EditDefaultsOnly, meta=(ClampMin=1))
	uint8 Capacity;
	UPROPERTY(Category= "Magazine", BlueprintReadOnly, meta=(ClampMin=0))
	uint8 CurrentAmmo;

	TArray<TSubclassOf<class ATVRCartridge> > InsertedAmmo;
};
