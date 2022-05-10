// Copyright Acinonyx Ltd. 2022. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "Camera/CameraActor.h"

#include "ExtendedCameraComponent.generated.h"

DECLARE_STATS_GROUP(TEXT("Acinonyx Extended Camera"), STATGROUP_ACIExtCam, STATCAT_Advanced);

UENUM(BlueprintType)
enum EExtendedCameraMode
{
	Ignore UMETA(DisplayName = "Ignore"),
	KeepLos UMETA(DisplayName = "Keep LOS to Owner in Frame"),
	KeepLosNoDot UMETA(DisplayName = "Always Keep Line of Sight"),
	TOTAL_CAMERA_MODES UMETA(Hidden)
};

UCLASS(config = Game, BlueprintType, Blueprintable, ClassGroup=Camera, meta=(BlueprintSpawnableComponent))
class EXTENDEDCAMERA_API UExtendedCameraComponent : public UCameraComponent
{
	GENERATED_BODY()

protected:
	// DollyZoom
	UPROPERTY(SaveGame)
	bool IsLOSBlocked;

	UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Extended Camera")
	float StoredLOSFOV;

	// Dolly Zooms

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
	UPROPERTY(SaveGame, Interp, EditAnywhere, BlueprintReadWrite, Category = "Extended Camera")
	float SecondTrackDollyZoomReferenceDistance;

	/**
	 * Dolly Zoom Enabled for Second Track
	 *
	 * This enables the Dolly zoom for the second track
	 * See SecondTrackDollyZoomReferenceDistance for more information
	 */
	UPROPERTY(SaveGame, Interp, EditAnywhere, BlueprintReadWrite, Category = "Extended Camera")
	bool SecondTrackDollyZoomEnabled;

	/**
	 * Dolly Zoom Live Update Enabled for Second Track
	 *
	 * This enables the live update of the reference distance
	 * See SecondTrackDollyZoomReferenceDistance for more information
	 */
	UPROPERTY(SaveGame, Interp, EditAnywhere, BlueprintReadWrite, Category = "Extended Camera")
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
	UPROPERTY(SaveGame, Interp, EditAnywhere, BlueprintReadWrite, Category = "Extended Camera")
	float FirstTrackDollyZoomReferenceDistance;

	/**
	 * Dolly Zoom Enabled for Second Track
	 *
	 * This enables the Dolly zoom for the second track
	 * See SecondTrackDollyZoomReferenceDistance for more information
	 */
	UPROPERTY(SaveGame, Interp, EditAnywhere, BlueprintReadWrite, Category = "Extended Camera")
	bool FirstTrackDollyZoomEnabled;

	/**
	 * Dolly Zoom Live Update Enabled for Second Track
	 *
	 * This enables the live update of the reference distance
	 * See SecondTrackDollyZoomReferenceDistance for more information
	 */
	UPROPERTY(SaveGame, Interp, EditAnywhere, BlueprintReadWrite, Category = "Extended Camera")
	bool FirstTrackDollyZoomDistanceLiveUpdate;
	
	// Extended Camera Blend Point

	// Target Camera
	UPROPERTY(SaveGame, Interp, EditAnywhere, BlueprintReadWrite, Category = "Extended Camera")
	ACameraActor* PrimaryTrackedCamera;
	
	// Target Camera
	UPROPERTY(SaveGame, Interp, EditAnywhere, BlueprintReadWrite, Category = "Extended Camera")
	ACameraActor* SecondTrackedCamera;

	// Write Tracked Camera to Secondary Track, otherwise Primary
	//UPROPERTY(SaveGame, Interp, EditAnywhere, BlueprintReadWrite, Category = "Extended Camera")
	//bool WriteTrackedToSecondary;

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
	UPROPERTY(SaveGame, Interp, EditAnywhere, BlueprintReadWrite, Category = "Extended Camera", meta = (UIMin = "0.0", UIMax = "175", ClampMin = "0.0", ClampMax = "360.0", Units = deg))
	float PrimaryTrackFOV = 0.f;

	// Secondary Track - Set by users
	UPROPERTY(SaveGame, Interp, EditAnywhere, BlueprintReadWrite, Category = "Extended Camera")
	FTransform SecondaryTrackTransform;

	/**
	 * Secondary FOV
	 * Zero disables FOV blending
	 */
	UPROPERTY(SaveGame, Interp, EditAnywhere, BlueprintReadWrite, Category = "Extended Camera", meta = (UIMin = "0.0", UIMax = "175", ClampMin = "0.0", ClampMax = "360.0", Units = deg))
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
	UPROPERTY(SaveGame, Interp, EditAnywhere, BlueprintReadWrite, Category = "Extended Camera")
	bool UseDollyZoomForLOS;



public:
	// Set the Blend Amount
	UFUNCTION(BlueprintCallable, Category = "Extended Camera")
	virtual void SetPrimaryCameraTrackAlpha(float Alpha);

	// Set the Blend Amount
	UFUNCTION(BlueprintCallable, Category = "Extended Camera")
	virtual void SetSecondaryCameraTrackAlpha(float Alpha);

	// Get the Blend Amount
	UFUNCTION(BlueprintCallable, Category = "Extended Camera")
	virtual float GetPrimaryCameraTrackAlpha();

	UFUNCTION(BlueprintCallable, Category = "Extended Camera")
	virtual float GetSecondaryCameraTrackAlpha();

	// Set Primary Track 
	UFUNCTION(BlueprintCallable, Category = "Extended Camera")
	virtual void SetCameraPrimaryTrack(UPARAM(ref) FVector& InLocation, UPARAM(ref) FRotator& InRotation, float InFOV);

	// Set Secondary Track
	UFUNCTION(BlueprintCallable, Category = "Extended Camera")
	virtual void SetCameraSecondaryTrack(UPARAM(ref) FVector& InLocation, UPARAM(ref) FRotator& InRotation, float InFOV);

	// Set Primary Track 
	UFUNCTION(BlueprintCallable, Category = "Extended Camera")
	virtual void SetCameraPrimaryTransform(UPARAM(ref) FTransform& InTransform, float InFOV);

	// Set Secondary Track
	UFUNCTION(BlueprintCallable, Category = "Extended Camera")
	virtual void SetCameraSecondaryTransform(UPARAM(ref) FTransform& InTransform, float InFOV);
	
	// Set Primary Track
	UFUNCTION(BlueprintCallable, Category = "Extended Camera")
	virtual void SetCameraPrimaryLocationRotation(UPARAM(ref) FVector& InLocation, UPARAM(ref) FRotator& InRotation);

	// Set Secondary Track
	UFUNCTION(BlueprintCallable, Category = "Extended Camera")
	virtual void SetCameraSecondaryLocationRotation(UPARAM(ref) FVector& InLocation, UPARAM(ref) FRotator& InRotation);

	// Set Primary Track
	UFUNCTION(BlueprintCallable, Category = "Extended Camera")
	virtual void SetCameraPrimaryRotation(UPARAM(ref) FRotator& InRotation);

	// Set Secondary Track
	UFUNCTION(BlueprintCallable, Category = "Extended Camera")
	virtual void SetCameraSecondaryRotation(UPARAM(ref) FRotator& InRotation);

	// Set Primary Track
	UFUNCTION(BlueprintCallable, Category = "Extended Camera")
	virtual void SetCameraPrimaryLocation(UPARAM(ref) FVector& InLocation);

	// Set Secondary Track
	UFUNCTION(BlueprintCallable, Category = "Extended Camera")
	virtual void SetCameraSecondaryLocation(UPARAM(ref) FVector& InLocation);

	// Set Primary Track 
	UFUNCTION(BlueprintCallable, Category = "Extended Camera")
	virtual void SetCameraPrimaryFOV(float InFOV);

	// Set Secondary Track
	UFUNCTION(BlueprintCallable, Category = "Extended Camera")
	virtual void SetCameraSecondaryFOV(float InFOV);

	UFUNCTION(BlueprintCallable, Category = "Extended Camera")
	virtual bool GetUsePrimaryTrack();

	UFUNCTION(BlueprintCallable, Category = "Extended Camera")
	virtual bool GetUseSecondaryTrack();

	UFUNCTION(BlueprintCallable, Category = "Extended Camera")
	virtual void SetCameraMode(EExtendedCameraMode NewMode);

	UFUNCTION(BlueprintCallable, Category = "Extended Camera")
	virtual TEnumAsByte<EExtendedCameraMode> GetCameraMode();

	UFUNCTION(BlueprintNativeEvent)
	void KeepInFrameLineOfSight(AActor *Owner, FMinimalViewInfo& DesiredView);
	virtual void KeepInFrameLineOfSight_Implementation(AActor* Owner, FMinimalViewInfo& DesiredView);

	UFUNCTION(BlueprintNativeEvent)
	void KeepAnyLineOfSight(AActor* Owner, FMinimalViewInfo& DesiredView);
	virtual void KeepAnyLineOfSight_Implementation(AActor* Owner, FMinimalViewInfo& DesiredView);

	UFUNCTION(BlueprintNativeEvent)
	void CommonKeepLineOfSight(AActor* Owner, FMinimalViewInfo& DesiredView);
	virtual void CommonKeepLineOfSight_Implementation(AActor* Owner, FMinimalViewInfo& DesiredView);
	
	virtual void DollyZoom(AActor* Owner, FMinimalViewInfo& DesiredView, FHitResult &LOSCheck);

	UFUNCTION(BlueprintCallable, Category = "Extended Camera")
	virtual float DollyZoom(float ReferenceDistance, float ReferenceFOV, float CurrentDistance);

	virtual void GetCameraView(float DeltaTime, FMinimalViewInfo& DesiredView) override;

public:
	// Movers for C++
	
	// Set Primary Track 
	virtual void SetCameraPrimaryTrack(FVector&& InLocation, FRotator&& InRotation, float InFOV);

	// Set Secondary Track
	virtual void SetCameraSecondaryTrack( FVector&& InLocation, FRotator&& InRotation, float InFOV);

	// Set Primary Track
	virtual void SetCameraPrimaryLocationRotation(FVector&& InLocation, FRotator&& InRotation);

	// Set Secondary Track
	virtual void SetCameraSecondaryLocationRotation(FVector&& InLocation, FRotator&& InRotation);

	// Set Primary Track
	virtual void SetCameraPrimaryRotation(FRotator&& InRotation);

	// Set Secondary Track
	virtual void SetCameraSecondaryRotation( FRotator&& InRotation);

	// Set Primary Track
	virtual void SetCameraPrimaryLocation(FVector&& InLocation);

	// Set Secondary Track
	virtual void SetCameraSecondaryLocation(FVector&& InLocation);
};