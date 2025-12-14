/*
 * @Author: Punal Manalan
 * @Description: Input Profile Structure - Complete input configuration snapshot
 * @Date: 06/12/2025
 */

#pragma once

#include "CoreMinimal.h"
#include "FS_InputActionBinding.h"
#include "FS_InputAxisBinding.h"
#include "FS_InputModifier.h"
#include "FS_InputProfile.generated.h"

/**
 * Complete input profile containing all bindings
 */
USTRUCT(BlueprintType)
struct FS_InputProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Profile")
    FName ProfileName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Profile")
    FText ProfileDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Profile")
    TArray<FS_InputActionBinding> ActionBindings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Profile")
    TArray<FS_InputAxisBinding> AxisBindings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Profile")
    TArray<FS_InputModifier> Modifiers;

    // ==================== Game Preferences (Optional) ====================

    /**
     * Which actions behave as Toggle instead of Hold.
     * NOTE: P_MEIS is modular; action names are provided by the game/module.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Profile")
    TArray<FName> ToggleModeActions;

    /**
     * Runtime state persisted per profile: explicit per-action toggle state.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Profile")
    TMap<FName, bool> ToggleActionStates;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Profile")
    FDateTime Timestamp = FDateTime::Now();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Profile")
    int32 Version = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Profile")
    FString CreatedBy;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Profile")
    bool bIsDefault = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Profile")
    bool bIsCompetitive = false;
};
