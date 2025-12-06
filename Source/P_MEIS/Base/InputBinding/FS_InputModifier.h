/*
 * @Author: Punal Manalan
 * @Description: Input Modifier Structure - Defines modifiers like dead zone, curves, etc.
 * @Date: 06/12/2025
 */

#pragma once

#include "CoreMinimal.h"
#include "FS_InputModifier.generated.h"

/**
 * Modifier types for input processing
 */
UENUM(BlueprintType)
enum class EInputModifierType : uint8
{
DeadZone = 0 UMETA(DisplayName = "Dead Zone"),
ResponseCurve = 1 UMETA(DisplayName = "Response Curve"),
Negate = 2 UMETA(DisplayName = "Negate"),
Swizzle = 3 UMETA(DisplayName = "Swizzle"),
Clamp = 4 UMETA(DisplayName = "Clamp"),
Scale = 5 UMETA(DisplayName = "Scale")
};

/**
 * Input modifier configuration
 */
USTRUCT(BlueprintType)
struct FS_InputModifier
{
GENERATED_BODY()

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Modifier")
EInputModifierType ModifierType = EInputModifierType::DeadZone;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Modifier")
float DeadZoneValue = 0.2f;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Modifier")
float ScaleValue = 1.0f;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Modifier")
FVector2D ClampRange = FVector2D(-1.0f, 1.0f);

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Modifier")
bool bInvert = false;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Modifier")
bool bEnabled = true;
};
