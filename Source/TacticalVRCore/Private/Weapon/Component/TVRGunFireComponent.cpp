// This file is covered by the LICENSE file in the root of this plugin.

#include "Weapon/Component/TVRGunFireComponent.h"

#include "Components/AudioComponent.h"
#include "Components/DecalComponent.h"
#include "Components/TVRGunHapticsComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Particles/ParticleSystemComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Player/TVRCharacter.h"
#include "Player/TVRPlayerController.h"
#include "Weapon/TVRGunBase.h"
#include "Weapon/TVRGunWithChild.h"
#include "Weapon/TVRCartridge.h"
#include "Weapon/Attachments/TVRWeaponAttachment.h"
#include "Weapon/Component/TVRAttachmentPoint.h"
#include "Weapon/Component/TVRChargingHandleInterface.h"

// Sets default values for this component's properties
UTVRGunFireComponent::UTVRGunFireComponent(const FObjectInitializer& OI) : Super(OI)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	MuzzleFlashPSC = nullptr;
	FireAudioComp = nullptr;
	EmptyAudioComp = nullptr;
	FireSoundCue = nullptr;
	EmptySoundCue = nullptr;

	ShotCount = 0;
	bIsFiring = false;
	
	RefireTime = 0.1f;
	RateOfFireRPM = 600;
	
	bHasSingleShot = true;
	bHasBurst = false;
	bHasFullAuto = false;
	
    bHasFireSelector = true;

	LoadedCartridge = nullptr;
	CurrentFireMode = ETVRFireMode::Single;
	bCartridgeIsSpent = false;
	
	bUseGunHapticsPistolGrip = false;
	bUseGunHapticsButtstock = false;

	bIsSuppressed = false;
	MuzzleFlashOverride = nullptr;
	
	bDefaultSuppressed = false;
	ImpactSoundComp = nullptr;
}

void UTVRGunFireComponent::SetSuppressed(bool NewValue)
{
	bIsSuppressed = NewValue;
	if(FireAudioComp)
	{
		FireAudioComp->SetBoolParameter(FName("IsSuppressed"), bIsSuppressed);
	}
}

void UTVRGunFireComponent::ResetSuppressed()
{
	SetSuppressed(bDefaultSuppressed);
}

// Called when the game starts
void UTVRGunFireComponent::BeginPlay()
{
	Super::BeginPlay();
	RandomFiringStream.GenerateNewSeed();
	
	// The order here determines the priority.
	// all weapons start out in single shot
	if(bHasSingleShot)
	{
		CurrentFireMode = ETVRFireMode::Single;
	}
	else if(bHasFullAuto)
	{
		CurrentFireMode = ETVRFireMode::Automatic;
	}
	else
	{
		CurrentFireMode = ETVRFireMode::Burst;
	}
	
	RefireTime = 60.f/RateOfFireRPM;
	
	TArray<USceneComponent*> ChildComponents;
	GetChildrenComponents(false, ChildComponents);
	for(USceneComponent* TestChild: ChildComponents)
	{
		if(MuzzleFlashPSC == nullptr)
		{
			if(auto const TestPSC = Cast<UParticleSystemComponent>(TestChild))
			{
				MuzzleFlashPSC = TestPSC;
			}
		}
	}
	
	if(FireSoundCue && !FireAudioComp)
	{
		FireAudioComp = NewObject<UAudioComponent>(GetOwner(), FName("FiringAudio"));
		FireAudioComp->AttachToComponent(this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		FireAudioComp->SetSound(FireSoundCue);
		FireAudioComp->bAutoActivate = false;
	}
	if(EmptySoundCue && !EmptyAudioComp)
	{
		EmptyAudioComp = NewObject<UAudioComponent>(GetOwner(), FName("EmptyAudio"));
		EmptyAudioComp->AttachToComponent(this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		EmptyAudioComp->SetSound(EmptySoundCue);
		EmptyAudioComp->bAutoActivate = false;
	}
	
	bDefaultSuppressed = bIsSuppressed;
	SetSuppressed(bIsSuppressed);

	
}

void UTVRGunFireComponent::PostInitProperties()
{
	Super::PostInitProperties();
	RefireTime = 60.f/RateOfFireRPM;
}

void UTVRGunFireComponent::BeginDestroy()
{
	if(FireAudioComp && !FireAudioComp->IsPendingKill())
	{
		FireAudioComp->DestroyComponent();
	}
	if(EmptyAudioComp && !EmptyAudioComp->IsPendingKill())
	{
		EmptyAudioComp->DestroyComponent();
	}
	Super::BeginDestroy();
}

FTimerManager& UTVRGunFireComponent::GetWorldTimerManager() const
{
	return GetWorld()->GetTimerManager();
}

bool UTVRGunFireComponent::IsOwnerLocalPlayerController() const
{
	const ACharacter* CharOwner = GetCharacterOwner();
	if(CharOwner && CharOwner->GetController())
	{
		return CharOwner->GetController()->IsLocalPlayerController();
	}
	return false;
}

ACharacter* UTVRGunFireComponent::GetCharacterOwner() const
{
	AActor* TestOwner = GetOwner();
	while(TestOwner)
	{
		if(ACharacter* C = Cast<ACharacter>(TestOwner))
		{
			return C;
		}
		TestOwner = TestOwner->GetOwner();
	}
	return nullptr;
}

ATVRCharacter* UTVRGunFireComponent::GetVRCharacterOwner() const
{
	AActor* TestOwner = GetOwner();
	while(TestOwner)
	{
		if(ATVRCharacter* C = Cast<ATVRCharacter>(TestOwner))
		{
			return C;
		}
		TestOwner = TestOwner->GetOwner();
	}
	return nullptr;
}


float UTVRGunFireComponent::GetRefireTime() const
{
	return FMath::Max(RefireTime, 0.02f);
}


bool UTVRGunFireComponent::HasRoundLoaded() const
{
	return LoadedCartridge != nullptr;
}

void UTVRGunFireComponent::StartEmpty()
{
	if(!IsInFiringCooldown())
	{
		LocalSimulateEmpty();
	}
	if(OnEmpty.IsBound())
	{
		OnEmpty.Broadcast();
	}
}

void UTVRGunFireComponent::StartFire()
{
	if(!IsInFiringCooldown())
	{
		ShotCount = 0;
		bIsFiring = true;
		
		if(!HasRoundLoaded() || bCartridgeIsSpent || !CanFire())
		{
			StartEmpty();
		}
		else
		{
			Fire();
		}

		if(GetOwner()->GetLocalRole() != ROLE_Authority)
		{
			ServerStartFire();
		}
	}
}

void UTVRGunFireComponent::ServerStartFire_Implementation()
{
	StartFire();
}

void UTVRGunFireComponent::StopFire()
{
	bIsFiring = false;
	ShotCount = 0;
	if(GetOwner()->GetLocalRole() != ROLE_Authority)
	{
		ServerStopFire();
	}
}

void UTVRGunFireComponent::ServerStopFire_Implementation()
{
	StopFire();
}

bool UTVRGunFireComponent::IsInFiringCooldown() const
{
	return GetWorldTimerManager().IsTimerActive(RefireTimer);
}

bool UTVRGunFireComponent::TryLoadCartridge(TSubclassOf<ATVRCartridge> NewCartridge)
{
	if(!HasRoundLoaded() && NewCartridge)
	{
		LoadedCartridge = NewCartridge;
		bCartridgeIsSpent = false;
		return true;
	}
	return false;
}

TSubclassOf<ATVRCartridge> UTVRGunFireComponent::TryEjectCartridge()
{
	if(HasRoundLoaded())
	{
		auto EjectedCartridge = LoadedCartridge;
		LoadedCartridge = nullptr;
		return EjectedCartridge;
	}
	return nullptr;
}

float UTVRGunFireComponent::GetRefireCooldownRemaining() const
{
	if(GetWorldTimerManager().IsTimerActive(RefireTimer))
	{		
		return GetWorldTimerManager().GetTimerRemaining(RefireTimer);
	}
	return 0.f;
}


float UTVRGunFireComponent::GetRefireCooldownRemainingPct() const
{
	return GetRefireCooldownRemaining()/GetRefireTime();
}

float UTVRGunFireComponent::GetDamage() const
{
	return 40.f;
}


bool UTVRGunFireComponent::ShouldRefire() const
{
	if(bIsFiring && HasRoundLoaded() && CanFire())
	{
		switch(GetCurrentFireMode())
		{
		case ETVRFireMode::Single:
			return ShotCount < 1;
		case ETVRFireMode::Burst:
			return ShotCount < BurstCount;
		case ETVRFireMode::Automatic:
			return true;
		}
	}
	return false;
}


void UTVRGunFireComponent::Fire()
{
	if(ShouldRefire())
	{
		const auto AmmoCDO = LoadedCartridge->GetDefaultObject<ATVRCartridge>();
		
		SimulateFire();        
		ShotCount++;
		bCartridgeIsSpent = true;
		if(OnCartridgeSpent.IsBound())
		{
			OnCartridgeSpent.Broadcast();
		}

		if(FireOverride.IsBound())
		{
			FireOverride.Broadcast(GetForwardVector(), LoadedCartridge);
		}
		else
		{
			if(AmmoCDO->IsBuckshot())
			{
				const FVector PendingBuckshotDir = GetForwardVector(); 
				FireBuckshot(AmmoCDO->GetNumBuckshot(), AmmoCDO, PendingBuckshotDir);
			}
			else
			{
				TArray<FHitResult> Hits;
				if(TraceFire(Hits, GetForwardVector() * AmmoCDO->GetTraceDistance()))
				{
					ProcessHits(Hits, LoadedCartridge);
				}
			}
		}

		GetWorldTimerManager().SetTimer(RefireTimer, this, &UTVRGunFireComponent::ReFire, GetRefireTime(), false);
		if(OnFire.IsBound())
		{
			OnFire.Broadcast();
		}
	}
}

void UTVRGunFireComponent::ReFire()
{
	if(GetWorldTimerManager().TimerExists(RefireTimer))
	{
		GetWorldTimerManager().ClearTimer(RefireTimer);
	}
	
	if(OnEndCycle.IsBound())
	{
		OnEndCycle.Broadcast();
	}
	Fire();
}

void UTVRGunFireComponent::SimulateFire()
{
	if(IsOwnerLocalPlayerController()) // forward prediction for local player controller
	{
		LocalSimulateFire();
	}
    
	if(GetOwner()->GetLocalRole() == ROLE_Authority) // Server: LocalRole == Authority, Client == Simulated Proxy
	{
		MulticastSimulateFire();
	}
}

void UTVRGunFireComponent::MulticastSimulateFire_Implementation()
{
	if(!IsOwnerLocalPlayerController()) // already done for owner
	{
		LocalSimulateFire();
	}
}

void UTVRGunFireComponent::LocalSimulateFire()
{
	if(GetNetMode() != ENetMode::NM_DedicatedServer)
	{
		if(MuzzleFlashOverride)
		{
			MuzzleFlashOverride->Activate(true);
		}
		else if(MuzzleFlashPSC != nullptr)
		{
			MuzzleFlashPSC->Activate(true);
		}

		if(FireAudioComp != nullptr)
		{
			FireAudioComp->Stop();
			// FireAudioComp->SetBoolParameter(FName("IsEmpty"), false);
			FireAudioComp->Play(0);
		}

		if(IsOwnerLocalPlayerController()) // ony for owner
		{
			ATVRPlayerController* OwnerPC = nullptr;
			ATVRCharacter* OwnerChar = GetVRCharacterOwner();
			if(OwnerChar)
			{
				if(OwnerChar->GetController())
				{
					OwnerPC = Cast<ATVRPlayerController>(OwnerChar->GetController());    				
				}
			}
			if(OwnerPC != nullptr)
			{
				if(const auto GunHapctics = OwnerPC->GetGunHapticsComponent())
				{
					if(bUseGunHapticsButtstock)
					{
						GunHapctics->ClientButtstockKick(255, GetRefireTime());
					}
					// todo
					// if(bUseGunHapticsPistolGrip)
					// {
					// 	GunHapctics->ClientPistolKick(255, GetRefireTime(), left or right hand);
					// }
				}
			}
		}
		if(OnSimulateFire.IsBound())
		{
			OnSimulateFire.Broadcast();
		}
	}
}

void UTVRGunFireComponent::SimulateEmpty()
{
	if(IsOwnerLocalPlayerController())
	{
		LocalSimulateEmpty();
	}
    
	if(GetOwner()->GetLocalRole() == ROLE_Authority)
	{
		MulticastSimulateEmpty();
	}
}

void UTVRGunFireComponent::MulticastSimulateEmpty_Implementation()
{
	if(!IsOwnerLocalPlayerController() && GetNetMode() != NM_DedicatedServer)
	{
		LocalSimulateEmpty();
	}
}

void UTVRGunFireComponent::LocalSimulateEmpty()
{
	if(EmptyAudioComp)
	{
		EmptyAudioComp->Stop();
		EmptyAudioComp->Play();
	}

	if(OnSimulateEmpty.IsBound())
	{
		OnSimulateEmpty.Broadcast();
	}
}

void UTVRGunFireComponent::FireBuckshot(uint8 PendingBuckshot, const ATVRCartridge* AmmoCDO, FVector PendingBuckshotDir)
{
	// Caution: In theory it can happen that multiples of this function are called in one frame for this component
	// This would happen during very high fire rates that need less that 3-4 frames. In that case there is no big benefit
	// from this logic, but it still gives a bit of a more even distribution of the load.
	
	constexpr uint8 MaxShotsPerFrame = 3;
	const uint8 ShotsToDo = FMath::Min(PendingBuckshot, MaxShotsPerFrame);
	const float BuckshotSpread = FMath::DegreesToRadians(AmmoCDO->GetBuckshotSpread());
	for(uint8 i = 0; i < ShotsToDo; i++)
	{
		const FVector TraceDir = RandomFiringStream.VRandCone(PendingBuckshotDir, BuckshotSpread);
		TArray<FHitResult> Hits;
		if(TraceFire(Hits, TraceDir * AmmoCDO->GetTraceDistance()))
		{
			ProcessHits(Hits, AmmoCDO->GetClass());		
		}
	}
	
	const uint8 NewPendingBuckshot = (PendingBuckshot > ShotsToDo) ? (PendingBuckshot - ShotsToDo) : 0;
	if(NewPendingBuckshot > 0)
	{
		FTimerDelegate BuckshotDelegate = FTimerDelegate::CreateUObject(
			this, &UTVRGunFireComponent::FireBuckshot,
			NewPendingBuckshot, AmmoCDO, PendingBuckshotDir);
		GetWorldTimerManager().SetTimerForNextTick(BuckshotDelegate);
	}
}

void UTVRGunFireComponent::AddTraceIgnoreActors(FCollisionQueryParams& QueryParams)
{
	QueryParams.AddIgnoredActor(GetOwner());
	ATVRGunBase* GunOwner = Cast<ATVRGunBase>(GetOwner());
	if(GunOwner && GunOwner->IsHeldByParentGun())
	{
		QueryParams.AddIgnoredActor(GunOwner->GetParentGun());
	}
	else if(AActor* OwnersOwner = GetOwner()->GetOwner())
	{
		const auto OwnerGunOwner = Cast<ATVRGunBase>(OwnersOwner);
		if(OwnerGunOwner && OwnerGunOwner->IsHeldByParentGun())
		{					
			QueryParams.AddIgnoredActor(OwnerGunOwner->GetParentGun());
		}
	}
}

bool UTVRGunFireComponent::TraceFire(TArray<FHitResult>& Hits, const FVector& TraceDir)
{
	const FVector TraceStart = GetComponentLocation();
	const FVector TraceEnd = TraceStart + TraceDir;
    FCollisionQueryParams QueryParams(FName("WeaponTrace"), true);
	AddTraceIgnoreActors(QueryParams);
	QueryParams.bReturnPhysicalMaterial = true;
	
	const FCollisionResponseParams ResponseParams(ECR_Block);
	GetWorld()->LineTraceMultiByChannel(
		Hits,
		TraceStart, TraceEnd,
		ECC_Visibility,
		QueryParams,
		ResponseParams
	);
	return Hits.Num() > 0;
}

void UTVRGunFireComponent::ProcessHits(TArray<FHitResult>& Hits, TSubclassOf<ATVRCartridge> Cartridge)
{
	if(Hits.Num() > 0)
	{		
		FHitResult& LastHit = Hits.Last();
		SimulateHit(LastHit, Cartridge);
		if(const auto HitActor = LastHit.GetActor())
		{
			const auto CartridgeCDO = Cartridge->GetDefaultObject<ATVRCartridge>();
			float Damage = CartridgeCDO->GetBaseDamage();
			if(GetOwner())
			{
				if(const auto GunOwner = Cast<ATVRGunBase>(GetOwner()))
				{
					for(const auto AttachPoint: GunOwner->GetAttachmentPoints())
					{
						if(const auto WPNA = AttachPoint->GetCurrentAttachment())
						{
							Damage *= WPNA->GetDamageModifier();
						}
					}
				}
			}
			const FVector TraceDir = (LastHit.ImpactPoint - LastHit.TraceStart).GetSafeNormal();
			UGameplayStatics::ApplyPointDamage(HitActor, Damage, TraceDir, LastHit, nullptr, GetOwner(), CartridgeCDO->GetDamageType());
		}
	}
}


void UTVRGunFireComponent::SimulateHit(const FHitResult& Hit, TSubclassOf<ATVRCartridge> Cartridge)
{
    ServerReceiveHit(Hit, Cartridge);
    LocalSimulateHit(Hit, Cartridge);
}

void UTVRGunFireComponent::ServerReceiveHit_Implementation(const FHitResult& Hit, TSubclassOf<ATVRCartridge> Cartridge)
{
    MulticastSimulateHit(Hit, Cartridge);

	if(Hit.bBlockingHit)
	{
		ATVRCharacter* MyChar = Cast<ATVRCharacter>(GetOwner());		
		UGameplayStatics::ApplyPointDamage(
			Hit.GetActor(),
			GetDamage(),
			Hit.TraceEnd-Hit.TraceStart, Hit,
			MyChar ? MyChar->GetController() : nullptr,
			MyChar,
			UDamageType::StaticClass()
		);
	}
}

void UTVRGunFireComponent::MulticastSimulateHit_Implementation(const FHitResult& Hit, TSubclassOf<ATVRCartridge> Cartridge)
{
    if(!IsOwnerLocalPlayerController())
    {
        LocalSimulateHit(Hit, Cartridge);
    }
}

void UTVRGunFireComponent::LocalSimulateHit(const FHitResult& Hit, TSubclassOf<ATVRCartridge> Cartridge)
{
	const auto CartridgeCDO = Cartridge->GetDefaultObject<ATVRCartridge>();
	const auto SurfaceType = Hit.PhysMaterial.IsValid() ? Hit.PhysMaterial->SurfaceType : SurfaceType_Default;
	const auto ImpactPS = CartridgeCDO->GetImpactParticle(SurfaceType);
	if(ImpactPS && ImpactPS->ParticleSystem)
	{
		const FVector TraceDir = (Hit.TraceEnd - Hit.TraceStart).GetSafeNormal();
		const FVector ImpactUpVector = Hit.Normal + TraceDir - 2 * (TraceDir | Hit.Normal) * Hit.Normal;
		FRotator ImpactRot;
		switch(ImpactPS->UpAxis)
		{
		case EAxisOption::X:
			ImpactRot = UKismetMathLibrary::MakeRotFromX(ImpactUpVector);
			break;
		case EAxisOption::Y:
			ImpactRot = UKismetMathLibrary::MakeRotFromY(ImpactUpVector);
			break;
		case EAxisOption::X_Neg:
			ImpactRot = UKismetMathLibrary::MakeRotFromX(-ImpactUpVector);
			break;
		case EAxisOption::Y_Neg:
			ImpactRot = UKismetMathLibrary::MakeRotFromY(-ImpactUpVector);
			break;
		case EAxisOption::Z_Neg:
			ImpactRot = UKismetMathLibrary::MakeRotFromZ(-ImpactUpVector);
			break;
		case EAxisOption::Z:
		default:
			ImpactRot = UKismetMathLibrary::MakeRotFromZ(ImpactUpVector);
			break;
		}
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactPS->ParticleSystem,
			Hit.ImpactPoint, ImpactRot, FVector(ImpactPS->ScaleFactor),
			true, EPSCPoolMethod::AutoRelease, true);
	}

	const auto ImpactDecal = CartridgeCDO->GetImpactDecal(SurfaceType);
	if(ImpactDecal && ImpactDecal->DecalMaterial && Hit.GetComponent() && Hit.GetComponent()->GetCollisionObjectType() == ECC_WorldStatic)
	{
		FRotator DecalRot = UKismetMathLibrary::MakeRotFromX(-Hit.ImpactNormal);
		DecalRot.Roll = FMath::RandRange(-180.f, 180.f);
		const auto NewDecal = UGameplayStatics::SpawnDecalAttached(
			ImpactDecal->DecalMaterial,
			ImpactDecal->ScaleFactor * FVector(0.5f, 1.f, 1.f),
			Hit.GetComponent(), Hit.BoneName,
			Hit.ImpactPoint, DecalRot,
			EAttachLocation::KeepWorldPosition
		);
		NewDecal->SetFadeScreenSize(0.0025f); // todo: make a setting or something
	}
	
	USoundBase* ImpactSound = CartridgeCDO->GetImpactSound();
	if(ImpactSound)
	{
		SpawnImpactSound(Hit, ImpactSound);
	}
		
	if(OnSimulateHit.IsBound())
	{
		OnSimulateHit.Broadcast(Hit, Cartridge);
	}
}

void UTVRGunFireComponent::SpawnImpactSound(const FHitResult& Hit, USoundBase* Sound)
{
	// if there already is an Audio Component for the impact, we just re-use it instead of respawning it
	// best case all cartridges use the same impact sound cue, but in case it is not, the sound will be changed
	// what is certain is that consecutive shots out of this component will have almost the same sound, at a similar location
	// the only weakness is buckshot or penetrating shots, where a lot of sound would theoretically trigger at a lot of locations
	// even then the impact points should be close enough
	//
	// A possible improvement might be to have a pooled method, but on the other hand the amount of sounds that can be played at once
	// is also limited, so a more complex system might have diminishing returns

	// we need to move back the sound a bit so that there is no occlusion though collision.
	// we use the normal
	constexpr float MoveBackDist = 1.f;
	const FVector SpawnLoc = Hit.ImpactPoint + Hit.ImpactNormal * MoveBackDist;
	const auto SurfaceType = Hit.PhysMaterial.IsValid() ? Hit.PhysMaterial->SurfaceType : SurfaceType_Default;
	if(ImpactSoundComp == nullptr)
	{
		ImpactSoundComp = UGameplayStatics::SpawnSoundAtLocation(GetWorld(), Sound,
			SpawnLoc, FRotator::ZeroRotator, 1.f, 1.f, 0.f,
			nullptr, nullptr, false);
	}
	else if(ImpactSoundComp->Sound != Sound)
	{
		ImpactSoundComp->SetSound(Sound);
	}
	
	if(ImpactSoundComp)
	{
		ImpactSoundComp->Stop();
		ImpactSoundComp->SetWorldLocation(SpawnLoc, false);
		ImpactSoundComp->SetIntParameter(FName(TEXT("SurfaceType")), SurfaceType);
		ImpactSoundComp->Play();
	}
}

ETVRFireMode UTVRGunFireComponent::GetNextFireMode(ETVRFireMode PrevFireMode) const
{
	switch (PrevFireMode)
	{
	case ETVRFireMode::Single:
		return ETVRFireMode::Burst;
	case ETVRFireMode::Burst:
		return ETVRFireMode::Automatic;
	case ETVRFireMode::Automatic:
		return ETVRFireMode::Single;            
	}
	return PrevFireMode;
}

bool UTVRGunFireComponent::SetFireMode(ETVRFireMode NewFireMode)
{
	if(HasFiringMode(NewFireMode))
	{
		CurrentFireMode = NewFireMode;
		return true;
	}
	return false;
}

bool UTVRGunFireComponent::HasFiringMode(ETVRFireMode CheckFireMode) const
{
	switch (CheckFireMode)
	{
	case ETVRFireMode::Single:
		return bHasSingleShot;
	case ETVRFireMode::Burst:
		return bHasBurst;
	case ETVRFireMode::Automatic:
		return bHasFullAuto;            
	}
	return false;
}


void UTVRGunFireComponent::CycleFireMode()
{
	if(bHasFireSelector && (bHasFullAuto || bHasBurst || bHasSingleShot))
	{
		CurrentFireMode = GetNextFireMode(CurrentFireMode);
		if(!HasFiringMode(CurrentFireMode))
		{
			CycleFireMode();
		}
		else
		{
			if(OnCycledFireMode.IsBound())
			{
				OnCycledFireMode.Broadcast();
			}
		}
	}

#if WITH_EDITOR
	if(!bHasFullAuto && !bHasBurst && !bHasSingleShot)
	{
		UE_LOG(LogTemp, Error, TEXT("Gun has no available fire mode. This will lead to errors"))
	}
#endif
}

bool UTVRGunFireComponent::CanFire() const
{
	const auto WPN = Cast<ATVRGunBase>(GetOwner());
	if(WPN && WPN->GetChargingHandleInterface() && WPN->GetBoltProgress() > KINDA_SMALL_NUMBER)
	{
		if(ITVRChargingHandleInterface::Execute_IsInUse(WPN->GetChargingHandleInterface()))
		{
			return false;
		}
	}
	return true;
}

