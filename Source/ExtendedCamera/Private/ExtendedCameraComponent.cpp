// Copyright Acinonyx Ltd. 2022. All Rights Reserved.

#include "ExtendedCameraComponent.h"
#include "CollisionQueryParams.h"
#include "Engine/World.h"

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

    // Do we LOS
    // This is done twice, kinda. C'est la vie
    if (EExtendedCameraMode::KeepLos == CameraLOSMode || EExtendedCameraMode::KeepLosNoDot == CameraLOSMode)
    {
        if (ComponentOwner)
        {
            auto ownerLocation = ComponentOwner->GetActorLocation();
            auto FOVAsRads = FMath::DegreesToRadians(DesiredView.FOV * 0.5f);
            auto FOVCheckRads = FMath::Cos(FOVAsRads) - FOVCheckOffsetInRadians;
            bool FrameLOS = EExtendedCameraMode::KeepLos == CameraLOSMode && FVector::DotProduct(DesiredView.Rotation.Vector(), (ownerLocation - DesiredView.Location).GetSafeNormal()) > FOVCheckRads;

            if ( FrameLOS)
            {
                KeepInFrameLineOfSight(ComponentOwner, DesiredView);
            }
            else if(EExtendedCameraMode::KeepLosNoDot == CameraLOSMode)
            {
                KeepAnyLineOfSight(ComponentOwner, DesiredView);
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
