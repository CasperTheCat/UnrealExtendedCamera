// Copyright Acinonyx Ltd. 2024. All Rights Reserved.

#include "ExtendedCameraComponent.h"

#include "CollisionQueryParams.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"

#if EXTENDEDCAMERA_DEBUG_DRAW
#include "DrawDebugHelpers.h"
#endif // EXTENDEDCAMERA_DEBUG_DRAW

DEFINE_LOG_CATEGORY_STATIC(LogExtendedCamera, Warning, All);

bool BoneCheck(AActor *Actor, FName TrackedName)
{
    // Ask for the bone
    if (IsValid(Actor))
    {

        const auto AsCharacter = Cast<ACharacter>(Actor);
        if (AsCharacter)
        {
            // Get the bones
            const USkeletalMeshComponent *Mesh = AsCharacter->GetMesh();
            if (Mesh)
            {
                const auto Index = Mesh->GetBoneIndex(TrackedName);

                return Index != INDEX_NONE;
            }
        }
    }

    return false;
}

FExtendedCameraViewInfo UExtendedCameraComponent::BlendTrackViews(const FMinimalViewInfo &FromView,
                                                                  const FExtendedCameraViewInfo &ToView,
                                                                  const float Alpha,
                                                                  const TEnumAsByte<EExtendedCameraOrbitMode>
                                                                  RequestedOrbitMode)
{
    FExtendedCameraViewInfo Output{};

    if (EExtendedCameraOrbitMode::SphericalInterpolate == RequestedOrbitMode && IsValid(GetOwner()))
    {
        const auto Owner = GetOwner();
        const auto OwnerLocation = Owner->GetActorLocation();

        // Convert Location to an owner-referenced spherical system
        const auto FromViewActorToCameraVector = FromView.Location - OwnerLocation;
        const auto FromViewLength = FromViewActorToCameraVector.Length();
        const auto FromViewAsSphericalQuat = FromViewActorToCameraVector.ToOrientationQuat();

        const auto ToViewActorToCameraVector = ToView.Location - Owner->GetActorLocation();
        const auto ToViewLength = ToViewActorToCameraVector.Length();
        const auto ToViewAsSphericalQuat = ToViewActorToCameraVector.ToOrientationQuat();

        const auto BlendedQuat = FQuat::Slerp(FromViewAsSphericalQuat, ToViewAsSphericalQuat, Alpha);
        const auto BlendedLength = FMath::Lerp(FromViewLength, ToViewLength, Alpha);
        Output.Location = OwnerLocation + BlendedQuat.Vector() * BlendedLength;
    }
    else
    {
        // Simpler, lerp mode
        Output.Location = FMath::Lerp(FromView.Location, ToView.Location, Alpha);
        //Output.Rotation = FMath::Lerp(DesiredView.Rotation, PrimaryTrackTransform.GetRotation().Rotator(), Alpha);
    }

    // Currently, we only need to modify Location
    Output.Rotation = FQuat::Slerp(FromView.Rotation.Quaternion(), ToView.Rotation, Alpha);
    Output.FOV = FMath::Lerp(FromView.FOV, ToView.FOV, Alpha);

    return Output;
}

FVector UExtendedCameraComponent::GetAimLocation_Implementation(AActor *Owner)
{
    DECLARE_SCOPE_CYCLE_COUNTER(TEXT("GetAimLocation"), STAT_ACIExtCam_GetAimLocation, STATGROUP_ACIExtCam);

    // Do we need return the aim point?
    // Or just the ComponentOwner's location?
    const auto OAL = Owner->GetActorLocation();
    FVector AimPoint = FVector::ZeroVector;

    if (IsValid(PrimaryTrackAim) && (FirstTrackCameraDriverMode == EExtendedCameraDriverMode::LocAndAim ||
                                     FirstTrackCameraDriverMode == EExtendedCameraDriverMode::DataAndAim ||
                                     FirstTrackCameraDriverMode == EExtendedCameraDriverMode::Skeleton ||
                                     FirstTrackCameraDriverMode == EExtendedCameraDriverMode::SkeletonAim ||
                                     FirstTrackCameraDriverMode == EExtendedCameraDriverMode::DataAndSkeletalAim ||
                                     FirstTrackCameraDriverMode == EExtendedCameraDriverMode::SkeletonLocator))
    {
        AimPoint = FMath::Lerp(OAL,
                               GetActorAimLocation(PrimaryTrackAim, FirstTrackCameraDriverMode, PrimaryAimBoneName)
                               .TransformPosition(PrimaryTrackAimOffset),
                               CameraPrimaryTrackBlendAlpha);
    }
    else
    {
        // Get normal
        AimPoint = OAL;
    }

    if (IsValid(SecondaryTrackAim) && (SecondTrackCameraDriverMode == EExtendedCameraDriverMode::LocAndAim ||
                                       SecondTrackCameraDriverMode == EExtendedCameraDriverMode::DataAndAim ||
                                       SecondTrackCameraDriverMode == EExtendedCameraDriverMode::Skeleton ||
                                       SecondTrackCameraDriverMode == EExtendedCameraDriverMode::SkeletonAim ||
                                       SecondTrackCameraDriverMode == EExtendedCameraDriverMode::DataAndSkeletalAim ||
                                       SecondTrackCameraDriverMode == EExtendedCameraDriverMode::SkeletonLocator))
    {
        AimPoint = FMath::Lerp(AimPoint,
                               GetActorAimLocation(SecondaryTrackAim, SecondTrackCameraDriverMode, SecondaryAimBoneName)
                               .TransformPosition(SecondaryTrackAimOffset),
                               CameraSecondaryTrackBlendAlpha);
    }
    else
    {
        // Get normal
        AimPoint = FMath::Lerp(AimPoint, OAL, CameraSecondaryTrackBlendAlpha);
    }

    return AimPoint;
}

FVector UExtendedCameraComponent::GetActorTrackLocation_Implementation(AActor *Owner,
                                                                       EExtendedCameraDriverMode CameraMode,
                                                                       FName LocatorBoneName)
{
    DECLARE_SCOPE_CYCLE_COUNTER(TEXT("GetActorTrackLocation"), STAT_ACIExtCam_GetActorTrackLocation,
                                STATGROUP_ACIExtCam);

    if (EExtendedCameraDriverMode::Skeleton == CameraMode ||
        EExtendedCameraDriverMode::SkeletonLocator == CameraMode ||
        EExtendedCameraDriverMode::SkeletalLocationAndData == CameraMode)
    {
        // Ask for the bone
        const auto AsCharacter = Cast<ACharacter>(Owner);
        if (AsCharacter)
        {
            // Get the bones
            const USkeletalMeshComponent *Mesh = AsCharacter->GetMesh();
            if (Mesh)
            {
                return Mesh->GetBoneLocation(LocatorBoneName);
            }
            {
                UE_LOG(LogExtendedCamera, Warning, TEXT("Invalid Bone Name (%s)"), *LocatorBoneName.ToString());
            }
        }
        else
        {
            UE_LOG(LogExtendedCamera, Warning, TEXT("Mesh is invalid? (%s)"), *LocatorBoneName.ToString());
        }
    }
    // We need non-skeletal locator
    else
    {
        return Owner->GetActorLocation();
    }

    return FVector();
}

FTransform UExtendedCameraComponent::GetActorAimLocation_Implementation(AActor *Owner,
                                                                        EExtendedCameraDriverMode CameraMode,
                                                                        FName LocatorBoneName)
{
    DECLARE_SCOPE_CYCLE_COUNTER(TEXT("GetActorAimLocation"), STAT_ACIExtCam_GetActorAimLocation, STATGROUP_ACIExtCam);

    if (EExtendedCameraDriverMode::Skeleton == CameraMode || EExtendedCameraDriverMode::SkeletonAim == CameraMode ||
        EExtendedCameraDriverMode::DataAndSkeletalAim == CameraMode)
    {
        // Ask for the bone
        const auto AsCharacter = Cast<ACharacter>(Owner);
        if (AsCharacter)
        {
            // Get the bones
            const USkeletalMeshComponent *Mesh = AsCharacter->GetMesh();
            if (Mesh)
            {
                // Mesh->GetSocketLocation(LocatorBoneName);
                const auto BoneIdx = Mesh->GetBoneIndex(LocatorBoneName);
                if (BoneIdx != INDEX_NONE)
                {
                    return Mesh->GetBoneTransform(BoneIdx);
                }
                else
                {
                    UE_LOG(LogExtendedCamera, Warning, TEXT("Invalid Bone Name (%s)"), *LocatorBoneName.ToString());
                }
            }
            {
                UE_LOG(LogExtendedCamera, Warning, TEXT("Mesh is invalid? (%s)"), *LocatorBoneName.ToString());
            }
        }
        else
        {
            UE_LOG(LogExtendedCamera, Warning, TEXT("Aim Target is not a subclass of ACharacter"));
        }
    }
    // We need non-skeletal aim
    else
    {
        return Owner->GetActorTransform();
    }

    return FTransform();
}

void UExtendedCameraComponent::SmoothReturn_Implementation(AActor *Owner, FMinimalViewInfo &DesiredView,
                                                           float DeltaTime)
{
    if (SmoothReturnOnLineOfSight && WasLineOfSightBlockedRecently && !IsLOSBlocked)
    {
        // If LOS is blocked, do nothing
        // Also do nothing is LOS was not blocked recently

        if (FVector::DistSquared(StoredPreviousLocationForReturn, DesiredView.Location) <
            ReturnFinishedThresholdSquared)
        {
            WasLineOfSightBlockedRecently = false;
            return;
        }

        StoredPreviousLocationForReturn =
            FMath::VInterpTo(StoredPreviousLocationForReturn, DesiredView.Location, DeltaTime, SmoothReturnSpeed);
        DesiredView.Location = StoredPreviousLocationForReturn;
    }
}

void UExtendedCameraComponent::LineOfCheckHandler_Implementation(AActor *Owner, FMinimalViewInfo &DesiredView)
{
    DECLARE_SCOPE_CYCLE_COUNTER(TEXT("LineOfCheckHandler"), STAT_ACIExtCam_LineOfCheckHandler, STATGROUP_ACIExtCam);

    if (Owner)
    {
        const auto ownerLocation = Owner->GetActorLocation();

        if (EExtendedCameraMode::KeepLos == CameraLOSMode)
        {
            const auto FOVAsRads = FMath::DegreesToRadians(DesiredView.FOV * 0.5f);
            const auto FOVCheckRads = FMath::Cos(FOVAsRads) - FOVCheckOffsetInRadians;

            if (FVector::DotProduct(DesiredView.Rotation.Vector(),
                                    (ownerLocation - DesiredView.Location).GetSafeNormal()) > FOVCheckRads)
            {
                KeepInFrameLineOfSight(Owner, DesiredView);
            }
        }

        else if (EExtendedCameraMode::KeepLosWithinLimit == CameraLOSMode)
        {
            if (FVector::DotProduct(DesiredView.Rotation.Vector(),
                                    (ownerLocation - DesiredView.Location).GetSafeNormal()) > FOVCheckOffsetInRadians)
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
        // Owner is not valid
    }
}

void UExtendedCameraComponent::Tracking_CameraHandler(FTransform &TrackTransform, float &TrackFOV,
                                                      TObjectPtr<ACameraActor> CameraActor)
{
    if (IsValid(CameraActor))
    {
        const auto CameraComp = CameraActor->GetCameraComponent();
        if (CameraComp)
        {
            TrackTransform = CameraActor->GetTransform();
            TrackFOV = CameraComp->FieldOfView;
        }
    }
}

void UExtendedCameraComponent::Tracking_LocatorAimHandler(FTransform &TrackTransform,
                                                          const EExtendedCameraDriverMode DriverMode,
                                                          const FExtendedCameraLocatorAimInfo &Locator,
                                                          const FExtendedCameraLocatorAimInfo &Aim)
{
    if (DriverMode == EExtendedCameraDriverMode::DataAndAim ||
        DriverMode == EExtendedCameraDriverMode::LocationAndData ||
        DriverMode == EExtendedCameraDriverMode::LocAndAim ||
        DriverMode == EExtendedCameraDriverMode::Skeleton ||
        DriverMode == EExtendedCameraDriverMode::SkeletonAim ||
        DriverMode == EExtendedCameraDriverMode::SkeletonLocator)
    {
        // Uses Locs and Aims
        if (IsValid(Locator.Actor) && DriverMode != EExtendedCameraDriverMode::DataAndAim)
        {
            const auto LocatorLocation =
                GetActorTrackLocation(Locator.Actor, DriverMode, Locator.BoneName);
            TrackTransform.SetLocation(LocatorLocation);
        }

        // Uses Aims
        if (IsValid(Aim.Actor) && DriverMode != EExtendedCameraDriverMode::LocationAndData)
        {
            const auto BaseAimLocation =
                GetActorAimLocation(Aim.Actor, DriverMode, Aim.BoneName)
                .TransformPosition(Aim.Offset);

            const auto LookAt = BaseAimLocation - TrackTransform.GetLocation();
            TrackTransform.SetRotation(LookAt.ToOrientationQuat());
        }
    }
}

void UExtendedCameraComponent::TrackingHandler_Implementation(AActor *Owner, FMinimalViewInfo &DesiredView,
                                                              float DeltaTime)
{
    DECLARE_SCOPE_CYCLE_COUNTER(TEXT("TrackingHandler"), STAT_ACIExtCam_TrackingHandler, STATGROUP_ACIExtCam);

    if (FirstTrackCameraDriverMode == EExtendedCameraDriverMode::Compat && !IgnorePrimaryTrackedCamera)
    {
        // Old Version. Sets the same stuff to maintain backwards compatibility
        // Write Tracked Values if we're using it
        Tracking_CameraHandler(PrimaryTrackTransform, PrimaryTrackFOV, PrimaryTrackedCamera);
    }
    else if (FirstTrackCameraDriverMode == EExtendedCameraDriverMode::DataDriven)
    {
        // Do Nothing? Data is being directly driven
    }
    else if (FirstTrackCameraDriverMode == EExtendedCameraDriverMode::ReferenceCameraDriven)
    {
        // New way to specify that we wish to track a camera directly
        Tracking_CameraHandler(PrimaryTrackTransform, PrimaryTrackFOV, PrimaryTrackedCamera);
    }
    else if (FirstTrackCameraDriverMode == EExtendedCameraDriverMode::DataAndAim ||
             FirstTrackCameraDriverMode == EExtendedCameraDriverMode::LocationAndData ||
             FirstTrackCameraDriverMode == EExtendedCameraDriverMode::LocAndAim ||
             FirstTrackCameraDriverMode == EExtendedCameraDriverMode::Skeleton ||
             FirstTrackCameraDriverMode == EExtendedCameraDriverMode::SkeletonAim ||
             FirstTrackCameraDriverMode == EExtendedCameraDriverMode::SkeletonLocator)
    {
        FExtendedCameraLocatorAimInfo Locator = {PrimaryTrackLocator, PrimaryLocatorBoneName, FVector()};
        FExtendedCameraLocatorAimInfo Aim = {PrimaryTrackAim, PrimaryAimBoneName, PrimaryTrackAimOffset};
        Tracking_LocatorAimHandler(PrimaryTrackTransform, FirstTrackCameraDriverMode, Locator, Aim);

        // Handle blended Rotation
        if (PrimaryTrackAimInterpolationSpeed > 0.f)
        {
            const auto FinalRotation = FMath::QInterpTo(PrimaryTrackPastFrameLookAt,
                                                        PrimaryTrackTransform.GetRotation(), DeltaTime,
                                                        PrimaryTrackAimInterpolationSpeed);
            PrimaryTrackPastFrameLookAt = FinalRotation;
            PrimaryTrackTransform.SetRotation(FinalRotation);
        }

#if EXTENDEDCAMERA_DEBUG_DRAW
        if (PrimaryTrackAimDebug)
        {
            const auto BaseAimLocation =
                GetActorAimLocation(PrimaryTrackAim, FirstTrackCameraDriverMode, PrimaryAimBoneName)
                .TransformPosition(PrimaryTrackAimOffset);
            DrawDebugSolidBox(GetWorld(), BaseAimLocation, FVector(12.f), FColor(200, 200, 32, 128));
            DrawDebugBox(GetWorld(), BaseAimLocation, FVector(12.f), FColor::Black);
        }
#endif // EXTENDEDCAMERA_DEBUG_DRAW
    }

    // Second
    if (SecondTrackCameraDriverMode == EExtendedCameraDriverMode::Compat && !IgnoreSecondTrackedCamera)
    {
        Tracking_CameraHandler(SecondaryTrackTransform, SecondaryTrackFOV, SecondaryTrackedCamera);
    }
    else if (SecondTrackCameraDriverMode == EExtendedCameraDriverMode::DataDriven)
    {
        // Do Nothing? Data is being directly driven
    }
    else if (SecondTrackCameraDriverMode == EExtendedCameraDriverMode::ReferenceCameraDriven)
    {
        // Write Tracked Values if we're using it
        Tracking_CameraHandler(SecondaryTrackTransform, SecondaryTrackFOV, SecondaryTrackedCamera);
    }
    else if (SecondTrackCameraDriverMode == EExtendedCameraDriverMode::DataAndAim ||
             SecondTrackCameraDriverMode == EExtendedCameraDriverMode::LocationAndData ||
             SecondTrackCameraDriverMode == EExtendedCameraDriverMode::LocAndAim ||
             SecondTrackCameraDriverMode == EExtendedCameraDriverMode::Skeleton ||
             SecondTrackCameraDriverMode == EExtendedCameraDriverMode::SkeletonAim ||
             SecondTrackCameraDriverMode == EExtendedCameraDriverMode::SkeletonLocator)
    {
        FExtendedCameraLocatorAimInfo Locator = {SecondaryTrackLocator, SecondaryLocatorBoneName, FVector()};
        FExtendedCameraLocatorAimInfo Aim = {SecondaryTrackAim, SecondaryAimBoneName, SecondaryTrackAimOffset};
        Tracking_LocatorAimHandler(SecondaryTrackTransform, SecondTrackCameraDriverMode, Locator, Aim);

        // Handle blended Rotation
        if (SecondaryTrackAimInterpolationSpeed > 0.f)
        {
            const auto FinalRotation = FMath::QInterpTo(SecondaryTrackPastFrameLookAt,
                                                        SecondaryTrackTransform.GetRotation(), DeltaTime,
                                                        SecondaryTrackAimInterpolationSpeed);
            SecondaryTrackPastFrameLookAt = FinalRotation;
            SecondaryTrackTransform.SetRotation(FinalRotation);
        }

#if EXTENDEDCAMERA_DEBUG_DRAW
        if (SecondaryTrackAimDebug)
        {
            const auto BaseAimLocation =
                GetActorAimLocation(SecondaryTrackAim, SecondTrackCameraDriverMode, SecondaryAimBoneName)
                .TransformPosition(SecondaryTrackAimOffset);
            DrawDebugSolidBox(GetWorld(), BaseAimLocation, FVector(12.f), FColor(32, 200, 200, 128));
            DrawDebugBox(GetWorld(), BaseAimLocation, FVector(12.f), FColor::Green);
        }
#endif // EXTENDEDCAMERA_DEBUG_DRAW
    }
}

UExtendedCameraComponent::UExtendedCameraComponent()
    : IsLOSBlocked(false)
    , StoredLOSFOV(0)
    , SecondTrackDollyZoomReferenceDistance(0)
    , SecondTrackDollyZoomEnabled(false)
    , SecondTrackDollyZoomDistanceLiveUpdate(false)
    , FirstTrackDollyZoomReferenceDistance(0)
    , FirstTrackDollyZoomEnabled(false)
    , FirstTrackDollyZoomDistanceLiveUpdate(false)
    , PrimaryTrackAimInterpolationSpeed(0)
    , PrimaryTrackAimDebug(false)
    , SecondaryTrackAimInterpolationSpeed(0)
    , SecondaryTrackAimDebug(false)
    , IgnorePrimaryTrackedCamera(false)
    , IgnoreSecondTrackedCamera(false)
    , CameraPrimaryTrackBlendAlpha(0)
    , CameraSecondaryTrackBlendAlpha(0)
    , CameraLOSMode(EExtendedCameraMode::Ignore)
    , FOVCheckOffsetInRadians(0)
    , UseDollyZoomForLOS(false)
    , SmoothReturnOnLineOfSight(false)
    , SmoothReturnSpeed(1)
    , ReturnFinishedThresholdSquared(27.f)
    , WasLineOfSightBlockedRecently(false)
    , FirstTrackCameraDriverMode(EExtendedCameraDriverMode::Compat)
    , SecondTrackCameraDriverMode(EExtendedCameraDriverMode::Compat)
    , FirstTrackOrbitalMode(EExtendedCameraOrbitMode::LinearInterpolate)
    , SecondTrackOrbitalMode(EExtendedCameraOrbitMode::LinearInterpolate)
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

void UExtendedCameraComponent::SetCameraPrimaryTrack(FVector &InLocation, FRotator &InRotation, float InFOV)
{
    PrimaryTrackTransform.SetLocation(InLocation);
    PrimaryTrackTransform.SetRotation(InRotation.Quaternion());
    PrimaryTrackFOV = InFOV;
}

void UExtendedCameraComponent::SetCameraSecondaryTrack(FVector &InLocation, FRotator &InRotation, float InFOV)
{
    SecondaryTrackTransform.SetLocation(InLocation);
    SecondaryTrackTransform.SetRotation(InRotation.Quaternion());
    SecondaryTrackFOV = InFOV;
}

void UExtendedCameraComponent::SetCameraPrimaryTransform(FTransform &InTransform, float InFOV)
{
    PrimaryTrackTransform = InTransform;
    PrimaryTrackFOV = InFOV;
}

void UExtendedCameraComponent::SetCameraSecondaryTransform(FTransform &InTransform, float InFOV)
{
    SecondaryTrackTransform = InTransform;
    SecondaryTrackFOV = InFOV;
}

void UExtendedCameraComponent::SetCameraPrimaryLocationRotation(FVector &InLocation, FRotator &InRotation)
{
    PrimaryTrackTransform.SetLocation(InLocation);
    PrimaryTrackTransform.SetRotation(InRotation.Quaternion());
}

void UExtendedCameraComponent::SetCameraSecondaryLocationRotation(FVector &InLocation, FRotator &InRotation)
{
    SecondaryTrackTransform.SetLocation(InLocation);
    SecondaryTrackTransform.SetRotation(InRotation.Quaternion());
}

void UExtendedCameraComponent::SetCameraPrimaryRotation(FRotator &InRotation)
{
    PrimaryTrackTransform.SetRotation(InRotation.Quaternion());
}

void UExtendedCameraComponent::SetCameraSecondaryRotation(FRotator &InRotation)
{
    SecondaryTrackTransform.SetRotation(InRotation.Quaternion());
}

void UExtendedCameraComponent::SetCameraPrimaryLocation(FVector &InLocation)
{
    PrimaryTrackTransform.SetLocation(InLocation);
}

void UExtendedCameraComponent::SetCameraSecondaryLocation(FVector &InLocation)
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

void UExtendedCameraComponent::SetPrimaryTrackDollyZoomReferenceDistance(float Distance)
{
    FirstTrackDollyZoomReferenceDistance = Distance;
}

void UExtendedCameraComponent::SetSecondaryTrackDollyZoomReferenceDistance(float Distance)
{
    SecondTrackDollyZoomReferenceDistance = Distance;
}

void UExtendedCameraComponent::SetPrimaryTrackDollyZoomEnabled(bool Enabled)
{
    FirstTrackDollyZoomEnabled = Enabled;
}

void UExtendedCameraComponent::SetSecondaryTrackDollyZoomEnabled(bool Enabled)
{
    SecondTrackDollyZoomEnabled = Enabled;
}

void UExtendedCameraComponent::SetPrimaryTrackDollyZoomLiveUpdate(bool Enabled)
{
    FirstTrackDollyZoomDistanceLiveUpdate = Enabled;
}

void UExtendedCameraComponent::SetSecondaryTrackDollyZoomLiveUpdate(bool Enabled)
{
    SecondTrackDollyZoomDistanceLiveUpdate = Enabled;
}

void UExtendedCameraComponent::SetPrimaryTrackedCamera(ACameraActor *TrackedCamera)
{
    PrimaryTrackedCamera = TrackedCamera;
}

void UExtendedCameraComponent::SetSecondaryTrackedCamera(ACameraActor *TrackedCamera)
{
    SecondaryTrackedCamera = TrackedCamera;
}

void UExtendedCameraComponent::SetPrimaryTrackLocator(AActor *TrackedActor)
{
    PrimaryTrackLocator = TrackedActor;
}

void UExtendedCameraComponent::SetSecondaryTrackLocator(AActor *TrackedActor)
{
    SecondaryTrackLocator = TrackedActor;
}

void UExtendedCameraComponent::SetPrimaryTrackAim(AActor *TrackedActor)
{
    PrimaryTrackLocator = TrackedActor;
}

void UExtendedCameraComponent::SetSecondaryTrackAim(AActor *TrackedActor)
{
    SecondaryTrackAim = TrackedActor;
}

void UExtendedCameraComponent::SetPrimaryTrackAimInterpolationSpeed(float Speed)
{
    PrimaryTrackAimInterpolationSpeed = Speed;
}

void UExtendedCameraComponent::SetSecondaryTrackAimInterpolationSpeed(float Speed)
{
    SecondaryTrackAimInterpolationSpeed = Speed;
}

void UExtendedCameraComponent::SetSecondaryTrackAimOffset(FVector &AimOffset)
{
    SecondaryTrackAimOffset = AimOffset;
}

void UExtendedCameraComponent::SetPrimaryTrackAimOffset(FVector &AimOffset)
{
    PrimaryTrackAimOffset = AimOffset;
}

void UExtendedCameraComponent::KeepInFrameLineOfSight_Implementation(AActor *Owner, FMinimalViewInfo &DesiredView)
{
    CommonKeepLineOfSight(Owner, DesiredView);
}

void UExtendedCameraComponent::KeepAnyLineOfSight_Implementation(AActor *Owner, FMinimalViewInfo &DesiredView)
{
    CommonKeepLineOfSight(Owner, DesiredView);
}

void UExtendedCameraComponent::CommonKeepLineOfSight_Implementation(AActor *Owner, FMinimalViewInfo &DesiredView)
{
    auto World = GetWorld();

    if (World)
    {
        // Cast from owner to pawn
        FCollisionQueryParams params{};
        params.AddIgnoredActor(Owner);
        FHitResult LOSCheck{};

        // Owner Location is assumed to be aim. It's not always though. So we need to get the aim
        auto Aim = GetAimLocation(Owner);

        World->LineTraceSingleByChannel(LOSCheck, Aim, DesiredView.Location, this->GetCollisionObjectType(), params);

#if EXTENDEDCAMERA_DEBUG_DRAW
        if (PrimaryTrackAimDebug || SecondaryTrackAimDebug)
        {
            DrawDebugLine(GetWorld(), Aim, DesiredView.Location, FColor::Red);

            DrawDebugSolidBox(GetWorld(), Aim, FVector(12.f), FColor(200, 20, 132, 128));
            DrawDebugBox(GetWorld(), Aim, FVector(12.f), FColor::Black);
        }
#endif // EXTENDEDCAMERA_DEBUG_DRAW

        if (LOSCheck.bBlockingHit)
        {
            if (!IsLOSBlocked)
            {
                StoredLOSFOV = DesiredView.FOV;
            }

            if (UseDollyZoomForLOS)
            {
                DollyZoom(Owner, DesiredView, LOSCheck);
            }

            // Must be done after dollyzoom, otherwise we'll lerp nothing
            DesiredView.Location = LOSCheck.ImpactPoint; // + LOSCheck.ImpactNormal * 0.1f;

            // Update to reflect the offset position of the camera
            // We need it later when the hit is not blocked
            if (SmoothReturnOnLineOfSight)
            {
                // This gets set back to false when the lerp ends
                WasLineOfSightBlockedRecently = true;
                StoredPreviousLocationForReturn = LOSCheck.ImpactPoint;
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

void UExtendedCameraComponent::DollyZoom(AActor *Owner, FMinimalViewInfo &DesiredView, FHitResult &LOSCheck)
{

    // Compute the theta now. This is just FOV at distance X
    // const auto CurrentTheta = FMath::DegreesToRadians(DesiredView.FOV * 0.5f);
    const auto DVL = DesiredView.Location;
    const auto OAL = Owner->GetActorLocation();
    const auto LIP = LOSCheck.ImpactPoint;
    // const auto CurrentDistanceSQ = FVector::DistSquared(OAL, DVL);
    // const auto NewDistanceSQ = FVector::DistSquared(OAL, LIP);
    // const auto DistanceRatio = FMath::Sqrt(CurrentDistanceSQ / NewDistanceSQ);

    // DesiredView.FOV = 2 * FMath::RadiansToDegrees(FMath::Atan((FMath::Tan(CurrentTheta) * FVector::Dist(OAL, DVL) /
    // FVector::Dist(OAL, LIP)))); DesiredView.FOV = 2 * FMath::RadiansToDegrees(FMath::Atan(FMath::Tan(CurrentTheta) *
    // DistanceRatio));
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

void UExtendedCameraComponent::GetCameraView(float DeltaTime, FMinimalViewInfo &DesiredView)
{
    // Start a counter here so it captures the super call
    DECLARE_SCOPE_CYCLE_COUNTER(TEXT("GetCameraView (Including Super::)"), STAT_ACIGetCameraViewInc,
                                STATGROUP_ACIExtCam);

    // Parent Call
    Super::GetCameraView(DeltaTime, DesiredView);

    // Start a second counter that excludes the parent view update
    DECLARE_SCOPE_CYCLE_COUNTER(TEXT("GetCameraView (Excluding Super::)"), STAT_ACIGetCameraViewExc,
                                STATGROUP_ACIExtCam);

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
        if (FirstTrackDollyZoomEnabled)
        {
            if (FirstTrackDollyZoomDistanceLiveUpdate)
            {
                FirstTrackDollyZoomReferenceDistance =
                    FVector::Dist(ComponentOwner->GetActorLocation(), PrimaryTrackTransform.GetLocation());
            }

            OffsetTrackFOV =
                DollyZoom(FirstTrackDollyZoomReferenceDistance, SecondaryTrackFOV,
                          FVector::Dist(ComponentOwner->GetActorLocation(), PrimaryTrackTransform.GetLocation()));
        }

        if (GetUsePrimaryTrack())
        {
            DesiredView.Location = PrimaryTrackTransform.GetLocation();
            DesiredView.Rotation = PrimaryTrackTransform.Rotator();
            DesiredView.FOV = OffsetTrackFOV;
        }
        else
        {
            const FExtendedCameraViewInfo InView = {PrimaryTrackTransform.GetLocation(),
                                                    PrimaryTrackTransform.GetRotation(), OffsetTrackFOV};
            const auto OutView = BlendTrackViews(DesiredView, InView, CameraPrimaryTrackBlendAlpha,
                                                 FirstTrackOrbitalMode);
            DesiredView.Location = OutView.Location;
            DesiredView.Rotation = OutView.Rotation.Rotator();
            DesiredView.FOV = OutView.FOV;
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
        if (SecondTrackDollyZoomEnabled)
        {
            if (SecondTrackDollyZoomDistanceLiveUpdate)
            {
                SecondTrackDollyZoomReferenceDistance =
                    FVector::Dist(ComponentOwner->GetActorLocation(), SecondaryTrackTransform.GetLocation());
            }

            OffsetTrackFOV =
                DollyZoom(SecondTrackDollyZoomReferenceDistance, SecondaryTrackFOV,
                          FVector::Dist(ComponentOwner->GetActorLocation(), SecondaryTrackTransform.GetLocation()));
        }

        // Are we fully blended to the secondary track
        if (GetUseSecondaryTrack())
        {
            DesiredView.Location = SecondaryTrackTransform.GetLocation();
            DesiredView.Rotation = SecondaryTrackTransform.Rotator();
            DesiredView.FOV = OffsetTrackFOV;
        }

        // We need to blend!
        else
        {
            const FExtendedCameraViewInfo InView = {SecondaryTrackTransform.GetLocation(),
                                                    SecondaryTrackTransform.GetRotation(), OffsetTrackFOV};
            const auto OutView = BlendTrackViews(DesiredView, InView, CameraSecondaryTrackBlendAlpha,
                                                 SecondTrackOrbitalMode);
            DesiredView.Location = OutView.Location;
            DesiredView.Rotation = OutView.Rotation.Rotator();
            DesiredView.FOV = OutView.FOV;
        }
    }

    // Now LOS
    LineOfCheckHandler(ComponentOwner, DesiredView);

    // Do SmoothReturn first, otherwise we can push the camera back out of bounds
    SmoothReturn(ComponentOwner, DesiredView, DeltaTime);
}

void UExtendedCameraComponent::BeginPlay()
{
    Super::BeginPlay();

    // Set up our temporary variables here
    PrimaryTrackPastFrameLookAt = PrimaryTrackTransform.GetRotation();
    SecondaryTrackPastFrameLookAt = SecondaryTrackTransform.GetRotation();
}

void UExtendedCameraComponent::SetCameraPrimaryTrack(FVector &&InLocation, FRotator &&InRotation, float InFOV)
{
    PrimaryTrackTransform.SetLocation(MoveTemp(InLocation));
    PrimaryTrackTransform.SetRotation(MoveTemp(InRotation).Quaternion());
    PrimaryTrackFOV = InFOV;
}

void UExtendedCameraComponent::SetCameraSecondaryTrack(FVector &&InLocation, FRotator &&InRotation, float InFOV)
{
    SecondaryTrackTransform.SetLocation(MoveTemp(InLocation));
    SecondaryTrackTransform.SetRotation(MoveTemp(InRotation).Quaternion());
    SecondaryTrackFOV = InFOV;
}

void UExtendedCameraComponent::SetCameraPrimaryLocationRotation(FVector &&InLocation, FRotator &&InRotation)
{
    PrimaryTrackTransform.SetLocation(MoveTemp(InLocation));
    PrimaryTrackTransform.SetRotation(MoveTemp(InRotation).Quaternion());
}

void UExtendedCameraComponent::SetCameraSecondaryLocationRotation(FVector &&InLocation, FRotator &&InRotation)
{
    SecondaryTrackTransform.SetLocation(MoveTemp(InLocation));
    SecondaryTrackTransform.SetRotation(MoveTemp(InRotation).Quaternion());
}

void UExtendedCameraComponent::SetCameraPrimaryRotation(FRotator &&InRotation)
{
    PrimaryTrackTransform.SetRotation(MoveTemp(InRotation).Quaternion());
}

void UExtendedCameraComponent::SetCameraSecondaryRotation(FRotator &&InRotation)
{
    SecondaryTrackTransform.SetRotation(MoveTemp(InRotation).Quaternion());
}

void UExtendedCameraComponent::SetCameraPrimaryLocation(FVector &&InLocation)
{
    PrimaryTrackTransform.SetLocation(MoveTemp(InLocation));
}

void UExtendedCameraComponent::SetCameraSecondaryLocation(FVector &&InLocation)
{
    SecondaryTrackTransform.SetLocation(MoveTemp(InLocation));
}

bool UExtendedCameraComponent::SetPrimaryLocatorBoneName(FName TrackedBoneName)
{
    PrimaryLocatorBoneName = TrackedBoneName;

    if (FirstTrackCameraDriverMode == EExtendedCameraDriverMode::SkeletonLocator ||
        FirstTrackCameraDriverMode == EExtendedCameraDriverMode::Skeleton)
    {
        return BoneCheck(PrimaryTrackLocator, TrackedBoneName);
    }

    return false;
}

bool UExtendedCameraComponent::SetSecondaryLocatorBoneName(FName TrackedBoneName)
{
    SecondaryLocatorBoneName = TrackedBoneName;

    if (SecondTrackCameraDriverMode == EExtendedCameraDriverMode::SkeletonLocator ||
        SecondTrackCameraDriverMode == EExtendedCameraDriverMode::Skeleton)
    {
        return BoneCheck(SecondaryTrackLocator, TrackedBoneName);
    }

    return false;
}

bool UExtendedCameraComponent::SetPrimaryLocatorAimName(FName TrackedAimName)
{
    PrimaryAimBoneName = TrackedAimName;

    if (FirstTrackCameraDriverMode == EExtendedCameraDriverMode::SkeletonAim ||
        FirstTrackCameraDriverMode == EExtendedCameraDriverMode::Skeleton)
    {
        return BoneCheck(PrimaryTrackAim, TrackedAimName);
    }

    return false;
}

bool UExtendedCameraComponent::SetSecondaryLocatorAimName(FName TrackedAimName)
{
    SecondaryAimBoneName = TrackedAimName;

    if (SecondTrackCameraDriverMode == EExtendedCameraDriverMode::SkeletonAim ||
        SecondTrackCameraDriverMode == EExtendedCameraDriverMode::Skeleton)
    {
        return BoneCheck(SecondaryTrackAim, TrackedAimName);
    }

    return false;
}

void UExtendedCameraComponent::SetFOVCheckOffsetInRadians(float FOVOffset)
{
    FOVCheckOffsetInRadians = FOVOffset;
}

void UExtendedCameraComponent::SetUseDollyZoom(bool NewState)
{
    UseDollyZoomForLOS = NewState;
}

void UExtendedCameraComponent::SetSmoothReturn(bool NewState)
{
    SmoothReturnOnLineOfSight = NewState;
}

void UExtendedCameraComponent::SetSmoothReturnSpeed(float NewReturnSpeed)
{
    SmoothReturnSpeed = NewReturnSpeed;
}

void UExtendedCameraComponent::SetSmoothReturnDeadzone(float NewDeadzone)
{
    ReturnFinishedThresholdSquared = NewDeadzone * NewDeadzone;
}

void UExtendedCameraComponent::SetPrimaryTrackMode(EExtendedCameraDriverMode NewMode)
{
    FirstTrackCameraDriverMode = NewMode;
}

void UExtendedCameraComponent::SetSecondaryTrackMode(EExtendedCameraDriverMode NewMode)
{
    SecondTrackCameraDriverMode = NewMode;
}

void UExtendedCameraComponent::SetPrimaryTrackAimDebug(bool Enabled)
{
#if EXTENDEDCAMERA_DEBUG_DRAW
    PrimaryTrackAimDebug = Enabled;
#endif // EXTENDEDCAMERA_DEBUG_DRAW
}

void UExtendedCameraComponent::SetSecondaryTrackAimDebug(bool Enabled)
{
#if EXTENDEDCAMERA_DEBUG_DRAW
    SecondaryTrackAimDebug = Enabled;
#endif // EXTENDEDCAMERA_DEBUG_DRAW
}
