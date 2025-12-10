/*
 * @Author: Punal Manalan
 * @Description: Input Axis Binding Structure - Stores analog axis mappings
 * @Date: 06/12/2025
 */

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "FS_InputAxisBinding.generated.h"

/**
 * Represents a single axis binding with scale
 */
USTRUCT(BlueprintType)
struct FS_AxisKeyBinding
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Binding")
    FKey Key;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Binding")
    float Scale = 1.0f;

    /** If true, swizzle input from X to Y axis (for W/S keys in WASD movement) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Binding")
    bool bSwizzleYXZ = false;
};

/**
 * Complete axis binding configuration
 */
USTRUCT(BlueprintType)
struct FS_InputAxisBinding
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Binding")
    FName InputAxisName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Binding")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Binding")
    FName Category = FName(TEXT("General"));

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Binding")
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Binding")
    TArray<FS_AxisKeyBinding> AxisBindings;

    /** The value type for this axis action (Axis1D for single axis, Axis2D for 2D movement like WASD) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Binding")
    EInputActionValueType ValueType = EInputActionValueType::Axis1D;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Binding")
    float DeadZone = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Binding")
    float Sensitivity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Binding")
    float Priority = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Binding")
    bool bEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Binding")
    bool bInvert = false;
};
