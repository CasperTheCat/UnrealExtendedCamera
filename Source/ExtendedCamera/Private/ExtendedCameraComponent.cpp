// Copyright Acinonyx Ltd. 2021. All Rights Reserved.

#include "ExtendedCameraComponent.h"

void UExtendedCameraComponent::SetCameraTrackAlpha(float Alpha)
{
    CameraTrackBlendAlpha = Alpha;
}

float UExtendedCameraComponent::GetCameraTrackAlpha()
{
    return CameraTrackBlendAlpha;
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

void UExtendedCameraComponent::SetUsePrimaryTrack(bool usePrimaryTrack)
{
    CameraIsUsingPrimaryTrack = usePrimaryTrack;
}

bool UExtendedCameraComponent::GetUsePrimaryTrack()
{
    return CameraIsUsingPrimaryTrack;
}

void UExtendedCameraComponent::GetCameraView(float DeltaTime, FMinimalViewInfo& DesiredView)
{
    Super::GetCameraView(DeltaTime, DesiredView);

    // Check for the primary track
    if (CameraIsUsingPrimaryTrack)
    {
        DesiredView.Location = PrimaryTrackLocation;
        DesiredView.Rotation = PrimaryTrackRotation;
    }


    // Are we fully blended to the either game or primary track
    if (FMath::IsNearlyZero(CameraTrackBlendAlpha))
    {
        // Do nothing when in game track
        // Already set Desired when primary track
        return;
    }

    // Are we fully blended to the secondary track
    else if (FMath::IsNearlyEqual(CameraTrackBlendAlpha, 1.f))
    {
        DesiredView.Location = SecondaryTrackLocation;
        DesiredView.Rotation = SecondaryTrackRotation;
    }

    // We need to blend!
    else
    {
        DesiredView.Location = FMath::Lerp(DesiredView.Location, SecondaryTrackLocation, CameraTrackBlendAlpha);
        DesiredView.Rotation = FMath::Lerp(DesiredView.Rotation, SecondaryTrackRotation, CameraTrackBlendAlpha);
    }
}