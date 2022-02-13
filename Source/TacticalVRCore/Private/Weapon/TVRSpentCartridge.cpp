// This file is covered by the LICENSE file in the root of this plugin.

#include "Weapon/TVRSpentCartridge.h"
#include "Components/CapsuleComponent.h"
#include "Components/AudioComponent.h"

ATVRSpentCartridge::ATVRSpentCartridge(const FObjectInitializer& OI) : Super(OI)
{
	Deactivate();
}

void ATVRSpentCartridge::SetLifeSpan(float InLifespan)
{
	Lifespan = InLifespan;
	GetWorldTimerManager().SetTimer(LifespanTimer, this, &ATVRSpentCartridge::Deactivate, Lifespan,false);

	bIsSpent = true;
}

void ATVRSpentCartridge::Deactivate()
{
	bActive = false;
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	// GetStaticMeshComponent()->SetSimulatePhysics(false);
}

void ATVRSpentCartridge::Activate()
{	
	bActive = true;
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
	// GetStaticMeshComponent()->SetSimulatePhysics(true);
}

void ATVRSpentCartridge::BeginPlay()
{
	Super::BeginPlay();
}

void ATVRSpentCartridge::ApplySpentCartridgeSettings(TSubclassOf<ATVRCartridge> TemplateType)
{	
	const ATVRCartridge* TemplateCDO = TemplateType->GetDefaultObject<ATVRCartridge>();
	if(TemplateCDO->GetSpentCartridgeMesh())
	{
		GetStaticMeshComponent()->SetStaticMesh(TemplateCDO->GetSpentCartridgeMesh());

		GetCollisionCapsule()->SetRelativeTransform(TemplateCDO->GetCollisionCapsule()->GetRelativeTransform());
		GetCollisionCapsule()->SetCapsuleSize(
			TemplateCDO->GetCollisionCapsule()->GetUnscaledCapsuleRadius(),
			TemplateCDO->GetCollisionCapsule()->GetUnscaledCapsuleHalfHeight());
	
		GetHitAudioComponent()->SetRelativeTransform(TemplateCDO->GetHitAudioComponent()->GetRelativeTransform());
		GetHitAudioComponent()->SetSound(TemplateCDO->GetHitAudioComponent()->Sound);

		VRGripInterfaceSettings.bDenyGripping = true; // this function is only used for spent casings
	}
}