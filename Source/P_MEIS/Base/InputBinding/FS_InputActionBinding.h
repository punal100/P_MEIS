/*
 * @Author: Punal Manalan
 * @Description: Input Action Binding Structure - Stores keyboard/gamepad action mappings
 * @Date: 06/12/2025
 */

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "FS_InputActionBinding.generated.h"

/**
 * Represents a single key or button binding
 */
USTRUCT(BlueprintType)
struct FS_KeyBinding
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Binding")
    FKey Key;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Binding")
    float Value = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Binding")
    bool bShift = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Binding")
    bool bCtrl = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Binding")
    bool bAlt = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Binding")
    bool bCmd = false;
};

/**
 * Complete action binding configuration
 */
USTRUCT(BlueprintType)
struct FS_InputActionBinding
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Binding")
    FName InputActionName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Binding")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Binding")
    FName Category = FName(TEXT("General"));

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Binding")
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Binding")
    TArray<FS_KeyBinding> KeyBindings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Binding")
    float Priority = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Binding")
    bool bEnabled = true;
};

/**
 * Represents a conflict between two input actions
 */
USTRUCT(BlueprintType)
struct FInputBindingConflict
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Binding")
    FName ActionA;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Binding")
    FName ActionB;

    FInputBindingConflict() {}
    FInputBindingConflict(const FName &InA, const FName &InB) : ActionA(InA), ActionB(InB) {}
};
