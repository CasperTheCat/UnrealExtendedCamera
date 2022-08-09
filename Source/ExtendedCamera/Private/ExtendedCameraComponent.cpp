// Copyright Acinonyx Ltd. 2022. All Rights Reserved.

#include "ExtendedCameraComponent.h"
#include "DrawDebugHelpers.h"
#include "CollisionQueryParams.h"
#include "Engine/World.h"

void UExtendedCameraComponent::SmoothReturn_Implementation(AActor* Owner, FMinimalViewInfo& DesiredView, float DeltaTime)
{
    if (SmoothReturnOnLineOfSight && WasLineOfSightBlockedRecently)
    {
        // If LOS is blocked, do nothing
        // Also do nothing is LOS was not blocked recently

        if (FVector::DistSquared(StoredPreviousLocationForReturn, DesiredView.Location) < KINDA_SMALL_NUMBER)
        {
            WasLineOfSightBlockedRecently = false;
            return;
        }

        StoredPreviousLocationForReturn = FMath::VInterpTo(StoredPreviousLocationForReturn, DesiredView.Location, DeltaTime, SmoothReturnSpeed);
        DesiredView.Location = StoredPreviousLocationForReturn;
    }
}

void UExtendedCameraComponent::LineOfCheckHandler_Implementation(AActor* Owner, FMinimalViewInfo& DesiredView)
{
    if (Owner)
    {
        auto ownerLocation = Owner->GetActorLocation();

        if (EExtendedCameraMode::KeepLos == CameraLOSMode)
        {
            auto FOVAsRads = FMath::DegreesToRadians(DesiredView.FOV * 0.5f);
            auto FOVCheckRads = FMath::Cos(FOVAsRads) - FOVCheckOffsetInRadians;

            if (FVector::DotProduct(DesiredView.Rotation.Vector(), (ownerLocation - DesiredView.Location).GetSafeNormal()) > FOVCheckRads)
            {
                KeepInFrameLineOfSight(Owner, DesiredView);
            }
        }

        else if (EExtendedCameraMode::KeepLosWithinLimit == CameraLOSMode)
        {
            if (FVector::DotProduct(DesiredView.Rotation.Vector(), (ownerLocation - DesiredView.Location).GetSafeNormal()) > FOVCheckOffsetInRadians)
            {
                KeepInFrameLineOfSight(Owner, DesiredView);
            }
        }

        else if (EExtendedCameraMode::KeepLosNoDot == CameraLOSMode)
        {
            KeepAnyLineOfSight(Owner, DesiredView);
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

void UExtendedCameraComponent::TrackingHandler_Implementation(AActor* Owner, FMinimalViewInfo& DesiredView, float DeltaTime)
{
    if (FirstTrackCameraDriverMode == EExtendedCameraDriverMode::Compat)
    {
        // Old Version. Sets the same stuff to maintain backwards compatibility
        // Write Tracked Values if we're using it
        if (!IgnorePrimaryTrackedCamera && IsValid(PrimaryTrackedCamera))
        {
            const auto CameraComp = PrimaryTrackedCamera->GetCameraComponent();
            if (CameraComp)
            {
                PrimaryTrackTransform = PrimaryTrackedCamera->GetTransform();
                PrimaryTrackFOV = CameraComp->FieldOfView;
            }
        }
    }
    else if (FirstTrackCameraDriverMode == EExtendedCameraDriverMode::DataDriven)
    {
        // Do Nothing? Data is being directly driven
    }
    else if (FirstTrackCameraDriverMode == EExtendedCameraDriverMode::ReferenceCameraDriven)
    {
        // New way to specify that we wish to track a camera directly
        if (IsValid(PrimaryTrackedCamera))
        {
            const auto CameraComp = PrimaryTrackedCamera->GetCameraComponent();
            if (CameraComp)
            {
                PrimaryTrackTransform = PrimaryTrackedCamera->GetTransform();
                PrimaryTrackFOV = CameraComp->FieldOfView;
            }
        }
    }
    else if (FirstTrackCameraDriverMode == EExtendedCameraDriverMode::LocAndAim)
    {
        // Uses Locs and Aims
        // Aim is not valid without locator. We need the data from it
        if (IsValid(PrimaryTrackLocator))
        {
            auto Locator = PrimaryTrackLocator->GetActorLocation();
            SetCameraPrimaryLocation(Locator);

            // Uses Locs and Aims
            if (IsValid(PrimaryTrackAim))
            {
                const auto BaseAimLocation = PrimaryTrackAim->GetActorTransform().TransformPosition(PrimaryTrackAimOffset);
                const auto LookAt = BaseAimLocation - Locator;
                FRotator FinalRotation = FMath::RInterpTo(PrimaryTrackPastFrameLookAt, LookAt.Rotation(), DeltaTime, PrimaryTrackAimInterpolationSpeed);
                PrimaryTrackPastFrameLookAt = FinalRotation;
                SetCameraPrimaryRotation(FinalRotation);

#if ENABLE_DRAW_DEBUG
                if (PrimaryTrackAimDebug)
                {
                    DrawDebugSolidBox(GetWorld(), BaseAimLocation, FVector(12.f), FColor(200, 200, 32, 128));
                    DrawDebugBox(GetWorld(), BaseAimLocation, FVector(12.f), FColor::Black);
                }
#endif // ENABLE_DRAW_DEBUG
            }
        }
    }


    // Second
    if (SecondTrackCameraDriverMode == EExtendedCameraDriverMode::Compat)
    {
        // Write Tracked Values if we're using it
        if (!IgnoreSecondTrackedCamera && IsValid(SecondTrackedCamera))
        {
            const auto CameraComp = SecondTrackedCamera->GetCameraComponent();
            if (CameraComp)
            {
                SecondaryTrackTransform = SecondTrackedCamera->GetTransform();
                SecondaryTrackFOV = CameraComp->FieldOfView;
            }
        }
    }
    else if (SecondTrackCameraDriverMode == EExtendedCameraDriverMode::DataDriven)
    {
        // Do Nothing? Data is being directly driven
    }
    else if (SecondTrackCameraDriverMode == EExtendedCameraDriverMode::ReferenceCameraDriven)
    {
        // Write Tracked Values if we're using it
        if (IsValid(SecondTrackedCamera))
        {
            const auto CameraComp = SecondTrackedCamera->GetCameraComponent();
            if (CameraComp)
            {
                SecondaryTrackTransform = SecondTrackedCamera->GetTransform();
                SecondaryTrackFOV = CameraComp->FieldOfView;
            }
        }
    }
    else if (SecondTrackCameraDriverMode == EExtendedCameraDriverMode::LocAndAim)
    {
        // Uses Locs and Aims
        // Aim is not valid without locator. We need the data from it
        if (IsValid(SecondaryTrackLocator))
        {
            auto Locator = SecondaryTrackLocator->GetActorLocation();
            SetCameraSecondaryLocation(Locator);

            // Uses Locs and Aims
            if (IsValid(SecondaryTrackAim))
            {
                const auto BaseAimLocation = SecondaryTrackAim->GetActorTransform().TransformPosition(SecondaryTrackAimOffset);
                const auto LookAt = BaseAimLocation - Locator;
                FRotator FinalRotation = FMath::RInterpTo(SecondaryTrackPastFrameLookAt, LookAt.Rotation(), DeltaTime, SecondaryTrackAimInterpolationSpeed);
                SecondaryTrackPastFrameLookAt = FinalRotation;
                SetCameraSecondaryRotation(FinalRotation);

#if ENABLE_DRAW_DEBUG
                if (SecondaryTrackAimDebug)
                {
                    DrawDebugSolidBox(GetWorld(), LookAt, FVector(12.f), FColor(250, 150, 32, 128));
                    DrawDebugBox(GetWorld(), LookAt, FVector(12.f), FColor::Black);
                }
#endif // ENABLE_DRAW_DEBUG
            }
        }
    }
}

UExtendedCameraComponent::UExtendedCameraComponent()
    : SmoothReturnOnLineOfSight(false)
    , SmoothReturnSpeed(1)
    , WasLineOfSightBlockedRecently(false)
    , FirstTrackCameraDriverMode(EExtendedCameraDriverMode::Compat)
    , SecondTrackCameraDriverMode(EExtendedCameraDriverMode::Compat)
{

}

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
    auto World = GetWorld();

    if (World)
    {
        // Cast from owner to pawn
        FCollisionQueryParams params{};
        params.AddIgnoredActor(Owner);
        FHitResult LOSCheck{};
        
        World->LineTraceSingleByChannel(LOSCheck, Owner->GetActorLocation(), DesiredView.Location, this->GetCollisionObjectType(), params);


        
        if (LOSCheck.bBlockingHit)
        {
            if(!IsLOSBlocked)
            {
                StoredLOSFOV = DesiredView.FOV;    
            }
            
            if(UseDollyZoomForLOS)
            {
                DollyZoom(Owner, DesiredView, LOSCheck);
            }

            // Must be done after dollyzoom, otherwise we'll lerp nothing
            DesiredView.Location = LOSCheck.ImpactPoint;// + LOSCheck.ImpactNormal * 0.1f;

            // Update to reflect the offset position of the camera
            // We need it later when the hit is not blocked
            if (SmoothReturnOnLineOfSight)
            {
                // This gets set back to false when the lerp ends
                WasLineOfSightBlockedRecently = true;
                StoredPreviousLocationForReturn = DesiredView.Location;
            }
        }
        IsLOSBlocked = LOSCheck.bBlockingHit;
    }
    else
    {
        // How did we get here?
        checkNoEntry();
    }
}

void UExtendedCameraComponent::DollyZoom(AActor* Owner, FMinimalViewInfo& DesiredView, FHitResult &LOSCheck)
{
   
    // Compute the theta now. This is just FOV at distance X
    //const auto CurrentTheta = FMath::DegreesToRadians(DesiredView.FOV * 0.5f);
    const auto DVL = DesiredView.Location;
    const auto OAL = Owner->GetActorLocation();
    const auto LIP = LOSCheck.ImpactPoint;
    //const auto CurrentDistanceSQ = FVector::DistSquared(OAL, DVL);
    //const auto NewDistanceSQ = FVector::DistSquared(OAL, LIP);
    //const auto DistanceRatio = FMath::Sqrt(CurrentDistanceSQ / NewDistanceSQ);

    //DesiredView.FOV = 2 * FMath::RadiansToDegrees(FMath::Atan((FMath::Tan(CurrentTheta) * FVector::Dist(OAL, DVL) / FVector::Dist(OAL, LIP)))); 
    //DesiredView.FOV = 2 * FMath::RadiansToDegrees(FMath::Atan(FMath::Tan(CurrentTheta) * DistanceRatio));
    DesiredView.FOV = DollyZoom(FVector::Dist(OAL, DVL), DesiredView.FOV, FVector::Dist(OAL, LIP));
}

float UExtendedCameraComponent::DollyZoom(float ReferenceDistance, float ReferenceFOV, float CurrentDistance)
{
    // A bit of theory
    // A size of TD := 2(tan(0) * x) where theta is FOV/2
    //     /|  -
    //   /  |  |
    // /__x_|  | TD
    // \    |  |
    //   \  |  |
    //     \|  -
    // This computes the width or height, depending on the which theta we use
    // We only have horizontal FOV, but the FOV shouldn't be varying between zooms
    //
    // Ultimately, We want TD to remain the same as we move from x to x' and we care about theta prime
    // 2(tan(0) * x) = 2(tan(0') * x')
    // tan(0) * x = tan(0') * x'
    // tan(0') = (tan(0) * x) / x'
    // 0' = atan((tan(0) * x) / x')
    //
    // Finally, we can combine x / x' as a ratio between the distances
    // 0' = atan(tan(0) * r)
    
    const auto ReferenceTheta = FMath::DegreesToRadians(ReferenceFOV * 0.5f);
    const auto DistanceRatio = ReferenceDistance / CurrentDistance;
    return 2 * FMath::RadiansToDegrees(FMath::Atan(FMath::Tan(ReferenceTheta) * DistanceRatio));
}

void UExtendedCameraComponent::GetCameraView(float DeltaTime, FMinimalViewInfo& DesiredView)
{     
    // Start a counter here so it captures the super call
    DECLARE_SCOPE_CYCLE_COUNTER(TEXT("GetCameraView (Including Super::)"), STAT_ACIGetCameraViewInc, STATGROUP_ACIExtCam);

    // Parent Call
    Super::GetCameraView(DeltaTime, DesiredView);

    // Start a second counter that excludes the parent view update
    DECLARE_SCOPE_CYCLE_COUNTER(TEXT("GetCameraView (Excluding Super::)"), STAT_ACIGetCameraViewExc, STATGROUP_ACIExtCam);

    // Get Owner
    const auto ComponentOwner = GetOwner();
    
    // Initialise the Offset
    float OffsetTrackFOV = IsLOSBlocked ? StoredLOSFOV : DesiredView.FOV;

    TrackingHandler(ComponentOwner, DesiredView, DeltaTime);


    // Blending
    
    // Set OffsetTrack for the primary blend if it's non-zero
    if (!FMath::IsNearlyZero(PrimaryTrackFOV))
    {
        OffsetTrackFOV = bUseAdditiveOffset ? (PrimaryTrackFOV + AdditiveFOVOffset) : PrimaryTrackFOV;
    }
     

    // Check for the primary track
    if (!FMath::IsNearlyZero(CameraPrimaryTrackBlendAlpha))
    {
        // DollyZoom
        if(FirstTrackDollyZoomEnabled)
        {
            if (FirstTrackDollyZoomDistanceLiveUpdate)
            {
                FirstTrackDollyZoomReferenceDistance = FVector::Dist(ComponentOwner->GetActorLocation(), PrimaryTrackTransform.GetLocation());
            }
        
            OffsetTrackFOV = DollyZoom(FirstTrackDollyZoomReferenceDistance, SecondaryTrackFOV, FVector::Dist(ComponentOwner->GetActorLocation(), PrimaryTrackTransform.GetLocation()));
        }
        
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
        // DollyZoom
        if(SecondTrackDollyZoomEnabled)
        {
            if (SecondTrackDollyZoomDistanceLiveUpdate)
            {
                SecondTrackDollyZoomReferenceDistance = FVector::Dist(ComponentOwner->GetActorLocation(), SecondaryTrackTransform.GetLocation());
            }

            OffsetTrackFOV = DollyZoom(SecondTrackDollyZoomReferenceDistance, SecondaryTrackFOV, FVector::Dist(ComponentOwner->GetActorLocation(), SecondaryTrackTransform.GetLocation()));
        }
        
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

    // Do SmoothReturn first, otherwise we can push the camera back out of bounds
    SmoothReturn(ComponentOwner, DesiredView, DeltaTime);

    // Now LOS
    LineOfCheckHandler(ComponentOwner, DesiredView);


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
