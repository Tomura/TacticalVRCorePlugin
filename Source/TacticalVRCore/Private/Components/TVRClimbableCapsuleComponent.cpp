// This file is covered by the LICENSE file in the root of this plugin.

#include "Components/TVRClimbableCapsuleComponent.h"
#include "Player/TVRCharacter.h"
#include "Player/TVRCharacterMovementComponent.h"

UTVRClimbableCapsuleComponent::UTVRClimbableCapsuleComponent(const FObjectInitializer& OI) : Super(OI)
{
    VRGripInterfaceSettings.SecondaryGripType = ESecondaryGripType::SG_None;
    VRGripInterfaceSettings.FreeDefaultGripType = EGripCollisionType::CustomGrip;
    VRGripInterfaceSettings.SlotDefaultGripType = EGripCollisionType::CustomGrip;
    VRGripInterfaceSettings.bSimulateOnDrop = false;
    VRGripInterfaceSettings.bAllowMultipleGrips = true;

    GameplayTags.AddTag(FGameplayTag::RequestGameplayTag(FName("GripType.Large")));
}

void UTVRClimbableCapsuleComponent::OnGrip_Implementation(UGripMotionControllerComponent* GrippingHand,
    const FBPActorGripInformation& GripInfo)
{
    Super::OnGrip_Implementation(GrippingHand, GripInfo);

    if(GrippingHand->GetOwner())
    {
        ATVRCharacter* HandChar = Cast<ATVRCharacter>(GrippingHand->GetOwner());
        if(HandChar != nullptr)
        {
            HandChar->TryStartClimbing(this, GrippingHand);
        }
    }
}

void UTVRClimbableCapsuleComponent::OnGripRelease_Implementation(UGripMotionControllerComponent* GrippingHand,
    const FBPActorGripInformation& GripInfo, bool bWasSocketed)
{
    Super::OnGripRelease_Implementation(GrippingHand, GripInfo, bWasSocketed);

    if(GrippingHand)
    {
        ATVRCharacter* HandChar = Cast<ATVRCharacter>(GrippingHand->GetOwner());
        if(HandChar != nullptr)
        {
            UTVRCharacterMovementComponent* MyMovementComp = Cast<UTVRCharacterMovementComponent>(HandChar->VRMovementReference);
            if(!MyMovementComp->IsClimbingHand(GrippingHand))
            {
                return;
            }
            
            // EControllerHand HandType = EControllerHand::AnyHand;
            // GrippingHand->GetHandType(HandType);
            // UGripMotionControllerComponent* OtherHand = HandChar->GetOtherControllerHand(HandType);
            //
            // TArray<UPrimitiveComponent*> GrippedComps;
            // if(OtherHand->HasGrippedObjects())
            // {
            //     OtherHand->GetGrippedComponents(GrippedComps);
            //     for(UPrimitiveComponent* TestComp : GrippedComps)
            //     {
            //         if(UTVRClimbableCapsuleComponent* GrippedClimbable = Cast<UTVRClimbableCapsuleComponent>(TestComp))
            //         {
            //             HandChar->TryStartClimbing(GrippedClimbable, OtherHand);
            //             return;
            //         }
            //     }
            // }
            
            if(MyMovementComp->IsClimbing() && MyMovementComp->IsClimbingHand(GrippingHand))
            {
                MyMovementComp->SetClimbingMode(false);
                MyMovementComp->InitiateClimbing(FClimbInfo::None);
            }
        }
    }
}


