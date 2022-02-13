// This file is covered by the LICENSE file in the root of this plugin.

#include "Weapon/TVRProjectile.h"

// Sets default values
ATVRProjectile::ATVRProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ATVRProjectile::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ATVRProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

