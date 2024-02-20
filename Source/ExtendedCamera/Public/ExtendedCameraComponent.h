// Copyright Acinonyx Ltd. 2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ExtendedCameraTypes.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"

#include "ExtendedCameraComponent.generated.h"

// Fix for v4.27
#ifndef EXTENDEDCAMERA_DEBUG_DRAW
#define EXTENDEDCAMERA_DEBUG_DRAW (!(UE_BUILD_SHIPPING || UE_BUILD_TEST) || WITH_EDITOR)
#endif

DECLARE_STATS_GROUP(TEXT("Acinonyx Extended Camera"), STATGROUP_ACIExtCam, STATCAT_Advanced);

UENUM(BlueprintType)
enum EExtendedCameraMode
{
    Ignore UMETA(DisplayName = "Ignore"),
    KeepLos UMETA(DisplayName = "Keep LOS to Owner in Frame"),
    KeepLosNoDot UMETA(DisplayName = "Always Keep Line of Sight"),
    KeepLosWithinLimit UMETA(DisplayName = "Use FOV Offset as Limit"),
    TOTAL_CAMERA_MODES UMETA(Hidden)
};

UENUM(BlueprintType)
enum EExtendedCameraDriverMode
{
    ReferenceCameraDriven UMETA(DisplayName = "Reference Camera"),
    Compat UMETA(DisplayName = "Compatibility Mode"),
    DataDriven UMETA(DisplayName = "Direct Data Driven"),
    DataAndAim UMETA(DisplayName = "Data Driven Location with Aim"),
    LocationAndData UMETA(DisplayName = "Location with Data Driven Aim"),
    LocAndAim UMETA(DisplayName = "Object Location and Aim"),
    Skeleton UMETA(DisplayName = "Skeletal Locator and Aim"),
    SkeletonLocator UMETA(DisplayName = "Skeletal Locator and Object Aim"),
    SkeletonAim UMETA(DisplayName = "Object Locator and Skeletal Aim"),
    DataAndSkeletalAim UMETA(DisplayName = "Data Driven Location with Skeletal Aim"),
    SkeletalLocationAndData UMETA(DisplayName = "Skeletal Location with Data Driven Aim"),

    TOTAL_CAMERA_DRIVER_MODES UMETA(Hidden)
};

UENUM(BlueprintType)
enum EExtendedCameraOrbitMode
{
    LinearInterpolate UMETA(DisplayName = "Linear Interpolate"),
    SphericalInterpolate UMETA(DisplayName = "Spherical Interpolate"),

    TOTAL_CAMERA_ORBIT_MODES UMETA(Hidden)
};

UCLASS(config = Game, BlueprintType, Blueprintable, ClassGroup = Camera, meta = (BlueprintSpawnableComponent))
class EXTENDEDCAMERA_API UExtendedCameraComponent : public UCameraComponent
{
    GENERATED_BODY()

protected:
    // DollyZoom
    UPROPERTY(SaveGame)
    bool IsLOSBlocked;

    UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Extended Camera")
    float StoredLOSFOV;

    ///// ///// ////////// ///// /////
    // Dolly Zooms
    //

    /**
     * Dolly Zoom Reference Distance
     *
     * This variable is used to apply the FOV neutrally to the camera. At this
     * distance, the FOV is the set FOV in the camera.
     *
     * This feature only functions when SecondTrackDollyZoomEnabled is true,
     * and this variable will automatically update when
     * SecondTrackDollyZoomDistanceLiveUpdate is true
     */
    UPROPERTY(SaveGame, Interp, EditAnywhere, BlueprintReadWrite, Category = "Extended Camera|Second Track|Dolly Zoom")
    float SecondTrackDollyZoomReferenceDistance;

    /**
     * Dolly Zoom Enabled for Second Track
     *
     * This enables the Dolly zoom for the second track
     * See SecondTrackDollyZoomReferenceDistance for more information
     */
    UPROPERTY(SaveGame, Interp, EditAnywhere, BlueprintReadWrite, Category = "Extended Camera|Second Track|Dolly Zoom")
    bool SecondTrackDollyZoomEnabled;

    /**
     * Dolly Zoom Live Update Enabled for Second Track
     *
     * This enables the live update of the reference distance
     * See SecondTrackDollyZoomReferenceDistance for more information
     */
    UPROPERTY(SaveGame, Interp, EditAnywhere, BlueprintReadWrite, Category = "Extended Camera|Second Track|Dolly Zoom")
    bool SecondTrackDollyZoomDistanceLiveUpdate;

    /**
     * Dolly Zoom Reference Distance
     *
     * This variable is used to apply the FOV neutrally to the camera. At this
     * distance, the FOV is the set FOV in the camera.
     *
     * This feature only functions when SecondTrackDollyZoomEnabled is true,
     * and this variable will automatically update when
     * SecondTrackDollyZoomDistanceLiveUpdate is true
     */
    UPROPERTY(SaveGame, Interp, EditAnywhere, BlueprintReadWrite, Category = "Extended Camera|First Track|Dolly Zoom")
    float FirstTrackDollyZoomReferenceDistance;

    /**
     * Dolly Zoom Enabled for Second Track
     *
     * This enables the Dolly zoom for the second track
     * See SecondTrackDollyZoomReferenceDistance for more information
     */
    UPROPERTY(SaveGame, Interp, EditAnywhere, BlueprintReadWrite, Category = "Extended Camera|First Track|Dolly Zoom")
    bool FirstTrackDollyZoomEnabled;

    /**
     * Dolly Zoom Live Update Enabled for Second Track
     *
     * This enables the live update of the reference distance
     * See SecondTrackDollyZoomReferenceDistance for more information
     */
    UPROPERTY(SaveGame, Interp, EditAnywhere, BlueprintReadWrite, Category = "Extended Camera|First Track|Dolly Zoom")
    bool FirstTrackDollyZoomDistanceLiveUpdate;

    ///// ///// ////////// ///// /////
    // Extended Camera Blend Point
    //

    // Target Camera
    UPROPERTY(SaveGame, Interp, EditAnywhere, BlueprintReadWrite,
        Category = "Extended Camera|First Track|Reference Camera")
    TObjectPtr<ACameraActor> PrimaryTrackedCamera;

    // Target Camera
    UPROPERTY(SaveGame, Interp, EditAnywhere, BlueprintReadWrite,
        Category = "Extended Camera|Second Track|Reference Camera")
    TObjectPtr<ACameraActor> SecondaryTrackedCamera;

    UPROPERTY(SaveGame, Interp, EditAnywhere, BlueprintReadWrite, Category = "Extended Camera|First Track|Locator")
    TObjectPtr<AActor> PrimaryTrackLocator;

    UPROPERTY(SaveGame, Interp, EditAnywhere, BlueprintReadWrite, Category = "Extended Camera|First Track|Locator")
    TObjectPtr<AActor> PrimaryTrackAim;

    UPROPERTY(SaveGame, Interp, EditAnywhere, BlueprintReadWrite, Category = "Extended Camera|First Track|Locator")
    FVector PrimaryTrackAimOffset;

    UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Extended Camera|First Track|Locator")
    FQuat PrimaryTrackPastFrameLookAt;

    UPROPERTY(SaveGame, Interp, EditAnywhere, BlueprintReadWrite, Category = "Extended Camera|First Track|Locator")
    float PrimaryTrackAimInterpolationSpeed;

    //#if EXTENDEDCAMERA_DEBUG_DRAW
    UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, Category = "Extended Camera|First Track|Locator")
    bool PrimaryTrackAimDebug;
    //#endif // EXTENDEDCAMERA_DEBUG_DRAW

    UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, Category = "Extended Camera|First Track|Locator")
    FName PrimaryLocatorBoneName;

    UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, Category = "Extended Camera|First Track|Locator")
    FName PrimaryAimBoneName;

    UPROPERTY(SaveGame, Interp, EditAnywhere, BlueprintReadWrite, Category = "Extended Camera|Second Track|Locator")
    TObjectPtr<AActor> SecondaryTrackLocator;

    UPROPERTY(SaveGame, Interp, EditAnywhere, BlueprintReadWrite, Category = "Extended Camera|Second Track|Locator")
    TObjectPtr<AActor> SecondaryTrackAim;

    UPROPERTY(SaveGame, Interp, EditAnywhere, BlueprintReadWrite, Category = "Extended Camera|Second Track|Locator")
    FVector SecondaryTrackAimOffset;

    UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Extended Camera|Second Track|Locator")
    FQuat SecondaryTrackPastFrameLookAt;

    UPROPERTY(SaveGame, Interp, EditAnywhere, BlueprintReadWrite, Category = "Extended Camera|Second Track|Locator")
    float SecondaryTrackAimInterpolationSpeed;

    //#if EXTENDEDCAMERA_DEBUG_DRAW
    UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, Category = "Extended Camera|Second Track|Locator")
    bool SecondaryTrackAimDebug;
    //#endif // EXTENDEDCAMERA_DEBUG_DRAW

    UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, Category = "Extended Camera|Second Track|Locator")
    FName SecondaryLocatorBoneName;

    UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, Category = "Extended Camera|Second Track|Locator")
    FName SecondaryAimBoneName;

    /**
     * You can either null the TrackedCamera or -- and this is easier in sequencer -- you can disable it here
     */
    UPROPERTY(SaveGame, Interp, EditAnywhere, BlueprintReadWrite, Category = "Extended Camera")
    bool IgnorePrimaryTrackedCamera;

    UPROPERTY(SaveGame, Interp, EditAnywhere, BlueprintReadWrite, Category = "Extended Camera")
    bool IgnoreSecondTrackedCamera;

    // Primary Track - Set by users
    UPROPERTY(SaveGame, Interp, EditAnywhere, BlueprintReadWrite, Category = "Extended Camera")
    FTransform PrimaryTrackTransform;

    /**
     * Primary FOV
     * Zero disables FOV blending
     */
    UPROPERTY(SaveGame, Interp, EditAnywhere, BlueprintReadWrite, Category = "Extended Camera",
        meta = (UIMin = "0.0", UIMax = "175", ClampMin = "0.0", ClampMax = "360.0", Units = deg))
    float PrimaryTrackFOV = 0.f;

    // Secondary Track - Set by users
    UPROPERTY(SaveGame, Interp, EditAnywhere, BlueprintReadWrite, Category = "Extended Camera")
    FTransform SecondaryTrackTransform;

    /**
     * Secondary FOV
     * Zero disables FOV blending
     */
    UPROPERTY(SaveGame, Interp, EditAnywhere, BlueprintReadWrite, Category = "Extended Camera",
        meta = (UIMin = "0.0", UIMax = "175", ClampMin = "0.0", ClampMax = "360.0", Units = deg))
    float SecondaryTrackFOV = 0.f;

    // Blend Amount for the first channel
    UPROPERTY(SaveGame, Interp, Category = "Extended Camera")
    float CameraPrimaryTrackBlendAlpha;

    // Blend Amount
    UPROPERTY(SaveGame, Interp, Category = "Extended Camera")
    float CameraSecondaryTrackBlendAlpha;

    // LOS Mode
    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Extended Camera")
    TEnumAsByte<EExtendedCameraMode> CameraLOSMode;

    // Small Offset for FOV checks
    UPROPERTY(SaveGame, Interp, EditAnywhere, BlueprintReadWrite, Category = "Extended Camera")
    float FOVCheckOffsetInRadians;

    // DollyZoom for LOS Modes
    UPROPERTY(SaveGame, Interp, EditAnywhere, BlueprintReadWrite, Category = "Extended Camera|Line of Sight")
    bool UseDollyZoomForLOS;

    ///// ///// ////////// ///// /////
    // Smooth Return
    //

    // Enables and disables SmoothReturn
    UPROPERTY(SaveGame, Interp, EditAnywhere, BlueprintReadWrite, Category = "Extended Camera|Smooth Return")
    bool SmoothReturnOnLineOfSight;

    // Stored Location
    UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Extended Camera|Smooth Return")
    FVector StoredPreviousLocationForReturn;

    UPROPERTY(SaveGame, Interp, EditAnywhere, BlueprintReadWrite, Category = "Extended Camera|Smooth Return")
    float SmoothReturnSpeed;

    /**
     * Smooth Return Finished Threshold
     *
     * Value to check when the return has been completed and we can jump to live
     * This helps when the blend is not very aggressive as you can get a bit of lag
     * while the player keeps moving
     */
    UPROPERTY(SaveGame, Interp, EditAnywhere, BlueprintReadWrite, Category = "Extended Camera|Smooth Return")
    float ReturnFinishedThresholdSquared;

    UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Extended Camera|Smooth Return")
    bool WasLineOfSightBlockedRecently;

    ///// ///// ////////// ///// /////
    // Driver Modes
    //

    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Extended Camera|First Track")
    TEnumAsByte<EExtendedCameraDriverMode> FirstTrackCameraDriverMode;

    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Extended Camera|Second Track")
    TEnumAsByte<EExtendedCameraDriverMode> SecondTrackCameraDriverMode;

protected:
    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Extended Camera|First Track")
    TEnumAsByte<EExtendedCameraOrbitMode> FirstTrackOrbitalMode;

    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Extended Camera|Second Track")
    TEnumAsByte<EExtendedCameraOrbitMode> SecondTrackOrbitalMode;

    UFUNCTION()
    FExtendedCameraViewInfo BlendTrackViews(const FMinimalViewInfo &FromView, const FExtendedCameraViewInfo &ToView,
                                            const float Alpha,
                                            const TEnumAsByte<EExtendedCameraOrbitMode> RequestedOrbitMode);
    //virtual FExtendedCameraViewInfo BlendTrackViews_Implementation(const FMinimalViewInfo& FromView, const FExtendedCameraViewInfo& ToView, const float Alpha, const TEnumAsByte<EExtendedCameraOrbitMode> RequestedOrbitMode);

protected:
    UFUNCTION(BlueprintNativeEvent)
    FVector GetAimLocation(AActor *Owner);
    virtual FVector GetAimLocation_Implementation(AActor *Owner);

    UFUNCTION(BlueprintNativeEvent)
    FVector GetActorTrackLocation(AActor *Owner, EExtendedCameraDriverMode CameraMode, FName LocatorBoneName);
    virtual FVector GetActorTrackLocation_Implementation(AActor *Owner, EExtendedCameraDriverMode CameraMode,
                                                         FName LocatorBoneName);

    UFUNCTION(BlueprintNativeEvent)
    FTransform GetActorAimLocation(AActor *Owner, EExtendedCameraDriverMode CameraMode, FName LocatorBoneName);
    virtual FTransform GetActorAimLocation_Implementation(AActor *Owner, EExtendedCameraDriverMode CameraMode,
                                                          FName LocatorBoneName);

    // Function to SmoothReturn
    UFUNCTION(BlueprintNativeEvent)
    void SmoothReturn(AActor *Owner, FMinimalViewInfo &DesiredView, float DeltaTime);
    virtual void SmoothReturn_Implementation(AActor *Owner, FMinimalViewInfo &DesiredView, float DeltaTime);

    // Function to wrap LOS checks
    UFUNCTION(BlueprintNativeEvent)
    void LineOfCheckHandler(AActor *Owner, FMinimalViewInfo &DesiredView);
    virtual void LineOfCheckHandler_Implementation(AActor *Owner, FMinimalViewInfo &DesiredView);


    virtual void Tracking_CameraHandler(FTransform &TrackTransform, float &TrackFOV,
                                        TObjectPtr<ACameraActor> CameraActor);
    virtual void Tracking_LocatorAimHandler(FTransform &TrackTransform, const EExtendedCameraDriverMode DriverMode,
                                            const FExtendedCameraLocatorAimInfo &Locator,
                                            const FExtendedCameraLocatorAimInfo &Aim);

    // Function to track
    UFUNCTION(BlueprintNativeEvent)
    void TrackingHandler(AActor *Owner, FMinimalViewInfo &DesiredView, float DeltaTime);
    virtual void TrackingHandler_Implementation(AActor *Owner, FMinimalViewInfo &DesiredView, float DeltaTime);

public:
    UExtendedCameraComponent();

    // Set the Blend Amount
    UFUNCTION(BlueprintCallable, Category = "Extended Camera|First Track")
    virtual void SetPrimaryCameraTrackAlpha(float Alpha);

    // Set the Blend Amount
    UFUNCTION(BlueprintCallable, Category = "Extended Camera|Second Track")
    virtual void SetSecondaryCameraTrackAlpha(float Alpha);

    // Get the Blend Amount
    UFUNCTION(BlueprintCallable, Category = "Extended Camera|First Track")
    virtual float GetPrimaryCameraTrackAlpha();

    UFUNCTION(BlueprintCallable, Category = "Extended Camera|Second Track")
    virtual float GetSecondaryCameraTrackAlpha();

    // Set Primary Track
    UFUNCTION(BlueprintCallable, Category = "Extended Camera|First Track")
    virtual void SetCameraPrimaryTrack(UPARAM(ref) FVector &InLocation, UPARAM(ref) FRotator &InRotation, float InFOV);

    // Set Secondary Track
    UFUNCTION(BlueprintCallable, Category = "Extended Camera|Second Track")
    virtual void SetCameraSecondaryTrack(UPARAM(ref) FVector &InLocation, UPARAM(ref) FRotator &InRotation,
                                         float InFOV);

    // Set Primary Track
    UFUNCTION(BlueprintCallable, Category = "Extended Camera|First Track")
    virtual void SetCameraPrimaryTransform(UPARAM(ref) FTransform &InTransform, float InFOV);

    // Set Secondary Track
    UFUNCTION(BlueprintCallable, Category = "Extended Camera|Second Track")
    virtual void SetCameraSecondaryTransform(UPARAM(ref) FTransform &InTransform, float InFOV);

    // Set Primary Track
    UFUNCTION(BlueprintCallable, Category = "Extended Camera|First Track")
    virtual void SetCameraPrimaryLocationRotation(UPARAM(ref) FVector &InLocation, UPARAM(ref) FRotator &InRotation);

    // Set Secondary Track
    UFUNCTION(BlueprintCallable, Category = "Extended Camera|Second Track")
    virtual void SetCameraSecondaryLocationRotation(UPARAM(ref) FVector &InLocation, UPARAM(ref) FRotator &InRotation);

    // Set Primary Track
    UFUNCTION(BlueprintCallable, Category = "Extended Camera|First Track")
    virtual void SetCameraPrimaryRotation(UPARAM(ref) FRotator &InRotation);

    // Set Secondary Track
    UFUNCTION(BlueprintCallable, Category = "Extended Camera|Second Track")
    virtual void SetCameraSecondaryRotation(UPARAM(ref) FRotator &InRotation);

    // Set Primary Track
    UFUNCTION(BlueprintCallable, Category = "Extended Camera|First Track")
    virtual void SetCameraPrimaryLocation(UPARAM(ref) FVector &InLocation);

    // Set Secondary Track
    UFUNCTION(BlueprintCallable, Category = "Extended Camera|Second Track")
    virtual void SetCameraSecondaryLocation(UPARAM(ref) FVector &InLocation);

    // Set Primary Track
    UFUNCTION(BlueprintCallable, Category = "Extended Camera|First Track")
    virtual void SetCameraPrimaryFOV(float InFOV);

    // Set Secondary Track
    UFUNCTION(BlueprintCallable, Category = "Extended Camera|Second Track")
    virtual void SetCameraSecondaryFOV(float InFOV);

    UFUNCTION(BlueprintCallable, Category = "Extended Camera|First Track")
    virtual bool GetUsePrimaryTrack();

    UFUNCTION(BlueprintCallable, Category = "Extended Camera|Second Track")
    virtual bool GetUseSecondaryTrack();

    UFUNCTION(BlueprintCallable, Category = "Extended Camera")
    virtual void SetCameraMode(EExtendedCameraMode NewMode);

    UFUNCTION(BlueprintCallable, Category = "Extended Camera")
    virtual TEnumAsByte<EExtendedCameraMode> GetCameraMode();

    UFUNCTION(BlueprintCallable, Category = "Extended Camera|First Track|Dolly Zoom")
    virtual void SetPrimaryTrackDollyZoomReferenceDistance(float Distance);

    UFUNCTION(BlueprintCallable, Category = "Extended Camera|Second Track|Dolly Zoom")
    virtual void SetSecondaryTrackDollyZoomReferenceDistance(float Distance);

    UFUNCTION(BlueprintCallable, Category = "Extended Camera|First Track|Dolly Zoom")
    virtual void SetPrimaryTrackDollyZoomEnabled(bool Enabled);

    UFUNCTION(BlueprintCallable, Category = "Extended Camera|Second Track|Dolly Zoom")
    virtual void SetSecondaryTrackDollyZoomEnabled(bool Enabled);

    UFUNCTION(BlueprintCallable, Category = "Extended Camera|First Track|Dolly Zoom")
    virtual void SetPrimaryTrackDollyZoomLiveUpdate(bool Enabled);

    UFUNCTION(BlueprintCallable, Category = "Extended Camera|Second Track|Dolly Zoom")
    virtual void SetSecondaryTrackDollyZoomLiveUpdate(bool Enabled);

    UFUNCTION(BlueprintCallable, Category = "Extended Camera|First Track")
    virtual void SetPrimaryTrackedCamera(ACameraActor *TrackedCamera);

    UFUNCTION(BlueprintCallable, Category = "Extended Camera|Second Track")
    virtual void SetSecondaryTrackedCamera(ACameraActor *TrackedCamera);

    UFUNCTION(BlueprintCallable, Category = "Extended Camera|First Track|Locator")
    virtual void SetPrimaryTrackLocator(AActor *TrackedActor);

    UFUNCTION(BlueprintCallable, Category = "Extended Camera|Second Track|Locator")
    virtual void SetSecondaryTrackLocator(AActor *TrackedActor);

    UFUNCTION(BlueprintCallable, Category = "Extended Camera|First Track|Locator")
    virtual void SetPrimaryTrackAim(AActor *TrackedActor);

    UFUNCTION(BlueprintCallable, Category = "Extended Camera|Second Track|Locator")
    virtual void SetSecondaryTrackAim(AActor *TrackedActor);

    UFUNCTION(BlueprintCallable, Category = "Extended Camera|First Track|Locator")
    virtual void SetPrimaryTrackAimOffset(UPARAM(ref) FVector &AimOffset);

    UFUNCTION(BlueprintCallable, Category = "Extended Camera|Second Track|Locator")
    virtual void SetSecondaryTrackAimOffset(UPARAM(ref) FVector &AimOffset);

    UFUNCTION(BlueprintCallable, Category = "Extended Camera|First Track|Locator")
    virtual void SetPrimaryTrackAimInterpolationSpeed(float Speed);

    UFUNCTION(BlueprintCallable, Category = "Extended Camera|Second Track|Locator")
    virtual void SetSecondaryTrackAimInterpolationSpeed(float Speed);


    /**
     * Set Primary Locator Bone Name
     *
     * If the actor is already set, and mode is correct, check for the bone
     * Returns whether the bone is valid
     */
    UFUNCTION(BlueprintCallable, Category = "Extended Camera|First Track|Locator")
    virtual bool SetPrimaryLocatorBoneName(FName TrackedBoneName);

    /**
     * Set Secondary Locator Bone Name
     *
     * If the actor is already set, and mode is correct, check for the bone
     * Returns whether the bone is valid
     */
    UFUNCTION(BlueprintCallable, Category = "Extended Camera|Second Track|Locator")
    virtual bool SetSecondaryLocatorBoneName(FName TrackedBoneName);

    /**
     * Set Primary Aim Bone Name
     *
     * If the actor is already set, and mode is correct, check for the bone
     * Returns whether the bone is valid
     */
    UFUNCTION(BlueprintCallable, Category = "Extended Camera|First Track|Locator")
    virtual bool SetPrimaryLocatorAimName(FName TrackedAimName);

    /**
     * Set Secondary Aim Bone Name
     *
     * If the actor is already set, and mode is correct, check for the bone
     * Returns whether the bone is valid
     */
    UFUNCTION(BlueprintCallable, Category = "Extended Camera|Second Track|Locator")
    virtual bool SetSecondaryLocatorAimName(FName TrackedAimName);

    UFUNCTION(BlueprintCallable, Category = "Extended Camera")
    virtual void SetFOVCheckOffsetInRadians(float FOVOffset);

    UFUNCTION(BlueprintCallable, Category = "Extended Camera|Dolly Zoom")
    virtual void SetUseDollyZoom(bool NewState);

    UFUNCTION(BlueprintCallable, Category = "Extended Camera|Smooth Return")
    virtual void SetSmoothReturn(bool NewState);

    UFUNCTION(BlueprintCallable, Category = "Extended Camera|Smooth Return")
    virtual void SetSmoothReturnSpeed(float NewReturnSpeed);

    UFUNCTION(BlueprintCallable, Category = "Extended Camera|Smooth Return")
    virtual void SetSmoothReturnDeadzone(float NewDeadzone);

    UFUNCTION(BlueprintCallable, Category = "Extended Camera|First Track")
    virtual void SetPrimaryTrackMode(EExtendedCameraDriverMode NewMode);

    UFUNCTION(BlueprintCallable, Category = "Extended Camera|Second Track")
    virtual void SetSecondaryTrackMode(EExtendedCameraDriverMode NewMode);

    UFUNCTION(BlueprintCallable, Category = "Extended Camera|First Track|Debug")
    virtual void SetPrimaryTrackAimDebug(bool Enabled);

    UFUNCTION(BlueprintCallable, Category = "Extended Camera|Second Track|Debug")
    virtual void SetSecondaryTrackAimDebug(bool Enabled);


    UFUNCTION(BlueprintNativeEvent)
    void KeepInFrameLineOfSight(AActor *Owner, FMinimalViewInfo &DesiredView);
    virtual void KeepInFrameLineOfSight_Implementation(AActor *Owner, FMinimalViewInfo &DesiredView);

    UFUNCTION(BlueprintNativeEvent)
    void KeepAnyLineOfSight(AActor *Owner, FMinimalViewInfo &DesiredView);
    virtual void KeepAnyLineOfSight_Implementation(AActor *Owner, FMinimalViewInfo &DesiredView);

    UFUNCTION(BlueprintNativeEvent)
    void CommonKeepLineOfSight(AActor *Owner, FMinimalViewInfo &DesiredView);
    virtual void CommonKeepLineOfSight_Implementation(AActor *Owner, FMinimalViewInfo &DesiredView);

    virtual void DollyZoom(AActor *Owner, FMinimalViewInfo &DesiredView, FHitResult &LOSCheck);

    UFUNCTION(BlueprintCallable, Category = "Extended Camera")
    virtual float DollyZoom(float ReferenceDistance, float ReferenceFOV, float CurrentDistance);

    virtual void GetCameraView(float DeltaTime, FMinimalViewInfo &DesiredView) override;

    virtual void BeginPlay() override;

public:
    // Movers for C++

    // Set Primary Track
    virtual void SetCameraPrimaryTrack(FVector &&InLocation, FRotator &&InRotation, float InFOV);

    // Set Secondary Track
    virtual void SetCameraSecondaryTrack(FVector &&InLocation, FRotator &&InRotation, float InFOV);

    // Set Primary Track
    virtual void SetCameraPrimaryLocationRotation(FVector &&InLocation, FRotator &&InRotation);

    // Set Secondary Track
    virtual void SetCameraSecondaryLocationRotation(FVector &&InLocation, FRotator &&InRotation);

    // Set Primary Track
    virtual void SetCameraPrimaryRotation(FRotator &&InRotation);

    // Set Secondary Track
    virtual void SetCameraSecondaryRotation(FRotator &&InRotation);

    // Set Primary Track
    virtual void SetCameraPrimaryLocation(FVector &&InLocation);

    // Set Secondary Track
    virtual void SetCameraSecondaryLocation(FVector &&InLocation);
};
