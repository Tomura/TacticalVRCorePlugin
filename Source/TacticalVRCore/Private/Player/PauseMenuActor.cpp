// This file is covered by the LICENSE file in the root of this plugin.

#include "Player/PauseMenuActor.h"

// Sets default values
APauseMenuActor::APauseMenuActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
    CurrentlyActiveWidget = nullptr;
}

// Called when the game starts or when spawned
void APauseMenuActor::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void APauseMenuActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

