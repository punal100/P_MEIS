/*
 * @Author: Punal Manalan
 * @Description: Blueprint Function Library - Public API for Input Binding System
 * @Date: 06/12/2025
 */

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "InputBinding/FS_InputProfile.h"
#include "InputBinding/FS_InputActionBinding.h"
#include "InputBinding/FS_InputAxisBinding.h"
#include "CPP_BPL_InputBinding.generated.h"

class UCPP_InputBindingManager;

/**
 * Public Blueprint Function Library for Input Binding operations
 */
UCLASS()
class P_MEIS_API UCPP_BPL_InputBinding : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    // Profile Management Functions
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Profile")
    static bool LoadProfile(const FString &ProfileName);

    UFUNCTION(BlueprintCallable, Category = "Input Binding|Profile")
    static bool SaveProfile(const FString &ProfileName);

    UFUNCTION(BlueprintCallable, Category = "Input Binding|Profile")
    static bool CreateProfile(const FString &ProfileName);

    UFUNCTION(BlueprintCallable, Category = "Input Binding|Profile")
    static bool DeleteProfile(const FString &ProfileName);

    UFUNCTION(BlueprintCallable, Category = "Input Binding|Profile")
    static bool RenameProfile(const FString &OldName, const FString &NewName);

    UFUNCTION(BlueprintCallable, Category = "Input Binding|Profile")
    static bool DuplicateProfile(const FString &SourceProfile, const FString &NewProfile);

    UFUNCTION(BlueprintCallable, Category = "Input Binding|Profile")
    static bool ResetToDefaults();

    UFUNCTION(BlueprintCallable, Category = "Input Binding|Profile")
    static void GetAvailableProfiles(TArray<FString> &OutProfiles);

    // Action Binding Functions
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Action")
    static bool SetActionBinding(const FName &ActionName, const FS_InputActionBinding &Binding);

    UFUNCTION(BlueprintCallable, Category = "Input Binding|Action")
    static bool RemoveActionBinding(const FName &ActionName);

    UFUNCTION(BlueprintCallable, Category = "Input Binding|Action")
    static bool GetActionBinding(const FName &ActionName, FS_InputActionBinding &OutBinding);

    UFUNCTION(BlueprintCallable, Category = "Input Binding|Action")
    static void GetActionBindings(TArray<FS_InputActionBinding> &OutBindings);

    UFUNCTION(BlueprintCallable, Category = "Input Binding|Action")
    static bool ClearActionBindings();

    // Axis Binding Functions
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Axis")
    static bool SetAxisBinding(const FName &AxisName, const FS_InputAxisBinding &Binding);

    UFUNCTION(BlueprintCallable, Category = "Input Binding|Axis")
    static bool RemoveAxisBinding(const FName &AxisName);

    UFUNCTION(BlueprintCallable, Category = "Input Binding|Axis")
    static bool GetAxisBinding(const FName &AxisName, FS_InputAxisBinding &OutBinding);

    UFUNCTION(BlueprintCallable, Category = "Input Binding|Axis")
    static void GetAxisBindings(TArray<FS_InputAxisBinding> &OutBindings);

    UFUNCTION(BlueprintCallable, Category = "Input Binding|Axis")
    static bool ClearAxisBindings();

    // Validation & Conflict Detection
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Validation")
    static bool ValidateBinding(const FS_InputActionBinding &Binding, FString &OutErrorMessage);

    // Note: Not exposed to Blueprint due to TPair incompatibility with UHT
    static void GetConflictingBindings(TArray<TPair<FName, FName>> &OutConflicts);

    UFUNCTION(BlueprintCallable, Category = "Input Binding|Validation")
    static bool IsKeyBound(const FKey &Key);

    // Import / Export
    UFUNCTION(BlueprintCallable, Category = "Input Binding|IO")
    static bool ExportProfile(const FString &ProfileName, const FString &ExportPath);

    UFUNCTION(BlueprintCallable, Category = "Input Binding|IO")
    static bool ImportProfile(const FString &ImportPath, FString &OutProfileName);

    // Helper Functions
    UFUNCTION(BlueprintPure, Category = "Input Binding|Utility")
    static UCPP_InputBindingManager *GetInputBindingManager();

    UFUNCTION(BlueprintPure, Category = "Input Binding|Utility")
    static FString GetProfileDirectory();

    UFUNCTION(BlueprintCallable, Category = "Input Binding|Utility")
    static bool ProfileExists(const FString &ProfileName);

private:
    static UCPP_InputBindingManager *GetManager();
};
