// Copyright Acinonyx Ltd. 2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "ExtendedCameraTypes.generated.h"

/**
 * Used to hold view info
 *
 * Avoids passing a large number of variables as parameters
 */
USTRUCT(Blueprintable, BlueprintType)
struct EXTENDEDCAMERA_API FExtendedCameraViewInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="View Info")
    FVector Location;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="View Info")
    FQuat Rotation;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="View Info")
    float FOV;
};

/**
 * Used to hold locator and aim info
 *
 * Avoids passing a large number of variables as parameters
 */
USTRUCT(Blueprintable, BlueprintType)
struct EXTENDEDCAMERA_API FExtendedCameraLocatorAimInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="LocAimInfo")
    TObjectPtr<AActor> Actor;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="LocAimInfo")
    FName BoneName;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="LocAimInfo")
    FVector Offset;
};
