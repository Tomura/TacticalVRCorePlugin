// This file is covered by the LICENSE file in the root of this plugin.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/TVRHandSocketInterface.h"
#include "Grippables/GrippableStaticMeshActor.h"
#include "TVRCartridge.generated.h"

USTRUCT()
struct TACTICALVRCORE_API FImpactParticleData
{
	GENERATED_BODY()
	
	FImpactParticleData()
	{
		ParticleSystem = nullptr;		
		UpAxis = EAxisOption::Z;
		ScaleFactor = 1.f;
	}

	UPROPERTY(Category="Impact Particle", EditDefaultsOnly)
	UParticleSystem* ParticleSystem;
	UPROPERTY(Category="Impact Particle", EditDefaultsOnly)
	TEnumAsByte<EAxisOption::Type> UpAxis;
	UPROPERTY(Category="Impact Particle", EditDefaultsOnly)
	float ScaleFactor;
};

USTRUCT()
struct TACTICALVRCORE_API FImpactDecalData
{
	GENERATED_BODY()
	
	FImpactDecalData()
	{
		DecalMaterial = nullptr;		
		ScaleFactor = 1.f;
	}

	UPROPERTY(Category="Impact Particle", EditDefaultsOnly)
	UMaterialInterface* DecalMaterial;
	UPROPERTY(Category="Impact Particle", EditDefaultsOnly)
	float ScaleFactor;
};

UCLASS(Abstract)
class TACTICALVRCORE_API ATVRCartridge : public AGrippableStaticMeshActor, public ITVRHandSocketInterface
{
	GENERATED_BODY()

	UPROPERTY(Category="Gun", BlueprintReadOnly, EditAnywhere, meta=(AllowPrivateAccess=true))
	class UCapsuleComponent* CollisionCapsule;
	
	UPROPERTY(Category="Gun", BlueprintReadOnly, EditAnywhere, meta=(AllowPrivateAccess=true))
	class UAudioComponent* HitAudioComponent;
	
public:	
	// Sets default values for this actor's properties
	ATVRCartridge(const FObjectInitializer& OI);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	virtual void OnComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	
	virtual void ClosestGripSlotInRange_Implementation(FVector WorldLocation, bool bSecondarySlot, bool& bHadSlotInRange, FTransform& SlotWorldTransform, FName& SlotName, UGripMotionControllerComponent* CallingController, FName OverridePrefix) override;
	
	UFUNCTION(Category="Cartridge", BlueprintCallable)
	bool IsBuckshot() const { return bIsBuckshot; }
	
	UFUNCTION(Category="Cartridge", BlueprintCallable)
	uint8 GetNumBuckshot() const { return NumBucks; }
	
	UFUNCTION(Category="Cartridge", BlueprintCallable)
	TSubclassOf<UDamageType> GetDamageType() const { return DamageType; }
	
	UFUNCTION(Category="Cartridge", BlueprintCallable)
	float GetBuckshotSpread() const { return BuckshotSpread; }

	class UCapsuleComponent* GetCollisionCapsule() const {return CollisionCapsule;}
	class UAudioComponent* GetHitAudioComponent() const {return HitAudioComponent;}

	UFUNCTION(Category="Cartridge", BlueprintCallable)
	UStaticMesh* GetSpentCartridgeMesh() const {return SpentCartridgeMesh;}

	UFUNCTION(Category="Cartridge", BlueprintCallable)
	float GetTraceDistance() const { return TraceDistance; }

	const FImpactParticleData* GetImpactParticle(EPhysicalSurface SurfaceType) const;
	const FImpactDecalData* GetImpactDecal(EPhysicalSurface SurfaceType) const;
	USoundBase* GetImpactSound() const { return ImpactSound; }
	
	UFUNCTION(Category="Cartridge", BlueprintCallable)
	float GetBaseDamage() const { return BaseDamage; }

public:
	UPROPERTY(Category="Cartridge", BlueprintReadWrite)
	bool bIsSpent;
	
protected:
	UPROPERTY(Category="Firing", EditDefaultsOnly)
	bool bIsBuckshot;
	
	UPROPERTY(Category="Firing", EditDefaultsOnly, meta=(EditCondition=bIsBuckshot))
	uint8  NumBucks;
	
	UPROPERTY(Category="Firing", EditDefaultsOnly, meta=(EditCondition=bIsBuckshot))
	float BuckshotSpread;
	
	UPROPERTY(Category="Firing", EditDefaultsOnly)
	UStaticMesh* SpentCartridgeMesh;
	
	UPROPERTY(Category="Firing", EditDefaultsOnly)
	TSubclassOf<UDamageType> DamageType;

	UPROPERTY(Category="Firing", EditDefaultsOnly)
	bool bIsProjectile;
	UPROPERTY(Category="Firing", EditDefaultsOnly, meta=(EditCondition=bIsProjectile))
	TSubclassOf<class ATVRProjectile> ProjectileClass;

	UPROPERTY(Category="Firing", EditDefaultsOnly)
	float TraceDistance;
	
	UPROPERTY(Category="Firing", EditDefaultsOnly)
	TMap< TEnumAsByte<EPhysicalSurface>, FImpactParticleData> ImpactParticles;
	
	UPROPERTY(Category="Firing", EditDefaultsOnly)
	TMap< TEnumAsByte<EPhysicalSurface>, FImpactDecalData> ImpactDecals;
	
	UPROPERTY(Category="Firing", EditDefaultsOnly)
	USoundBase* ImpactSound;

	UPROPERTY(Category="Firing", EditDefaultsOnly)
	float BaseDamage;
};