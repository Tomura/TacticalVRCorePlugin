// This file is covered by the LICENSE file in the root of this plugin.

#include "Weapon/TVRBullet.h"

// Sets default values
ATVRBullet::ATVRBullet()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bIsActive = false;
	Velocity = FVector::ZeroVector;
}

// Called when the game starts or when spawned
void ATVRBullet::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ATVRBullet::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if(bIsActive)
	{
		const int32 NumSubsteps = 4;
		const float StepTime = DeltaTime / NumSubsteps;
		for (int32 i = 0; i < NumSubsteps; i++)
		{
			const float GravityZ = GetWorld()->GetGravityZ();
			Velocity.Z += GravityZ;
		}	
	}
}

