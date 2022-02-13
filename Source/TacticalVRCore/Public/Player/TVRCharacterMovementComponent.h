// This file is covered by the LICENSE file in the root of this plugin.

#pragma once

#include "CoreMinimal.h"
#include "VRCharacterMovementComponent.h"
#include "TVRCharacterMovementComponent.generated.h"

USTRUCT()
struct FClimbInfo
{
    GENERATED_BODY()
public:
    FClimbInfo();
    class UTVRClimbableCapsuleComponent* GrippedCapsule;
    class UGripMotionControllerComponent* GrippingHand;
    FVector GripLocation;
    bool bIsRelative;

    static const FClimbInfo None;

    bool IsValid() const
    {
        if(GrippedCapsule != nullptr && GrippingHand != nullptr)
        {
            return true;
        }
        return false;
    }
};

/**
 * 
 */
UCLASS()
class TACTICALVRCORE_API UTVRCharacterMovementComponent : public UVRCharacterMovementComponent
{
	GENERATED_BODY()
public:
    UTVRCharacterMovementComponent(const FObjectInitializer& OI);

    virtual void PhysCustom_Climbing(float DeltaTime, int32 Iterations) override;
    virtual void InitiateClimbing(const FClimbInfo& NewClimbInfo);
    virtual bool IsClimbingHand(UGripMotionControllerComponent* TestHand) const;
    virtual UTVRClimbableCapsuleComponent* CurrentClimbComp() const;
    
protected:
    FClimbInfo ClimbInfo;
};
