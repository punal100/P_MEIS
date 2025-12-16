/*
 * @Author: Punal Manalan
 * @Description: Input Binding Manager - Core subsystem for managing dynamic input bindings
 *               Supports multi-player with per-PlayerController Profile AND Integration instances
 *               Each player has their OWN key bindings and runtime Enhanced Input objects
 * @Date: 06/12/2025
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "InputBinding/FS_InputActionBinding.h"
#include "InputBinding/FS_InputAxisBinding.h"
#include "InputBinding/FS_InputProfile.h"
#include "InputBinding/FS_PlayerInputData.h"
#include "CPP_InputBindingManager.generated.h"

class UCPP_EnhancedInputIntegration;
class APlayerController;
class AController;

/**
 * Core input binding manager subsystem
 *
 * ARCHITECTURE:
 * - ProfileTemplates: Global library of saved profiles (on disk)
 * - PlayerDataMap: Per-player data (each player has OWN profile + integration)
 *
 * Each PlayerController gets:
 * - Their own FS_InputProfile (ActiveProfile) - their key bindings
 * - Their own UCPP_EnhancedInputIntegration - runtime IA/IMC objects
 *
 * Profile templates are shared presets that can be COPIED to players.
 * Players can customize their bindings independently.
 */
UCLASS()
class P_MEIS_API UCPP_InputBindingManager : public UEngineSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase &Collection) override;
    virtual void Deinitialize() override;

    // ==================== Player Management (Per-Player Data) ====================

    /**
     * Register a player and create their data (Profile + Integration)
     * @param PlayerController The player controller to register
     * @return The Integration instance for this player
     */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Player")
    UCPP_EnhancedInputIntegration *RegisterPlayer(APlayerController *PlayerController);

    // ==================== Controller Management (AI + Non-Player Controllers) ====================

    /**
     * Register a controller (e.g., AIController) and create its data (Profile + Integration)
     * Note: Mapping contexts are only applied for local player controllers.
     */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Controller")
    UCPP_EnhancedInputIntegration *RegisterController(AController *Controller);

    /** Unregister a controller and clean up its data */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Controller")
    void UnregisterController(AController *Controller);

    /** Get the Integration for a controller (lazy init if needed) */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Controller")
    UCPP_EnhancedInputIntegration *GetIntegrationForController(AController *Controller);

    /**
     * Unregister a player and clean up their data
     * @param PlayerController The player controller to unregister
     */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Player")
    void UnregisterPlayer(APlayerController *PlayerController);

    /**
     * Get the Integration for a specific player (lazy init if needed)
     * @param PlayerController The player controller
     * @return The Integration instance for this player
     */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Player")
    UCPP_EnhancedInputIntegration *GetIntegrationForPlayer(APlayerController *PlayerController);

    /**
     * Get the Profile for a specific player
     * @param PlayerController The player controller
     * @return The Profile struct for this player (copy)
     */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Player")
    FS_InputProfile GetProfileForPlayer(APlayerController *PlayerController);

    /**
     * Get reference to player's profile (for modification)
     * @param PlayerController The player controller
     * @return Pointer to the profile (nullptr if not registered)
     */
    FS_InputProfile *GetProfileRefForPlayer(APlayerController *PlayerController);

    /**
     * Check if a player is registered
     * @param PlayerController The player controller to check
     * @return True if registered
     */
    UFUNCTION(BlueprintPure, Category = "Input Binding|Player")
    bool HasPlayerRegistered(APlayerController *PlayerController) const;

    /**
     * Get all registered player controllers
     * @param OutPlayers Array to fill with registered players
     */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Player")
    void GetRegisteredPlayers(TArray<APlayerController *> &OutPlayers) const;

    /**
     * Get the number of registered players
     * @return Number of registered players
     */
    UFUNCTION(BlueprintPure, Category = "Input Binding|Player")
    int32 GetRegisteredPlayerCount() const;

    /**
     * Get the template name that a player loaded from
     * @param PlayerController The player
     * @return The template name, or NAME_None if not loaded from template
     */
    UFUNCTION(BlueprintPure, Category = "Input Binding|Player")
    FName GetPlayerLoadedTemplateName(APlayerController *PlayerController) const;

    /**
     * Check if a player has modified their profile since loading from template
     * Note: Currently tracks if LoadedTemplateName differs from profile name
     * @param PlayerController The player
     * @return True if profile appears modified
     */
    UFUNCTION(BlueprintPure, Category = "Input Binding|Player")
    bool HasPlayerModifiedProfile(APlayerController *PlayerController) const;

    // ==================== Profile Template Management (Global Library) ====================

    /**
     * Load a profile template from disk into memory
     * @param TemplateName Name of the template to load
     * @return True if successful
     */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Templates")
    bool LoadProfileTemplate(const FName &TemplateName);

    /**
     * Save a profile template to disk
     * @param TemplateName Name for the template
     * @param Profile The profile data to save
     * @return True if successful
     */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Templates")
    bool SaveProfileTemplate(const FName &TemplateName, const FS_InputProfile &Profile);

    /**
     * Create a new empty profile template
     * @param TemplateName Name for the new template
     * @param Description Optional description
     * @return True if successful
     */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Templates")
    bool CreateProfileTemplate(const FName &TemplateName, const FText &Description = FText::GetEmpty());

    /**
     * Delete a profile template from disk
     * @param TemplateName Name of the template to delete
     * @return True if successful
     */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Templates")
    bool DeleteProfileTemplate(const FName &TemplateName);

    /**
     * Get list of available profile templates
     * @param OutTemplates Array to fill with template names
     */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Templates")
    void GetAvailableTemplates(TArray<FName> &OutTemplates);

    /**
     * Check if a template exists
     * @param TemplateName Name of the template
     * @return True if template exists
     */
    UFUNCTION(BlueprintPure, Category = "Input Binding|Templates")
    bool HasTemplate(const FName &TemplateName) const;

    /**
     * Check if a template exists (alias for HasTemplate)
     * @param TemplateName Name of the template
     * @return True if template exists
     */
    UFUNCTION(BlueprintPure, Category = "Input Binding|Templates")
    bool DoesTemplateExist(const FName &TemplateName) const;

    /**
     * Get the number of loaded templates
     * @return Number of templates
     */
    UFUNCTION(BlueprintPure, Category = "Input Binding|Templates")
    int32 GetTemplateCount() const;

    /**
     * Get a template by name (returns copy)
     * @param TemplateName Name of the template
     * @param OutProfile Profile to fill
     * @return True if found
     */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Templates")
    bool GetTemplate(const FName &TemplateName, FS_InputProfile &OutProfile) const;

    // ==================== Per-Player Profile Operations ====================

    /**
     * Apply a template to a specific player (copies template to player's active profile)
     * @param PlayerController The player to apply to
     * @param TemplateName Name of the template to apply
     * @return True if successful
     */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Player Profile")
    bool ApplyTemplateToPlayer(APlayerController *PlayerController, const FName &TemplateName);

    /**
     * Save a player's current profile as a template
     * @param PlayerController The player whose profile to save
     * @param TemplateName Name for the template
     * @return True if successful
     */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Player Profile")
    bool SavePlayerProfileAsTemplate(APlayerController *PlayerController, const FName &TemplateName);

    /**
     * Apply the player's profile to their Enhanced Input system
     * @param PlayerController The player
     * @return True if successful
     */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Player Profile")
    bool ApplyPlayerProfileToEnhancedInput(APlayerController *PlayerController);

    // ==================== Per-Player Action Binding Operations ====================

    /**
     * Set an action binding for a specific player
     * @param PlayerController The player
     * @param ActionName Name of the action
     * @param Binding The binding configuration
     * @return True if successful
     */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Player Action")
    bool SetPlayerActionBinding(APlayerController *PlayerController, const FName &ActionName, const FS_InputActionBinding &Binding);

    /**
     * Get an action binding for a specific player
     * @param PlayerController The player
     * @param ActionName Name of the action
     * @param OutBinding Output binding
     * @return True if found
     */
    UFUNCTION(BlueprintPure, Category = "Input Binding|Player Action")
    bool GetPlayerActionBinding(APlayerController *PlayerController, const FName &ActionName, FS_InputActionBinding &OutBinding);

    /**
     * Remove an action binding for a specific player
     * @param PlayerController The player
     * @param ActionName Name of the action to remove
     * @return True if removed
     */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Player Action")
    bool RemovePlayerActionBinding(APlayerController *PlayerController, const FName &ActionName);

    /**
     * Get all action bindings for a specific player
     * @param PlayerController The player
     * @param OutBindings Array to fill
     */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Player Action")
    void GetPlayerActionBindings(APlayerController *PlayerController, TArray<FS_InputActionBinding> &OutBindings);

    /**
     * Check if an action exists in a player's profile
     * @param PlayerController The player
     * @param ActionName Name of the action
     * @return True if action exists
     */
    UFUNCTION(BlueprintPure, Category = "Input Binding|Player Action")
    bool DoesActionExist(APlayerController *PlayerController, const FName &ActionName) const;

    /**
     * Get all keys bound to an action for a player
     * @param PlayerController The player
     * @param ActionName Name of the action
     * @param OutKeys Array to fill with bound keys
     */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Player Action")
    void GetKeysForAction(APlayerController *PlayerController, const FName &ActionName, TArray<FKey> &OutKeys);

    /**
     * Get the primary (first) key bound to an action
     * @param PlayerController The player
     * @param ActionName Name of the action
     * @return The primary key, or EKeys::Invalid if not found
     */
    UFUNCTION(BlueprintPure, Category = "Input Binding|Player Action")
    FKey GetPrimaryKeyForAction(APlayerController *PlayerController, const FName &ActionName);

    /**
     * Get all actions that use a specific key (reverse lookup)
     * @param PlayerController The player
     * @param Key The key to look up
     * @param OutActions Array to fill with action names
     */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Player Action")
    void GetActionsForKey(APlayerController *PlayerController, const FKey &Key, TArray<FName> &OutActions);

    /**
     * Get the number of keys bound to an action
     * @param PlayerController The player
     * @param ActionName Name of the action
     * @return Number of keys bound
     */
    UFUNCTION(BlueprintPure, Category = "Input Binding|Player Action")
    int32 GetKeyCountForAction(APlayerController *PlayerController, const FName &ActionName) const;

    /**
     * Set/Replace the primary key for an action (index 0)
     * @param PlayerController The player
     * @param ActionName Name of the action
     * @param Key The key to set
     * @return True if successful
     */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Player Action")
    bool SetPrimaryKeyForAction(APlayerController *PlayerController, const FName &ActionName, const FKey &Key);

    /**
     * Add a key to an existing action (append)
     * @param PlayerController The player
     * @param ActionName Name of the action
     * @param Key The key to add
     * @return True if successful
     */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Player Action")
    bool AddKeyToAction(APlayerController *PlayerController, const FName &ActionName, const FKey &Key);

    /**
     * Remove a specific key from an action
     * @param PlayerController The player
     * @param ActionName Name of the action
     * @param Key The key to remove
     * @return True if removed
     */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Player Action")
    bool RemoveKeyFromAction(APlayerController *PlayerController, const FName &ActionName, const FKey &Key);

    /**
     * Clear all keys from an action (keeps action, removes all bindings)
     * @param PlayerController The player
     * @param ActionName Name of the action
     * @return True if successful
     */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Player Action")
    bool ClearActionKeys(APlayerController *PlayerController, const FName &ActionName);

    /**
     * Swap key bindings between two actions
     * @param PlayerController The player
     * @param ActionA First action
     * @param ActionB Second action
     * @return True if successful
     */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Player Action")
    bool SwapActionBindings(APlayerController *PlayerController, const FName &ActionA, const FName &ActionB);

    // ==================== Per-Player Axis Binding Operations ====================

    /**
     * Set an axis binding for a specific player
     * @param PlayerController The player
     * @param AxisName Name of the axis
     * @param Binding The binding configuration
     * @return True if successful
     */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Player Axis")
    bool SetPlayerAxisBinding(APlayerController *PlayerController, const FName &AxisName, const FS_InputAxisBinding &Binding);

    /**
     * Get an axis binding for a specific player
     * @param PlayerController The player
     * @param AxisName Name of the axis
     * @param OutBinding Output binding
     * @return True if found
     */
    UFUNCTION(BlueprintPure, Category = "Input Binding|Player Axis")
    bool GetPlayerAxisBinding(APlayerController *PlayerController, const FName &AxisName, FS_InputAxisBinding &OutBinding);

    /**
     * Remove an axis binding for a specific player
     * @param PlayerController The player
     * @param AxisName Name of the axis to remove
     * @return True if removed
     */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Player Axis")
    bool RemovePlayerAxisBinding(APlayerController *PlayerController, const FName &AxisName);

    /**
     * Get all axis bindings for a specific player
     * @param PlayerController The player
     * @param OutBindings Array to fill
     */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Player Axis")
    void GetPlayerAxisBindings(APlayerController *PlayerController, TArray<FS_InputAxisBinding> &OutBindings);

    /**
     * Get the sensitivity for an axis binding
     * @param PlayerController The player
     * @param AxisName Name of the axis
     * @return Sensitivity value, or 1.0 if not found
     */
    UFUNCTION(BlueprintPure, Category = "Input Binding|Player Axis")
    float GetAxisSensitivity(APlayerController *PlayerController, const FName &AxisName) const;

    /**
     * Set the sensitivity for an axis binding
     * @param PlayerController The player
     * @param AxisName Name of the axis
     * @param Sensitivity New sensitivity value
     * @return True if successful
     */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Player Axis")
    bool SetAxisSensitivity(APlayerController *PlayerController, const FName &AxisName, float Sensitivity);

    /**
     * Get the dead zone for an axis binding
     * @param PlayerController The player
     * @param AxisName Name of the axis
     * @return DeadZone value, or 0.2 (default) if not found
     */
    UFUNCTION(BlueprintPure, Category = "Input Binding|Player Axis")
    float GetAxisDeadZone(APlayerController *PlayerController, const FName &AxisName) const;

    /**
     * Set the dead zone for an axis binding
     * @param PlayerController The player
     * @param AxisName Name of the axis
     * @param DeadZone New dead zone value
     * @return True if successful
     */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Player Axis")
    bool SetAxisDeadZone(APlayerController *PlayerController, const FName &AxisName, float DeadZone);

    // ==================== Validation & Conflict Detection ====================

    UFUNCTION(BlueprintCallable, Category = "Input Binding|Validation")
    bool ValidateBinding(const FS_InputActionBinding &Binding, FString &OutErrorMessage);

    /**
     * Check if a key is bound for a specific player
     * @param PlayerController The player
     * @param Key The key to check
     * @return True if bound
     */
    UFUNCTION(BlueprintPure, Category = "Input Binding|Validation")
    bool IsKeyBoundForPlayer(APlayerController *PlayerController, const FKey &Key);

    // ==================== Import/Export ====================

    UFUNCTION(BlueprintCallable, Category = "Input Binding|IO")
    bool ExportTemplate(const FName &TemplateName, const FString &FilePath);

    UFUNCTION(BlueprintCallable, Category = "Input Binding|IO")
    bool ImportTemplate(const FString &FilePath, FName &OutTemplateName);

    // ==================== Legacy Compatibility (Deprecated - Use Per-Player versions) ====================

    // These operate on a "default" template for backwards compatibility
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Legacy", meta = (DeprecatedFunction, DeprecationMessage = "Use template or per-player functions instead"))
    bool LoadProfile(const FName &ProfileName);

    UFUNCTION(BlueprintCallable, Category = "Input Binding|Legacy", meta = (DeprecatedFunction, DeprecationMessage = "Use SaveProfileTemplate or SavePlayerProfileAsTemplate instead"))
    bool SaveProfile(const FName &ProfileName, const FText &Description = FText::GetEmpty());

    UFUNCTION(BlueprintPure, Category = "Input Binding|Legacy", meta = (DeprecatedFunction, DeprecationMessage = "Use GetTemplate or GetProfileForPlayer instead"))
    FS_InputProfile GetCurrentProfile() const;

    UFUNCTION(BlueprintPure, Category = "Input Binding|Legacy", meta = (DeprecatedFunction, DeprecationMessage = "Use GetAvailableTemplates instead"))
    void GetAvailableProfiles(TArray<FName> &OutProfiles);

    // ==================== Validation & Conflict Detection ====================

    /**
     * Get conflicting action bindings for a specific player
     * @param PlayerController The player
     * @param OutConflicts Array to fill with conflict structs
     */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Validation")
    void GetConflictingBindings(APlayerController *PlayerController, TArray<FInputBindingConflict> &OutConflicts);

protected:
    // ==================== Profile Templates (Global Library - Stored on Disk) ====================

    /** Library of profile templates that can be applied to players */
    UPROPERTY(VisibleAnywhere, Category = "Input Binding")
    TMap<FName, FS_InputProfile> ProfileTemplates;

    // ==================== Per-Player Data ====================

    /** Map of PlayerController to their input data (Profile + Integration) */
    UPROPERTY()
    TMap<APlayerController *, FS_PlayerInputData> PlayerDataMap;

    /** Map of Controller (e.g., AIController) to their input data (Profile + Integration) */
    UPROPERTY()
    TMap<AController *, FS_PlayerInputData> ControllerDataMap;

    // ==================== Helper Functions ====================

    bool LoadDefaultTemplate();
    void BroadcastBindingChanges(APlayerController *PlayerController);
    void CleanupInvalidPlayers();
    void CleanupInvalidControllers();
    FS_PlayerInputData *GetPlayerData(APlayerController *PlayerController);
    const FS_PlayerInputData *GetPlayerData(APlayerController *PlayerController) const;
};
