// This file is covered by the LICENSE file in the root of this plugin.

#include "Components/TVRHoverInputVolume.h"

#include "GripMotionControllerComponent.h"
#include "TacticalCollisionProfiles.h"


// Sets default values for this component's properties
UTVRHoverInputVolume::UTVRHoverInputVolume(): Super()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	SetGenerateOverlapEvents(true);
	SetCollisionProfileName(COLLISION_INTERACT_VOLUME);
	SetAutoActivate(true);
	InputBreakOffDistance = 10.f;
}


// Called when the game starts
void UTVRHoverInputVolume::BeginPlay()
{
	Super::BeginPlay();

	OnComponentEndOverlap.AddDynamic(this, &UTVRHoverInputVolume::OnEndOverlap);
}


void UTVRHoverInputVolume::CheckEnableTick()
{
	SetComponentTickEnabled(MagReleasePressed != nullptr || BoltReleasePressed != nullptr);
}

// Called every frame
void UTVRHoverInputVolume::TickComponent(float DeltaTime, ELevelTick TickType,
                                         FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if(MagReleasePressed != nullptr)
	{
		const auto C = MagReleasePressed;
		const float InputBreakOffDistSq = InputBreakOffDistance * InputBreakOffDistance;
		if(IsValid(C))
		{
			if ((C->GetPivotLocation() - GetComponentLocation()).SizeSquared() > InputBreakOffDistSq)
			{
				OnMagReleaseReleased(C);
			}
		}
	}

	if(BoltReleasePressed != nullptr)
	{
		const auto C = BoltReleasePressed;
		const float InputBreakOffDistSq = InputBreakOffDistance * InputBreakOffDistance;
		if(C->IsValidLowLevelFast() && !C->IsPendingKill())
		{
			if ((C->GetPivotLocation() - GetComponentLocation()).SizeSquared() > InputBreakOffDistSq)
			{
				OnBoltReleaseReleased(C);
			}
		}
	}
}

void UTVRHoverInputVolume::OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
}

bool UTVRHoverInputVolume::OnUsed(UGripMotionControllerComponent* Controller)
{
	if(IsActive() && EventOnUsed.IsBound())
	{
		EventOnUsed.Broadcast(Controller);
		return true;
	}
	return false;
}

bool UTVRHoverInputVolume::OnMagReleasePressed(UGripMotionControllerComponent* Controller)
{
	if(IsActive() && MagReleasePressed == nullptr && EventOnMagReleasePressed.IsBound())
	{
		EventOnMagReleasePressed.Broadcast(Controller);
		MagReleasePressed = Controller;
		CheckEnableTick();
		return true;
	}
	return false;
}

bool UTVRHoverInputVolume::OnBoltReleasePressed(UGripMotionControllerComponent* Controller)
{
	if(IsActive() && EventOnBoltReleasePressed.IsBound())
	{
		EventOnBoltReleasePressed.Broadcast(Controller);
		BoltReleasePressed = Controller;
		CheckEnableTick();
		return true;
	}
	return false;
}

bool UTVRHoverInputVolume::OnMagReleaseReleased(UGripMotionControllerComponent* Controller)
{
	if(IsActive() && MagReleasePressed != nullptr && EventOnMagReleaseReleased.IsBound())
	{		
		EventOnMagReleaseReleased.Broadcast(Controller);
		MagReleasePressed = nullptr;
		CheckEnableTick();
		return true;
	}
	return false;
}

bool UTVRHoverInputVolume::OnBoltReleaseReleased(UGripMotionControllerComponent* Controller)
{
	if(IsActive() && BoltReleasePressed != nullptr && EventOnBoltReleaseReleased.IsBound())
	{		
		EventOnBoltReleaseReleased.Broadcast(Controller);
		BoltReleasePressed = nullptr;
		CheckEnableTick();
		return true;
	}
	return false;
}

bool UTVRHoverInputVolume::OnLaserPressed(UGripMotionControllerComponent* Controller)
{
	if(IsActive() && EventOnLaserPressed.IsBound())
	{
		EventOnLaserPressed.Broadcast(Controller);
		return true;
	}
	return false;
}

bool UTVRHoverInputVolume::OnLightPressed(UGripMotionControllerComponent* Controller)
{
	if(IsActive() && EventOnLightPressed.IsBound())
	{
		EventOnLightPressed.Broadcast(Controller);
		return true;
	}
	return false;
}

