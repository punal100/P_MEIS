/*
 * @Author: Punal Manalan
 * @Description: Input Validator - Validates input bindings and detects conflicts
 * @Date: 06/12/2025
 */

#pragma once

#include "CoreMinimal.h"
#include "InputBinding/FS_InputActionBinding.h"
#include "InputBinding/FS_InputAxisBinding.h"
#include "CPP_InputValidator.generated.h"

/**
 * Static utility class for input validation
 */
UCLASS()
class P_MEIS_API UCPP_InputValidator : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Validation")
    static bool ValidateKeyBinding(const FS_KeyBinding &KeyBinding, FString &OutErrorMessage);

    UFUNCTION(BlueprintCallable, Category = "Input Binding|Validation")
    static bool ValidateAxisKeyBinding(const FS_AxisKeyBinding &AxisBinding, FString &OutErrorMessage);

    UFUNCTION(BlueprintCallable, Category = "Input Binding|Validation")
    static bool ValidateActionBinding(const FS_InputActionBinding &ActionBinding, FString &OutErrorMessage);

    UFUNCTION(BlueprintCallable, Category = "Input Binding|Validation")
    static bool ValidateAxisBinding(const FS_InputAxisBinding &AxisBinding, FString &OutErrorMessage);

    UFUNCTION(BlueprintCallable, Category = "Input Binding|Validation")
    static bool IsValidKey(const FKey &Key);

    UFUNCTION(BlueprintCallable, Category = "Input Binding|Validation")
    static bool IsValidGamepadKey(const FKey &Key);

    // Note: Not exposed to Blueprint due to TPair incompatibility with UHT
    static bool DetectConflicts(const TArray<FS_InputActionBinding> &ActionBindings,
                                TArray<TPair<FName, FName>> &OutConflicts);
};
