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
	// Extended Camera Blend Points

	// Target Camera
	UPROPERTY(SaveGame, Interp, EditAnywhere, BlueprintReadWrite, Category = "Extended Camera")
	ACameraActor* TrackedCamera;

	// Write Tracked Camera to Secondary Track, otherwise Primary
	UPROPERTY(SaveGame, Interp, EditAnywhere, BlueprintReadWrite, Category = "Extended Camera")
	bool WriteTrackedToSecondary;

	// Primary Track - Set by users
	UPROPERTY(SaveGame, Interp, EditAnywhere, BlueprintReadWrite, Category = "Extended Camera")
	FTransform PrimaryTrackTransform;

	UPROPERTY(SaveGame, Interp, EditAnywhere, BlueprintReadWrite, Category = "Extended Camera", meta = (UIMin = "5.0", UIMax = "170", ClampMin = "0.001", ClampMax = "360.0", Units = deg))
	float PrimaryTrackFOV = 0.f;


	// Secondary Track - Set by users
	UPROPERTY(SaveGame, Interp, EditAnywhere, BlueprintReadWrite, Category = "Extended Camera")
	FTransform SecondaryTrackTransform;

	UPROPERTY(SaveGame, Interp, EditAnywhere, BlueprintReadWrite, Category = "Extended Camera", meta = (UIMin = "5.0", UIMax = "170", ClampMin = "0.001", ClampMax = "360.0", Units = deg))
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




public:
	// Set the Blend Amount
	UFUNCTION(BlueprintCallable)
	virtual void SetPrimaryCameraTrackAlpha(float Alpha);

	// Set the Blend Amount
	UFUNCTION(BlueprintCallable)
	virtual void SetSecondaryCameraTrackAlpha(float Alpha);

	// Get the Blend Amount
	UFUNCTION(BlueprintCallable)
	virtual float GetPrimaryCameraTrackAlpha();

	UFUNCTION(BlueprintCallable)
	virtual float GetSecondaryCameraTrackAlpha();

	// Set Primary Track 
	UFUNCTION(BlueprintCallable)
	virtual void SetCameraPrimaryTrack(UPARAM(ref) FVector& InLocation, UPARAM(ref) FRotator& InRotation, float InFOV);

	// Set Secondary Track
	UFUNCTION(BlueprintCallable)
	virtual void SetCameraSecondaryTrack(UPARAM(ref) FVector& InLocation, UPARAM(ref) FRotator& InRotation, float InFOV);

	// Set Primary Track 
	UFUNCTION(BlueprintCallable)
	virtual void SetCameraPrimaryTransform(UPARAM(ref) FTransform& InTransform, float InFOV);

	// Set Secondary Track
	UFUNCTION(BlueprintCallable)
	virtual void SetCameraSecondaryTransform(UPARAM(ref) FTransform& InTransform, float InFOV);
	
	// Set Primary Track
	UFUNCTION(BlueprintCallable)
	virtual void SetCameraPrimaryLocationRotation(UPARAM(ref) FVector& InLocation, UPARAM(ref) FRotator& InRotation);

	// Set Secondary Track
	UFUNCTION(BlueprintCallable)
	virtual void SetCameraSecondaryLocationRotation(UPARAM(ref) FVector& InLocation, UPARAM(ref) FRotator& InRotation);

	// Set Primary Track
	UFUNCTION(BlueprintCallable)
	virtual void SetCameraPrimaryRotation(UPARAM(ref) FRotator& InRotation);

	// Set Secondary Track
	UFUNCTION(BlueprintCallable)
	virtual void SetCameraSecondaryRotation(UPARAM(ref) FRotator& InRotation);

	// Set Primary Track
	UFUNCTION(BlueprintCallable)
	virtual void SetCameraPrimaryLocation(UPARAM(ref) FVector& InLocation);

	// Set Secondary Track
	UFUNCTION(BlueprintCallable)
	virtual void SetCameraSecondaryLocation(UPARAM(ref) FVector& InLocation);

	// Set Primary Track 
	UFUNCTION(BlueprintCallable)
	virtual void SetCameraPrimaryFOV(float InFOV);

	// Set Secondary Track
	UFUNCTION(BlueprintCallable)
	virtual void SetCameraSecondaryFOV(float InFOV);

	UFUNCTION(BlueprintCallable)
	virtual bool GetUsePrimaryTrack();

	UFUNCTION(BlueprintCallable)
	virtual bool GetUseSecondaryTrack();

	UFUNCTION(BlueprintCallable)
	virtual void SetCameraMode(EExtendedCameraMode NewMode);

	UFUNCTION(BlueprintCallable)
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