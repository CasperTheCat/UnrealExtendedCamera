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
    PrimaryTrackTransform.SetLocation(InLocation);
    PrimaryTrackTransform.SetRotation(InRotation.Quaternion());
    PrimaryTrackFOV = InFOV;
}

void UExtendedCameraComponent::SetCameraSecondaryTrack(FVector& InLocation, FRotator& InRotation, float InFOV)
{
    SecondaryTrackTransform.SetLocation(InLocation);
    SecondaryTrackTransform.SetRotation(InRotation.Quaternion());
    SecondaryTrackFOV = InFOV;
}

void UExtendedCameraComponent::SetCameraPrimaryTransform(FTransform& InTransform, float InFOV)
{
    PrimaryTrackTransform = InTransform;
    PrimaryTrackFOV = InFOV;
}

void UExtendedCameraComponent::SetCameraSecondaryTransform(FTransform& InTransform, float InFOV)
{
    SecondaryTrackTransform = InTransform;
    SecondaryTrackFOV = InFOV;
}

void UExtendedCameraComponent::SetCameraPrimaryLocationRotation(FVector& InLocation, FRotator& InRotation)
{
    PrimaryTrackTransform.SetLocation(InLocation);
    PrimaryTrackTransform.SetRotation(InRotation.Quaternion());
}

void UExtendedCameraComponent::SetCameraSecondaryLocationRotation(FVector& InLocation, FRotator& InRotation)
{
    SecondaryTrackTransform.SetLocation(InLocation);
    SecondaryTrackTransform.SetRotation(InRotation.Quaternion());
}

void UExtendedCameraComponent::SetCameraPrimaryRotation(FRotator& InRotation)
{
    PrimaryTrackTransform.SetRotation(InRotation.Quaternion());
}

void UExtendedCameraComponent::SetCameraSecondaryRotation(FRotator& InRotation)
{
    SecondaryTrackTransform.SetRotation(InRotation.Quaternion());
}

void UExtendedCameraComponent::SetCameraPrimaryLocation(FVector& InLocation)
{
    PrimaryTrackTransform.SetLocation(InLocation);
}

void UExtendedCameraComponent::SetCameraSecondaryLocation(FVector& InLocation)
{
    SecondaryTrackTransform.SetLocation(InLocation);
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

    // Write Tracked Values if we're using it
    if (TrackedCamera)
    {
        const auto CameraComp = TrackedCamera->GetCameraComponent();
        if (CameraComp)
        {
            // We need to write values
            if (WriteTrackedToSecondary)
            {
                // Writing to Secondary
                // SecondaryTrackTransform = TrackedCamera->GetComponentTransform();
                // SecondaryTrackFOV = TrackedCamera->FieldOfView;
                SecondaryTrackTransform = TrackedCamera->GetTransform();
                SecondaryTrackFOV = CameraComp->FieldOfView;
            }
            else
            {
                PrimaryTrackTransform = TrackedCamera->GetTransform();
                PrimaryTrackFOV = CameraComp->FieldOfView;
            }
        }
    }

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
            DesiredView.Location = PrimaryTrackTransform.GetLocation();
            DesiredView.Rotation = PrimaryTrackTransform.GetRotation().Rotator();
            DesiredView.FOV = OffsetTrackFOV;
        }
        else
        {
            DesiredView.Location = FMath::Lerp(DesiredView.Location, PrimaryTrackTransform.GetLocation(), CameraPrimaryTrackBlendAlpha);
            DesiredView.Rotation = FMath::Lerp(DesiredView.Rotation, PrimaryTrackTransform.GetRotation().Rotator(), CameraPrimaryTrackBlendAlpha);
            DesiredView.FOV = FMath::Lerp(DesiredView.FOV, OffsetTrackFOV, CameraPrimaryTrackBlendAlpha);
        }
    }


    // Set OffsetTrack for the second if it's non-zero
    if (!FMath::IsNearlyZero(SecondaryTrackFOV))
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
            DesiredView.Location = SecondaryTrackTransform.GetLocation();
            DesiredView.Rotation = SecondaryTrackTransform.GetRotation().Rotator();
            DesiredView.FOV = OffsetTrackFOV;
        }

        // We need to blend!
        else
        {
            DesiredView.Location = FMath::Lerp(DesiredView.Location, SecondaryTrackTransform.GetLocation(), CameraSecondaryTrackBlendAlpha);
            DesiredView.Rotation = FMath::Lerp(DesiredView.Rotation, SecondaryTrackTransform.GetRotation().Rotator(), CameraSecondaryTrackBlendAlpha);
            DesiredView.FOV = FMath::Lerp(DesiredView.FOV, OffsetTrackFOV, CameraSecondaryTrackBlendAlpha);
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
    PrimaryTrackTransform.SetLocation(MoveTemp(InLocation));
    PrimaryTrackTransform.SetRotation(MoveTemp(InRotation).Quaternion());
    PrimaryTrackFOV = InFOV;
}

void UExtendedCameraComponent::SetCameraSecondaryTrack(FVector&& InLocation, FRotator&& InRotation, float InFOV)
{
    SecondaryTrackTransform.SetLocation(MoveTemp(InLocation));
    SecondaryTrackTransform.SetRotation(MoveTemp(InRotation).Quaternion());
    SecondaryTrackFOV = InFOV;
}



void UExtendedCameraComponent::SetCameraPrimaryLocationRotation(FVector&& InLocation, FRotator&& InRotation)
{
    PrimaryTrackTransform.SetLocation(MoveTemp(InLocation));
    PrimaryTrackTransform.SetRotation(MoveTemp(InRotation).Quaternion());
}

void UExtendedCameraComponent::SetCameraSecondaryLocationRotation(FVector&& InLocation, FRotator&& InRotation)
{
    SecondaryTrackTransform.SetLocation(MoveTemp(InLocation));
    SecondaryTrackTransform.SetRotation(MoveTemp(InRotation).Quaternion());
}

void UExtendedCameraComponent::SetCameraPrimaryRotation(FRotator&& InRotation)
{
    PrimaryTrackTransform.SetRotation(MoveTemp(InRotation).Quaternion());
}

void UExtendedCameraComponent::SetCameraSecondaryRotation(FRotator&& InRotation)
{
    SecondaryTrackTransform.SetRotation(MoveTemp(InRotation).Quaternion());
}

void UExtendedCameraComponent::SetCameraPrimaryLocation(FVector&& InLocation)
{
    PrimaryTrackTransform.SetLocation(MoveTemp(InLocation));
}

void UExtendedCameraComponent::SetCameraSecondaryLocation(FVector&& InLocation)
{
    SecondaryTrackTransform.SetLocation(MoveTemp(InLocation));
}
