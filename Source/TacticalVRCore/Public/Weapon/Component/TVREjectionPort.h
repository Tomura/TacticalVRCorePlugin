// This file is covered by the LICENSE file in the root of this plugin.

#pragma once

#include "CoreMinimal.h"
#include "TransformNoScale.h"
#include "Components/BoxComponent.h"
#include "TVREjectionPort.generated.h"

/**
 * Ejection port component, that ejects cartridges with the ability to eject spent cartridges from a pool.
 * The pooling just cycles through the pool, instead of more sophisticated approaches that searches for the first
 * unused object in the pool. This is more efficient and probably beneficial as we might be rapid firing cartridges.
 */
UCLASS(Blueprintable, meta = (BlueprintSpawnableComponent), ClassGroup = (TacticalVR))
class TACTICALVRCORE_API UTVREjectionPort : public UBoxComponent
{
	GENERATED_BODY()

	UPROPERTY()
	class UArrowComponent* EjectionArrow;
	
public:
	UTVREjectionPort(const FObjectInitializer& OI);
	virtual void BeginPlay() override;
	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;

#if WITH_EDITOR
	virtual void OnRegister() override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;
	virtual void PostEditUndo() override;
#endif

	virtual void PopulateCartridgePool();
	virtual class ATVRSpentCartridge* GetCartridgeFromPool();
	
	UFUNCTION(Category="Ejection", BlueprintCallable)
	virtual FTransform GetEjectionDir() const;

	virtual class ATVRCartridge* SpawnEjectedCartridge(TSubclassOf<class ATVRCartridge> CartridgeClass, bool bSpent);

	UFUNCTION()
	virtual void OnPooledCartridgeDestroyed(AActor* PooledCatridge);
	
protected:
	virtual void TryLoadChamber(ATVRCartridge* Cartridge);


	UPROPERTY(Category="Ejection", EditDefaultsOnly, BlueprintReadOnly, meta=(MakeEditWidget))
	FTransform EjectionDir;
	
	UPROPERTY(Category="Ejection", EditDefaultsOnly, BlueprintReadOnly)
	uint8 CartridgePoolSize;

	/** Array Index from which we grabbed our last pooled cartridge from. We just cycle through our pool. */
	int32 CartridgePoolIdx;
	bool bDoNotRespawnPool;
	
	UPROPERTY()
	TArray<class ATVRSpentCartridge*> CartridgePool;
	
	UPROPERTY(Category="Chamber", EditDefaultsOnly, BlueprintReadOnly)
	bool bAllowChamberload;

	UPROPERTY(Category="Ejection", EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<class ATVRSpentCartridge> SpentCartridgeClass;

	FRandomStream CartridgeEjectRandomStream;
};
