// This file is covered by the LICENSE file in the root of this plugin.

#pragma once

#include "CoreMinimal.h"
#include "Grippables/GrippableCapsuleComponent.h"
#include "TVRClimbableCapsuleComponent.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, meta = (BlueprintSpawnableComponent), ClassGroup = (TacticalVR))
class TACTICALVRCORE_API UTVRClimbableCapsuleComponent : public UGrippableCapsuleComponent
{
	GENERATED_BODY()
public:
    UTVRClimbableCapsuleComponent(const FObjectInitializer& OI);

    virtual void OnGrip_Implementation(UGripMotionControllerComponent* GrippingHand, const FBPActorGripInformation& GripInfo) override;
    virtual void OnGripRelease_Implementation(UGripMotionControllerComponent* GrippingHand, const FBPActorGripInformation& GripInfo, bool bWasSocketed) override;

};
