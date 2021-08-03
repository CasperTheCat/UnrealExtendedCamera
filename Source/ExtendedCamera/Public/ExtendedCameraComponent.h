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

	// Secondary Track - Set by users
	UPROPERTY(Transient)
	FVector SecondaryTrackLocation;

	UPROPERTY(Transient)
	FRotator SecondaryTrackRotation;

	// This bool switches between Primary and Game track
	// When on, we blend from Primary to Secondary
	// When off, we blend from Game to Secondary
	UPROPERTY(Transient)
	bool CameraIsUsingPrimaryTrack;

	// Blend Amount
	UPROPERTY(Transient)
	float CameraTrackBlendAlpha;

public:
	// Set the Blend Amount
	UFUNCTION(BlueprintCallable)
	virtual void SetCameraTrackAlpha(float Alpha);

	// Get the Blend Amount
	UFUNCTION(BlueprintCallable)
	virtual float GetCameraTrackAlpha();

	// Set Primary Track
	UFUNCTION(BlueprintCallable)
	virtual void SetCameraPrimaryLocationRotation(UPARAM(ref) FVector& InLocation, UPARAM(ref) FRotator& InRotation);

	// Set Secondary Track
	UFUNCTION(BlueprintCallable)
	virtual void SetCameraSecondaryLocationRotation(UPARAM(ref) FVector& InLocation, UPARAM(ref) FRotator& InRotation);

	// Set
	UFUNCTION(BlueprintCallable)
	virtual void SetUsePrimaryTrack(bool usePrimaryTrack);

	UFUNCTION(BlueprintCallable)
	virtual bool GetUsePrimaryTrack();

	// Called by CalcCamera in AActor
	//virtual void GetCameraView(float DeltaTime, FMinimalViewInfo& DesiredView);

	virtual void GetCameraView(float DeltaTime, FMinimalViewInfo& DesiredView) override;
};