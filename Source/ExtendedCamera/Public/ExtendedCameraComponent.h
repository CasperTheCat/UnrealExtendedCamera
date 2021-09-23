// Copyright Acinonyx Ltd. 2021. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"

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

UCLASS(config = Game)
class EXTENDEDCAMERA_API UExtendedCameraComponent : public UCameraComponent
{
	GENERATED_BODY()

protected:
	// Extended Camera Blend Points

	// Primary Track - Set by users
	UPROPERTY(SaveGame)
	FVector PrimaryTrackLocation;

	UPROPERTY(SaveGame)
	FRotator PrimaryTrackRotation;

	UPROPERTY(SaveGame)
	float PrimaryTrackFOV = 0.f;


	// Secondary Track - Set by users
	UPROPERTY(SaveGame)
	FVector SecondaryTrackLocation;

	UPROPERTY(SaveGame)
	FRotator SecondaryTrackRotation;

	UPROPERTY(SaveGame)
	float SecondaryTrackFOV = 0.f;

	// Blend Amount for the first channel
	UPROPERTY(SaveGame)
	float CameraPrimaryTrackBlendAlpha;

	// Blend Amount
	UPROPERTY(SaveGame)
	float CameraSecondaryTrackBlendAlpha;

	// LOS Mode
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Extended Camera")
	TEnumAsByte<EExtendedCameraMode> CameraLOSMode;
	
	// Small Offset for FOV checks
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Extended Camera")
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

	// // Set
	// UFUNCTION(BlueprintCallable)
	// virtual void SetUsePrimaryTrack(bool usePrimaryTrack);

	UFUNCTION(BlueprintCallable)
	virtual bool GetUsePrimaryTrack();

	UFUNCTION(BlueprintCallable)
	virtual bool GetUseSecondaryTrack();

	UFUNCTION(BlueprintCallable)
	virtual void SetCameraMode(EExtendedCameraMode NewMode);

	UFUNCTION(BlueprintCallable)
	virtual TEnumAsByte<EExtendedCameraMode> GetCameraMode();


	// Called by CalcCamera in AActor
	//virtual void GetCameraView(float DeltaTime, FMinimalViewInfo& DesiredView);

	virtual void GetCameraView(float DeltaTime, FMinimalViewInfo& DesiredView) override;
};