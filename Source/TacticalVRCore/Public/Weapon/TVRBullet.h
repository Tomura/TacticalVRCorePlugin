// This file is covered by the LICENSE file in the root of this plugin.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TVRBullet.generated.h"

UCLASS()
class TACTICALVRCORE_API ATVRBullet : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATVRBullet();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


	bool bIsActive;
	FVector Velocity;
	float DragCoefficient;

};
