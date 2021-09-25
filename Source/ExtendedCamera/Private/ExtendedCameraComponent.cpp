// Copyright Acinonyx Ltd. 2021. All Rights Reserved.

#include "ExtendedCameraComponent.h"


// TODO REMOVE
#include "DrawDebugHelpers.h"


// Set Primary
void UExtendedCameraComponent::SetPrimaryCameraTrackAlpha(float Alpha)
{
    CameraPrimaryTrackBlendAlpha = Alpha;
}

float UExtendedCameraComponent::GetPrimaryCameraTrackAlpha()
{
    return CameraPrimaryTrackBlendAlpha;
}


// Set Secondary
void UExtendedCameraComponent::SetSecondaryCameraTrackAlpha(float Alpha)
{
    CameraSecondaryTrackBlendAlpha = Alpha;
}

float UExtendedCameraComponent::GetSecondaryCameraTrackAlpha()
{
    return CameraSecondaryTrackBlendAlpha;
}

void UExtendedCameraComponent::SetCameraPrimaryTrack(FVector& InLocation, FRotator& InRotation, float InFOV)
{
    PrimaryTrackLocation = InLocation;
    PrimaryTrackRotation = InRotation;
    PrimaryTrackFOV = InFOV;
}

void UExtendedCameraComponent::SetCameraSecondaryTrack(FVector& InLocation, FRotator& InRotation, float InFOV)
{
    SecondaryTrackLocation = InLocation;
    SecondaryTrackRotation = InRotation;
    SecondaryTrackFOV = InFOV;
}

void UExtendedCameraComponent::SetCameraPrimaryLocationRotation(FVector& InLocation, FRotator& InRotation)
{
    PrimaryTrackLocation = InLocation;
    PrimaryTrackRotation = InRotation;
}

void UExtendedCameraComponent::SetCameraSecondaryLocationRotation(FVector& InLocation, FRotator& InRotation)
{
    SecondaryTrackLocation = InLocation;
    SecondaryTrackRotation = InRotation;
}

void UExtendedCameraComponent::SetCameraPrimaryRotation(FRotator& InRotation)
{
    PrimaryTrackRotation = InRotation;
}

void UExtendedCameraComponent::SetCameraSecondaryRotation(FRotator& InRotation)
{
    SecondaryTrackRotation = InRotation;
}

void UExtendedCameraComponent::SetCameraPrimaryLocation(FVector& InLocation)
{
    PrimaryTrackLocation = InLocation;
}

void UExtendedCameraComponent::SetCameraSecondaryLocation(FVector& InLocation)
{
    SecondaryTrackLocation = InLocation;
}

void UExtendedCameraComponent::SetCameraPrimaryFOV(float InFOV)
{
    PrimaryTrackFOV = InFOV;
}

void UExtendedCameraComponent::SetCameraSecondaryFOV(float InFOV)
{
    SecondaryTrackFOV = InFOV;
}


bool UExtendedCameraComponent::GetUsePrimaryTrack()
{
    return FMath::IsNearlyEqual(CameraPrimaryTrackBlendAlpha, 1.f);
}

bool UExtendedCameraComponent::GetUseSecondaryTrack()
{
    return FMath::IsNearlyEqual(CameraSecondaryTrackBlendAlpha, 1.f);
}

void UExtendedCameraComponent::SetCameraMode(EExtendedCameraMode NewMode)
{
    CameraLOSMode = NewMode;
}

TEnumAsByte<EExtendedCameraMode> UExtendedCameraComponent::GetCameraMode()
{
    return CameraLOSMode;
}

void UExtendedCameraComponent::KeepInFrameLineOfSight_Implementation(AActor* Owner, FMinimalViewInfo& DesiredView)
{
    CommonKeepLineOfSight(Owner, DesiredView);
}

void UExtendedCameraComponent::KeepAnyLineOfSight_Implementation(AActor* Owner, FMinimalViewInfo& DesiredView)
{
    CommonKeepLineOfSight(Owner, DesiredView);
}

void UExtendedCameraComponent::CommonKeepLineOfSight_Implementation(AActor* Owner, FMinimalViewInfo& DesiredView)
{
    auto world = GetWorld();

    if (world)
    {
        // Cast from owner to pawn
        FCollisionQueryParams params{};
        params.AddIgnoredActor(Owner);
        FHitResult LOSCheck{};

        world->LineTraceSingleByChannel(LOSCheck, Owner->GetActorLocation(), DesiredView.Location, this->GetCollisionObjectType(), params);

        if (LOSCheck.bBlockingHit)
        {
            DesiredView.Location = LOSCheck.ImpactPoint;
        }
    }
    else
    {
        // How did we get here?
        checkNoEntry();
    }
}

void UExtendedCameraComponent::GetCameraView(float DeltaTime, FMinimalViewInfo& DesiredView)
{     
    // Start a counter here so it captures the super call
    DECLARE_SCOPE_CYCLE_COUNTER(TEXT("GetCameraView (Including Super::)"), STAT_ACIGetCameraViewInc, STATGROUP_ACIExtCam);

    // Parent Call
    Super::GetCameraView(DeltaTime, DesiredView);

    // Start a second counter that excludes the parent view update
    DECLARE_SCOPE_CYCLE_COUNTER(TEXT("GetCameraView (Excluding Super::)"), STAT_ACIGetCameraViewExc, STATGROUP_ACIExtCam);

    // Initialise the Offset
    float OffsetTrackFOV = DesiredView.FOV;

    // Set OffsetTrack for the primary blend if it's non-zero
    if (!FMath::IsNearlyZero(PrimaryTrackFOV))
    {
        OffsetTrackFOV = bUseAdditiveOffset ? (PrimaryTrackFOV + AdditiveFOVOffset) : PrimaryTrackFOV;
    }
     

    // Check for the primary track
    if (!FMath::IsNearlyZero(CameraPrimaryTrackBlendAlpha))
    {
        if (GetUsePrimaryTrack())
        {
            DesiredView.Location = PrimaryTrackLocation;
            DesiredView.Rotation = PrimaryTrackRotation;
            DesiredView.FOV = OffsetTrackFOV;
        }
        else
        {
            DesiredView.Location = FMath::Lerp(DesiredView.Location, PrimaryTrackLocation, CameraPrimaryTrackBlendAlpha);
            DesiredView.Rotation = FMath::Lerp(DesiredView.Rotation, PrimaryTrackRotation, CameraPrimaryTrackBlendAlpha);
            DesiredView.FOV = FMath::Lerp(DesiredView.FOV, OffsetTrackFOV, CameraPrimaryTrackBlendAlpha);
        }
    }


    // Set OffsetTrack for the second if it's non-zero
    if (!FMath::IsNearlyZero(PrimaryTrackFOV))
    {
        OffsetTrackFOV = bUseAdditiveOffset ? (SecondaryTrackFOV + AdditiveFOVOffset) : SecondaryTrackFOV;
    }
    else
    {
        // Initialise the Offset, again
        OffsetTrackFOV = DesiredView.FOV;
    }


    // Are we fully blended to the either game or primary track
    if (!FMath::IsNearlyZero(CameraSecondaryTrackBlendAlpha))
    {
        // Are we fully blended to the secondary track
        if (GetUseSecondaryTrack())
        {
            DesiredView.Location = SecondaryTrackLocation;
            DesiredView.Rotation = SecondaryTrackRotation;
            DesiredView.FOV = OffsetTrackFOV;
        }

        // We need to blend!
        else
        {
            DesiredView.Location = FMath::Lerp(DesiredView.Location, SecondaryTrackLocation, CameraSecondaryTrackBlendAlpha);
            DesiredView.Rotation = FMath::Lerp(DesiredView.Rotation, SecondaryTrackRotation, CameraSecondaryTrackBlendAlpha);
            DesiredView.FOV = FMath::Lerp(DesiredView.FOV, OffsetTrackFOV, CameraPrimaryTrackBlendAlpha);
        }
    }

    // Do we LOS
    // This is done twice, kinda. C'est la vie
    if (EExtendedCameraMode::KeepLos == CameraLOSMode || EExtendedCameraMode::KeepLosNoDot == CameraLOSMode)
    {
        auto componentOwner = GetOwner();

        if (componentOwner)
        {
            auto ownerLocation = componentOwner->GetActorLocation();
            auto FOVCheckRads = FMath::Cos(DesiredView.FOV * 0.5f) - FOVCheckOffsetInRadians;
            bool FrameLOS = EExtendedCameraMode::KeepLos == CameraLOSMode && FVector::DotProduct(DesiredView.Rotation.Vector(), (ownerLocation - DesiredView.Location).GetSafeNormal()) > FOVCheckRads;

            if ( FrameLOS)
            {
                KeepInFrameLineOfSight(componentOwner, DesiredView);
            }
            else if(EExtendedCameraMode::KeepLosNoDot == CameraLOSMode)
            {
                KeepAnyLineOfSight(componentOwner, DesiredView);
            }
            else
            {
                // Owner went out of frame
                // We aren't looking at the player
            }
        }
        else
        {
            //Owner is not valid
        }
    }
    else
    {
        // Do nothing. We're not in LOS mode at all
    }
}

void UExtendedCameraComponent::SetCameraPrimaryTrack(FVector&& InLocation, FRotator&& InRotation, float InFOV)
{
    PrimaryTrackLocation = InLocation;
    PrimaryTrackRotation = InRotation;
    PrimaryTrackFOV = InFOV;
}

void UExtendedCameraComponent::SetCameraSecondaryTrack(FVector&& InLocation, FRotator&& InRotation, float InFOV)
{
    SecondaryTrackLocation = InLocation;
    SecondaryTrackRotation = InRotation;
    SecondaryTrackFOV = InFOV;
}

void UExtendedCameraComponent::SetCameraPrimaryLocationRotation(FVector&& InLocation, FRotator&& InRotation)
{
    PrimaryTrackLocation = InLocation;
    PrimaryTrackRotation = InRotation;
}

void UExtendedCameraComponent::SetCameraSecondaryLocationRotation(FVector&& InLocation, FRotator&& InRotation)
{
    SecondaryTrackLocation = InLocation;
    SecondaryTrackRotation = InRotation;
}

void UExtendedCameraComponent::SetCameraPrimaryRotation(FRotator&& InRotation)
{
    PrimaryTrackRotation = InRotation;
}

void UExtendedCameraComponent::SetCameraSecondaryRotation(FRotator&& InRotation)
{
    SecondaryTrackRotation = InRotation;
}

void UExtendedCameraComponent::SetCameraPrimaryLocation(FVector&& InLocation)
{
    PrimaryTrackLocation = InLocation;
}

void UExtendedCameraComponent::SetCameraSecondaryLocation(FVector&& InLocation)
{
    SecondaryTrackLocation = InLocation;
}
