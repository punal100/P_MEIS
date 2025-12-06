/*
 * @Author: Punal Manalan
 * @Description: Input Binding Manager - Core subsystem for managing dynamic input bindings
 * @Date: 06/12/2025
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "InputBinding/FS_InputActionBinding.h"
#include "InputBinding/FS_InputAxisBinding.h"
#include "InputBinding/FS_InputProfile.h"
#include "CPP_InputBindingManager.generated.h"

class UCPP_EnhancedInputIntegration;

/**
 * Core input binding manager subsystem
 * Manages all dynamic input binding operations
 */
UCLASS()
class P_MEIS_API UCPP_InputBindingManager : public UEngineSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase &Collection) override;
    virtual void Deinitialize() override;

    // Profile Management
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Profile")
    bool LoadProfile(const FName &ProfileName);

    UFUNCTION(BlueprintCallable, Category = "Input Binding|Profile")
    bool SaveProfile(const FName &ProfileName, const FText &Description = FText::GetEmpty());

    UFUNCTION(BlueprintCallable, Category = "Input Binding|Profile")
    bool CreateProfile(const FName &ProfileName, const FText &Description = FText::GetEmpty());

    UFUNCTION(BlueprintCallable, Category = "Input Binding|Profile")
    bool DeleteProfile(const FName &ProfileName);

    UFUNCTION(BlueprintCallable, Category = "Input Binding|Profile")
    bool RenameProfile(const FName &OldName, const FName &NewName);

    UFUNCTION(BlueprintCallable, Category = "Input Binding|Profile")
    bool DuplicateProfile(const FName &SourceProfile, const FName &NewProfileName);

    UFUNCTION(BlueprintCallable, Category = "Input Binding|Profile")
    bool ResetToDefaults();

    // Profile Queries
    UFUNCTION(BlueprintPure, Category = "Input Binding|Profile")
    void GetAvailableProfiles(TArray<FName> &OutProfiles);

    UFUNCTION(BlueprintPure, Category = "Input Binding|Profile")
    FName GetCurrentProfileName() const { return CurrentProfile.ProfileName; }

    UFUNCTION(BlueprintPure, Category = "Input Binding|Profile")
    FS_InputProfile GetCurrentProfile() const { return CurrentProfile; }

    // Action Binding Operations
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Action")
    bool SetActionBinding(const FName &ActionName, const FS_InputActionBinding &Binding);

    UFUNCTION(BlueprintCallable, Category = "Input Binding|Action")
    bool RemoveActionBinding(const FName &ActionName);

    UFUNCTION(BlueprintCallable, Category = "Input Binding|Action")
    bool ClearActionBindings();

    UFUNCTION(BlueprintPure, Category = "Input Binding|Action")
    bool GetActionBinding(const FName &ActionName, FS_InputActionBinding &OutBinding);

    UFUNCTION(BlueprintPure, Category = "Input Binding|Action")
    void GetActionBindings(TArray<FS_InputActionBinding> &OutBindings);

    // Axis Binding Operations
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Axis")
    bool SetAxisBinding(const FName &AxisName, const FS_InputAxisBinding &AxisBinding);

    UFUNCTION(BlueprintCallable, Category = "Input Binding|Axis")
    bool RemoveAxisBinding(const FName &AxisName);

    UFUNCTION(BlueprintCallable, Category = "Input Binding|Axis")
    bool ClearAxisBindings();

    UFUNCTION(BlueprintPure, Category = "Input Binding|Axis")
    bool GetAxisBinding(const FName &AxisName, FS_InputAxisBinding &OutBinding);

    UFUNCTION(BlueprintPure, Category = "Input Binding|Axis")
    void GetAxisBindings(TArray<FS_InputAxisBinding> &OutBindings);

    // Validation & Conflict Detection
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Validation")
    bool ValidateBinding(const FS_InputActionBinding &Binding, FString &OutErrorMessage);

    // Note: Not exposed to Blueprint due to TPair incompatibility with UHT
    static void GetConflictingBindings(TArray<TPair<FName, FName>> &OutConflicts);

    UFUNCTION(BlueprintPure, Category = "Input Binding|Validation")
    bool IsKeyBound(const FKey &Key);

    // Import/Export
    UFUNCTION(BlueprintCallable, Category = "Input Binding|IO")
    bool ExportProfile(const FName &ProfileName, const FString &FilePath);

    UFUNCTION(BlueprintCallable, Category = "Input Binding|IO")
    bool ImportProfile(const FString &FilePath, FName &OutProfileName);

protected:
    // Profile Storage
    UPROPERTY(VisibleAnywhere, Category = "Input Binding")
    FS_InputProfile CurrentProfile;

    UPROPERTY(VisibleAnywhere, Category = "Input Binding")
    TMap<FName, FS_InputProfile> CachedProfiles;

    // Enhanced Input Integration
    UPROPERTY()
    UCPP_EnhancedInputIntegration *InputIntegration;

    // Helper Functions
    bool LoadDefaultProfile();
    bool ApplyProfileToEnhancedInput();
    void BroadcastBindingChanges();
};
