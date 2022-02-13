// This file is covered by the LICENSE file in the root of this plugin.

#include "Weapon/TVRCartridge.h"

#include "TacticalCollisionProfiles.h"
#include "Interfaces/TVRHandSocketInterface.h"
#include "Components/CapsuleComponent.h"
#include "Components/AudioComponent.h"
#include "Particles/ParticleSystem.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

// Sets default values
ATVRCartridge::ATVRCartridge(const FObjectInitializer& OI) : Super(OI)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	GetStaticMeshComponent()->SetNotifyRigidBodyCollision(true); // enable hit events
	GetStaticMeshComponent()->SetCollisionProfileName(COLLISION_WEAPON);

	CollisionCapsule = CreateDefaultSubobject<UCapsuleComponent>(FName("CollisionCapsule"));
	CollisionCapsule->SetupAttachment(GetStaticMeshComponent());
	CollisionCapsule->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));
	CollisionCapsule->SetCapsuleSize(0.75f, 2.f);
	CollisionCapsule->SetCollisionProfileName(COLLISION_MAGAZINE_INSERT, false);

	HitAudioComponent = CreateDefaultSubobject<UAudioComponent>(FName("HitAudio"));
	HitAudioComponent->SetupAttachment(GetStaticMeshComponent());
	HitAudioComponent->SetAutoActivate(false);

	bIsBuckshot = false;
	NumBucks = 1;
	BuckshotSpread = 0.f;
	SpentCartridgeMesh = nullptr;
	bIsSpent = false;
	TraceDistance = 5000.f;
}

// Called when the game starts or when spawned
void ATVRCartridge::BeginPlay()
{
	Super::BeginPlay();

	GetStaticMeshComponent()->OnComponentHit.AddDynamic(this, &ATVRCartridge::OnComponentHit);
}

// Called every frame
void ATVRCartridge::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ATVRCartridge::OnComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	constexpr float HitSoundThresholdSq = 0.3f*0.3f;
	const float HitStrength = NormalImpulse.SizeSquared();
	const float DeltaStrength = HitStrength-HitSoundThresholdSq;
	if(DeltaStrength > 0)
	{
		HitAudioComponent->Stop();
		HitAudioComponent->SetVolumeMultiplier(FMath::Clamp(DeltaStrength/10.f, 0.f, 1.f));
		HitAudioComponent->SetIntParameter(FName(TEXT("SurfaceType")),
			Hit.PhysMaterial.IsValid() ? Hit.PhysMaterial->SurfaceType : EPhysicalSurface::SurfaceType_Default);
		HitAudioComponent->Play();
	}
}

void ATVRCartridge::ClosestGripSlotInRange_Implementation(FVector WorldLocation, bool bSecondarySlot,
                                                             bool& bHadSlotInRange, FTransform& SlotWorldTransform, FName& SlotName,
                                                             UGripMotionControllerComponent* CallingController, FName OverridePrefix)
{
	if(UHandSocketComponent* HS = ITVRHandSocketInterface::Execute_GetHandSocket(this, NAME_None))
	{
		bHadSlotInRange = true;
		SlotName = FName("HandSocket");
		SlotWorldTransform = HS->GetHandSocketTransform(CallingController);
	}
	else
	{
		SlotName = NAME_None;
		bHadSlotInRange = false;
	}
}

const FImpactParticleData* ATVRCartridge::GetImpactParticle(EPhysicalSurface SurfaceType) const
{
	if(const auto ParticleRef = ImpactParticles.Find(SurfaceType))
	{
		return ParticleRef;
	}
	// we'll look for the default surface particle
	if(const auto ParticleRef = ImpactParticles.Find(EPhysicalSurface::SurfaceType_Default))
	{
		return ParticleRef;
	}
	// there's nothing setup
	return nullptr;
}

const FImpactDecalData* ATVRCartridge::GetImpactDecal(EPhysicalSurface SurfaceType) const
{
	if(const auto DecalRef = ImpactDecals.Find(SurfaceType))
	{
		return DecalRef;
	}
	// we'll look for the default surface particle
	if(const auto DecalRef = ImpactDecals.Find(EPhysicalSurface::SurfaceType_Default))
	{
		return DecalRef;
	}
	// there's nothing setup
	return nullptr;
}

