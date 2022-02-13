// This file is covered by the LICENSE file in the root of this plugin.

#include "Player/TVRCharacterMovementComponent.h"
#include "Components/TVRClimbableCapsuleComponent.h"
#include "Player/TVRCharacter.h"

FClimbInfo::FClimbInfo()
    : GrippedCapsule(nullptr),
    GrippingHand(nullptr),
    GripLocation(FVector::ZeroVector),
    bIsRelative(false)
{
}
const FClimbInfo FClimbInfo::None = FClimbInfo();

UTVRCharacterMovementComponent::UTVRCharacterMovementComponent(const FObjectInitializer& OI) : Super(OI)
{
    ClimbInfo = FClimbInfo::None;
}

void UTVRCharacterMovementComponent::PhysCustom_Climbing(float DeltaTime, int32 Iterations)
{
    if (CharacterOwner->IsLocallyControlled())
    {
        if(ATVRCharacter* VRCharacter = Cast<ATVRCharacter>(CharacterOwner))
        {
            if(ClimbInfo.IsValid())
            {
                const FVector GripWorldLocation =ClimbInfo.GrippedCapsule->GetComponentTransform().TransformPosition(ClimbInfo.GripLocation);
                const FVector HandWorldLocation = ClimbInfo.GrippingHand->GetComponentLocation();
                const FVector Movement = GripWorldLocation - HandWorldLocation;
                AddCustomReplicatedMovement(Movement);
            }
        }
    }    
    Super::PhysCustom_Climbing(DeltaTime, Iterations);    
}

void UTVRCharacterMovementComponent::InitiateClimbing(const FClimbInfo& NewClimbInfo)
{
    if(NewClimbInfo.IsValid())
    {
        ClimbInfo = NewClimbInfo;
        SetClimbingMode(true);
    }
    else
    {
        ClimbInfo = NewClimbInfo;
        SetClimbingMode(false);
    }
}

bool UTVRCharacterMovementComponent::IsClimbingHand(UGripMotionControllerComponent* TestHand) const
{
    return ClimbInfo.IsValid() ? TestHand == ClimbInfo.GrippingHand : false;
}

UTVRClimbableCapsuleComponent* UTVRCharacterMovementComponent::CurrentClimbComp() const
{
    if(ClimbInfo.IsValid())
    {
        return ClimbInfo.GrippedCapsule;
    }
    return nullptr;
}
