// This file is covered by the LICENSE file in the root of this plugin.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "TVRGunFireComponent.generated.h"


UENUM(BlueprintType)
enum class ETVRFireMode : uint8
{
	Single,
	Burst,
	Automatic
};

/** Generic Event for GunFireComponents without any parameters. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFiringCompEvent);

/** Event for hit events. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FFireHitEvent, const FHitResult&, Hit, TSubclassOf<class ATVRCartridge>, CartridgeClass);

/** Event for overriding the firing logic. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FFireOverrideEvent, const FVector&, FireDirection, TSubclassOf<class ATVRCartridge>, CartridgeClass);

UCLASS(
	ClassGroup=(Custom),
	HideCategories=(Rendering, ComponentTick, ComponentReplication, Activation, Physics, LOD, Collision),
	meta=(BlueprintSpawnableComponent) )
class TACTICALVRCORE_API UTVRGunFireComponent : public USceneComponent
{
	GENERATED_BODY()

	
public:	
	// Sets default values for this component's properties
	UTVRGunFireComponent(const FObjectInitializer& OI);


	/**
	 * Event called when the fire mode is cycled
	 */
	UPROPERTY(Category="Events", BlueprintAssignable)
	FFiringCompEvent OnCycledFireMode;

	/**
	 * Event called when the gun is fired. This should be used to implement additional logic outside of this component.
	 */
	UPROPERTY(Category="Events", BlueprintAssignable)
	FFiringCompEvent OnFire;
	
	/**
	 * Event called when the gun is fired. This should be used to implement additional logic outside of this component.
	 */
	UPROPERTY(Category="Events", BlueprintAssignable)
	FFiringCompEvent OnEndCycle;

	/**
	 * Event called when gun fire is simulated (effects, sound) this will be run locally on all clients.
	 * Use this for additional effects
	 */
	UPROPERTY(Category="Events", BlueprintAssignable)
	FFiringCompEvent OnSimulateFire;

	
	UPROPERTY(Category="Events", BlueprintAssignable)
	FFiringCompEvent OnEmpty;
	
	/**
	 * Event called when the gun tries to fire unsuccessfully and a click is played. Additional effects that are not
	 * relevant to gameplay can be added here.
	 */
	UPROPERTY(Category="Events", BlueprintAssignable)
	FFiringCompEvent OnSimulateEmpty;

	/**
	 * Event called when the gun has hit something with hit-scan. Use it to add additional particles or other effects.
	 */
	UPROPERTY(Category="Events", BlueprintAssignable)
	FFireHitEvent OnSimulateHit;

	/**
	 * Event called when the cartridge is spent (fired). Use this event in case you want to replace your catrridge model
	 * with one of a spent cartridge.
	 */
	UPROPERTY(Category="Events", BlueprintAssignable)
	FFiringCompEvent OnCartridgeSpent;

	/**
	 * Implementing this event overrides the default firing methods (bullet). E.g. in the weapon spawns a projectile
	 * or does something else. The basic functionality like activating and replicating muzzle flash and sound will
	 * be perserved.
	 */
	UPROPERTY(Category="Events", BlueprintAssignable)
	FFireOverrideEvent FireOverride;

	virtual void SetSuppressed(bool NewValue);
	virtual void ResetSuppressed();
	
	UPROPERTY()
	class UParticleSystemComponent* MuzzleFlashOverride;
	
protected:
	
	UPROPERTY(Category = "Firing", EditDefaultsOnly, BlueprintReadWrite)
	bool bIsSuppressed;
	bool bDefaultSuppressed;

	/** Particle System to simulate the muzzle flash. Will be set during BeginPlay if this component has a suitable child component */
	UPROPERTY()
	class UParticleSystemComponent* MuzzleFlashPSC;
	

	/** Audio component for gunshots. Will be set during BeginPlay if this component has a suitable child component */
	UPROPERTY()
	class UAudioComponent* FireAudioComp;
	
	UPROPERTY()
	class UAudioComponent* EmptyAudioComp;

	
	UPROPERTY(Category = "Firing", EditDefaultsOnly)
	class USoundBase* FireSoundCue;
	
	UPROPERTY(Category = "Firing", EditDefaultsOnly)
	class USoundBase* EmptySoundCue;
	

	/** Type of the currently loaded cartridge. Will be used to determine data about the shot that is fired. */
	TSubclassOf<ATVRCartridge> LoadedCartridge;

	/** Is true if the gun is currently firing */
	bool bIsFiring;
	
	/** Count of shots fired during burst or auto fire. Is reset when the weapon stops firing. */
	uint16 ShotCount;

	/** The currently used fire mode. */
	ETVRFireMode CurrentFireMode;

	/** Flag that controls whether this gun has single shot mode */
	UPROPERTY(Category="Firing", EditDefaultsOnly)
	uint8 bHasSingleShot: 1;
	
	/** Flag that controls whether this gun has single shot mode */
	UPROPERTY(Category="Firing", EditDefaultsOnly)
	uint8 bHasBurst: 1;
	
	/** Flag that controls whether this gun has single shot mode */
	UPROPERTY(Category="Firing", EditDefaultsOnly)
	uint8 bHasFullAuto: 1;
	
	/** Flag that controls whether this gun has single shot mode */
	UPROPERTY(Category="Firing", EditDefaultsOnly)
	uint8 bHasFireSelector: 1;

	/** Time between two shots (will be corrected later) */
	float RefireTime;

	/**
	 * Rate of fire in Rounds Per Minute. Will be converted to refire time later.
	 * Changing this won't do anything after BeginPlay()
	 */
	UPROPERTY(Category="Firing", EditDefaultsOnly)
	float RateOfFireRPM;
	
	/** Count of shots fired on Burst Fire */
	UPROPERTY(Category="Firing", EditDefaultsOnly, meta=(EditCondition="bHasBurst"))
	uint8 BurstCount;
	
	/** Whether or not to initiate a kick with a haptic feedback device at buttstock (like ForceTube) */
	UPROPERTY(Category="Firing|Haptics", EditDefaultsOnly)
	uint8 bUseGunHapticsButtstock: 1;
		
	/** Whether or not to initiate a kick with haptic feedback device at pistol grip (like Provolver) */
	UPROPERTY(Category="Firing|Haptics", EditDefaultsOnly)
	uint8 bUseGunHapticsPistolGrip: 1;
	
	/** Timer that tracks when the weapon can fire again */
	FTimerHandle RefireTimer;

	/** Random Stream for Firing Logic */
	FRandomStream RandomFiringStream;

	/** True if the Cartridge was spent and cannot be used anymore. */
	bool bCartridgeIsSpent;

	UPROPERTY()
	UAudioComponent* ImpactSoundComp;
	
protected:	
	// Called when the game starts
	virtual void BeginPlay() override;
	/** Called to initialize properties before BeginPlay. */
	virtual void PostInitProperties() override;

	virtual void BeginDestroy() override;

	/**
	 * Provides easier access to the timer manager
	 * @returns the owner's World Timer Manager
	 */
	FTimerManager& GetWorldTimerManager() const;

	/**
	 * @returns true if there is a local player controller in the owner chain
	 */
	bool IsOwnerLocalPlayerController() const;

	/**
	 * @returns the character in the owner chain. Can also be null
	 */
	class ACharacter* GetCharacterOwner() const;

	/**
	 * @returns the TVRCharacter in the owner chain. Can also be null
	 */
	class ATVRCharacter* GetVRCharacterOwner() const;

	/**
	 * @returns true if the gun should Refire (or even fire at all). Mostly used to handle ending bursts, etc.
	 */
	virtual bool ShouldRefire() const;

	/**
	 * Calls the actual firing logic (Hit scan, buckshots, triggering effects, etc).
	 * Should run on the owner (forward prediction) and the server (server authority)
	 */
	virtual void Fire();
	virtual void ReFire();

	/**
	 * Calls the function that simulates fire.
	 * If this is called on the server it will send an multicast event to all clients.
	 * If this is called in the owner it will just simualate fire.
	 */
	void SimulateFire();

	/**
	 * Calls Simulate Fire on simulating clients.
	 * Will prevent execution on owner client, since it should already have performed this due to forward prediction
	 */
	UFUNCTION(Category = "Firing", Unreliable, NetMulticast)
	void MulticastSimulateFire();
	void MulticastSimulateFire_Implementation();

	/**
	 * Anything that is needed to simulate the weapon firing, such as Muzzle Flash, Firing Sound, etc.
	 * Only effects, no gameplay.
	 */
	virtual void LocalSimulateFire();

	/**
	 * Called to simulate an empty gun (click) for all simulating instances (whether they are proxies or not
	 */
	void SimulateEmpty();

	/**
	 * Calls LocalSimulateEmpty on simulating clients
	 * Weill prevent execution on owner client, since it should have already happened due to forward prediction
	 */
	UFUNCTION(Category = "Gun", Unreliable, NetMulticast)
	void MulticastSimulateEmpty();
	void MulticastSimulateEmpty_Implementation();

	/**
	 * Locally simulates an empty gun. (Sounds, effects, etc)
	 */
	UFUNCTION(Category = "Firing", BlueprintCallable)
	void LocalSimulateEmpty();

	/**
	 * Fires multiple bucks with hit-scan. The number of bucks per frame is limited, this function will call
	 * itself in the next frame to process all the other pending buckshots until it is done.
	 * @param PendingBuckshot Number of pending bucks to process
	 * @param AmmoCDO constant default object of the fired ammunition
	 * @param PendingBuckshotDir direction of the pending shot (otherwise recoil will affect this)
	 */
	UFUNCTION()
	virtual void FireBuckshot(uint8 PendingBuckshot, const ATVRCartridge* AmmoCDO, FVector PendingBuckshotDir);

	/**
	 * Adds ignored actors to the trace query params.
	 * @param QueryParams Reference to the Query Params that the ignored actors will be added to.
	 */
	virtual void AddTraceIgnoreActors(struct FCollisionQueryParams& QueryParams);

	/**
	 * Trace functionality for gun fire
	 * @param Hits Reference to the array where hits will be stored to
	 * @param TraceDir Direction of the trace
	 * @return True if we have hit anything (even overlaps/penetration)
	 */
	virtual bool TraceFire(TArray<FHitResult>& Hits, const FVector& TraceDir);

	/**
	 * Processes the hits we encountered during our trace
	 * @param Hits Reference to the hit array that is processed.
	 * @param Cartridge Class of the Cartridge that was fired
	 */
	virtual void ProcessHits(TArray<FHitResult>& Hits, TSubclassOf<class ATVRCartridge> Cartridge);
	

	UFUNCTION(Category = "Firing", Reliable, Server, WithValidation)
	void ServerReceiveHit(const FHitResult& Hit, TSubclassOf<class ATVRCartridge> Cartridge = nullptr);
	void ServerReceiveHit_Implementation(const FHitResult& Hit, TSubclassOf<class ATVRCartridge> Cartridge = nullptr);
	bool ServerReceiveHit_Validate(const FHitResult& Hit, TSubclassOf<class ATVRCartridge> Cartridge = nullptr) {return true;}
    
	void SimulateHit(const FHitResult& Hit, TSubclassOf<class ATVRCartridge> Cartridge = nullptr);
	
	UFUNCTION(Category = "Firing", NetMulticast, Unreliable) // tbh maybe make it unreliable
	void MulticastSimulateHit(const FHitResult& Hit, TSubclassOf<class ATVRCartridge> Cartridge = nullptr);
	void MulticastSimulateHit_Implementation(const FHitResult& Hit, TSubclassOf<class ATVRCartridge> Cartridge = nullptr);
    
	void LocalSimulateHit(const FHitResult& Hit, TSubclassOf<class ATVRCartridge> Cartridge = nullptr);

	void SpawnImpactSound(const FHitResult& Hit, USoundBase* Sound);
public:
	/**
	 * @returns the time it takes to fire the weapon again (min cooldown for the weapon to be ready to shoot)
	 */
	float GetRefireTime() const;

	/**
	 * @returns the current fire mode
	 */
	UFUNCTION(Category="Firing", BlueprintCallable)
	ETVRFireMode GetCurrentFireMode() const {return CurrentFireMode;}

	/**
	 * Gets the next firing mode based on the input
	 * @param PrevFireMode the previous fire mode
	 */
	ETVRFireMode GetNextFireMode(ETVRFireMode PrevFireMode) const;

	/**
	 * Changes the firing mode, outside of the normal cycling logic. E.g. blueprint implementation of staged triggers
	 * @param NewFireMode the new fire mode to switch to
	 * @returns true if the component is now in the requested fire mode
	 */
	UFUNCTION(Category="Gun", BlueprintCallable)
	bool SetFireMode(ETVRFireMode NewFireMode);

	/**
	 * Checks whether the component allows the given fire mode
	 * @param CheckFireMode the fire mode to check for
	 * @returns true if the component allows the given fire mode
	 */
	UFUNCTION(Category="Gun", BlueprintCallable)
	bool HasFiringMode(ETVRFireMode CheckFireMode) const;

	/**
	 * Cycles to the next fire mode. Will also call an event in case this happened correctly
	 */
	virtual void CycleFireMode();

	/**
	 * @returns true if the component can fire right now
	 */
	virtual bool CanFire() const;

	/**
	 * @returns true if the component has a loaded round
	 */
	UFUNCTION(Category="Gun", BlueprintCallable)
	virtual bool HasRoundLoaded() const;

	virtual void StartEmpty();
	
	UFUNCTION(Category = "Firing", BlueprintCallable)
	virtual void StartFire();
	
	UFUNCTION(Category = "Firing", Reliable, Server, WithValidation)
	void ServerStartFire();
	void ServerStartFire_Implementation();    
	bool ServerStartFire_Validate() {return true;}
	
	UFUNCTION(Category = "Firing", BlueprintCallable)
	virtual void StopFire();

	UFUNCTION(Category = "Firing", Reliable, Server, WithValidation)
	void ServerStopFire();
	void ServerStopFire_Implementation();    
	bool ServerStopFire_Validate() {return true;} 

	bool IsInFiringCooldown() const;

	UFUNCTION(Category = "Firing", BlueprintCallable)
	bool TryLoadCartridge(TSubclassOf<class ATVRCartridge> NewCartridge);
	
	UFUNCTION(Category = "Firing", BlueprintCallable)
	TSubclassOf<class ATVRCartridge> TryEjectCartridge();

	float GetRefireCooldownRemaining() const;
	float GetRefireCooldownRemainingPct() const;

	UFUNCTION(Category = "Firing", BlueprintCallable)
	TSubclassOf<ATVRCartridge> GetLoadedCartridge() const { return LoadedCartridge; }
	
	UFUNCTION(Category = "Firing", BlueprintCallable)
	bool IsCartridgeSpent() const { return bCartridgeIsSpent; }
	
	virtual float GetDamage() const;
};
