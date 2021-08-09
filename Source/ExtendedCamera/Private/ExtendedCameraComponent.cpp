// Copyright Acinonyx Ltd. 2021. All Rights Reserved.

#include "ExtendedCameraComponent.h"


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

bool UExtendedCameraComponent::GetUsePrimaryTrack()
{
    return FMath::IsNearlyEqual(CameraPrimaryTrackBlendAlpha, 1.f);
}

bool UExtendedCameraComponent::GetUseSecondaryTrack()
{
    return FMath::IsNearlyEqual(CameraSecondaryTrackBlendAlpha, 1.f);
}

void UExtendedCameraComponent::GetCameraView(float DeltaTime, FMinimalViewInfo& DesiredView)
{
    Super::GetCameraView(DeltaTime, DesiredView);

    // Check for the primary track
    if (!FMath::IsNearlyZero(CameraPrimaryTrackBlendAlpha))
    {
        if (GetUsePrimaryTrack())
        {
            DesiredView.Location = PrimaryTrackLocation;
            DesiredView.Rotation = PrimaryTrackRotation;
        }
        else
        {
            DesiredView.Location = FMath::Lerp(DesiredView.Location, PrimaryTrackLocation, CameraPrimaryTrackBlendAlpha);
            DesiredView.Rotation = FMath::Lerp(DesiredView.Rotation, PrimaryTrackRotation, CameraPrimaryTrackBlendAlpha);
        }
    }


    // Are we fully blended to the either game or primary track
    if (FMath::IsNearlyZero(CameraSecondaryTrackBlendAlpha))
    {
        // Do nothing when in game track
        // Already set Desired when primary track
        return;
    }

    // Are we fully blended to the secondary track
    else if (GetUseSecondaryTrack())
    {
        DesiredView.Location = SecondaryTrackLocation;
        DesiredView.Rotation = SecondaryTrackRotation;
    }

    // We need to blend!
    else
    {
        DesiredView.Location = FMath::Lerp(DesiredView.Location, SecondaryTrackLocation, CameraSecondaryTrackBlendAlpha);
        DesiredView.Rotation = FMath::Lerp(DesiredView.Rotation, SecondaryTrackRotation, CameraSecondaryTrackBlendAlpha);
    }
}