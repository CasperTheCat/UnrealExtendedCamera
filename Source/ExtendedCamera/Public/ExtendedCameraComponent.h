// Copyright Acinonyx Ltd. 2021. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"

#include "ExtendedCameraComponent.generated.h"

UCLASS(config = Game)
class EXTENDEDCAMERA_API UExtendedCameraComponent : public UCameraComponent
{
	GENERATED_BODY()

protected:
	// Extended Camera Blend Points

	// Primary Track - Set by users
	UPROPERTY(Transient)
	FVector PrimaryTrackLocation;

	UPROPERTY(Transient)
	FRotator PrimaryTrackRotation;

	UPROPERTY(Transient)
	float PrimaryTrackFOV = 0.f;


	// Secondary Track - Set by users
	UPROPERTY(Transient)
	FVector SecondaryTrackLocation;

	UPROPERTY(Transient)
	FRotator SecondaryTrackRotation;

	UPROPERTY(Transient)
	float SecondaryTrackFOV = 0.f;

	// Blend Amount for the first channel
	UPROPERTY(Transient)
	float CameraPrimaryTrackBlendAlpha;

	// Blend Amount
	UPROPERTY(Transient)
	float CameraSecondaryTrackBlendAlpha;

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
	virtual void SetCameraPrimaryLocationRotation(UPARAM(ref) FVector& InLocation, UPARAM(ref) FRotator& InRotation);

	// Set Secondary Track
	UFUNCTION(BlueprintCallable)
	virtual void SetCameraSecondaryLocationRotation(UPARAM(ref) FVector& InLocation, UPARAM(ref) FRotator& InRotation);

	// // Set
	// UFUNCTION(BlueprintCallable)
	// virtual void SetUsePrimaryTrack(bool usePrimaryTrack);

	UFUNCTION(BlueprintCallable)
	virtual bool GetUsePrimaryTrack();

	UFUNCTION(BlueprintCallable)
	virtual bool GetUseSecondaryTrack();

	// Called by CalcCamera in AActor
	//virtual void GetCameraView(float DeltaTime, FMinimalViewInfo& DesiredView);

	virtual void GetCameraView(float DeltaTime, FMinimalViewInfo& DesiredView) override;
};