/*
 * @Author: Punal Manalan
 * @Description: Input Axis Binding Structure - Stores analog axis mappings
 * @Date: 06/12/2025
 */

#pragma once

#include "CoreMinimal.h"
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
