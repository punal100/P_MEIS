
#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "InputBinding/FS_InputProfile.h"
#include "InputBinding/FS_InputActionBinding.h"
#include "InputBinding/FS_InputAxisBinding.h"
#include "CPP_BPL_InputBinding.generated.h"

class UCPP_InputBindingManager;
class UCPP_EnhancedInputIntegration;
class UInputAction;
class UInputMappingContext;
class APlayerController;
class UAsyncAction_WaitForInputAction;

/**
 * Public Blueprint Function Library for Input Binding operations
 * All Dynamic/Enhanced Input functions now route through Manager for multi-player support
 */
UCLASS()
class P_MEIS_API UCPP_BPL_InputBinding : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    // ==================== Conflict Detection (Per-Player) ====================

    /**
     * Get conflicting bindings for a specific player
     * @param PlayerController The player to check
     * @param OutConflicts Array to fill with conflict structs
     */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Validation")
    static void GetConflictingBindings(APlayerController *PlayerController, TArray<FInputBindingConflict> &OutConflicts);

    // ==================== Profile Management ====================

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

    UFUNCTION(BlueprintCallable, Category = "Input Binding|Profile", meta = (DeprecatedFunction, DeprecationMessage = "Use ApplyTemplateToPlayer with 'Default' template instead"))
    static bool ResetToDefaults();

    UFUNCTION(BlueprintCallable, Category = "Input Binding|Profile", meta = (DeprecatedFunction, DeprecationMessage = "Use GetAvailableProfileTemplates instead"))
    static void GetAvailableProfiles(TArray<FString> &OutProfiles);

    // ==================== Action Binding (DEPRECATED - Use Per-Player versions) ====================

    UFUNCTION(BlueprintCallable, Category = "Input Binding|Legacy", meta = (DeprecatedFunction, DeprecationMessage = "Global profiles removed. Use per-player profile operations instead."))
    static bool SetActionBinding(const FName &ActionName, const FS_InputActionBinding &Binding);

    UFUNCTION(BlueprintCallable, Category = "Input Binding|Legacy", meta = (DeprecatedFunction, DeprecationMessage = "Global profiles removed. Use per-player profile operations instead."))
    static bool RemoveActionBinding(const FName &ActionName);

    UFUNCTION(BlueprintCallable, Category = "Input Binding|Legacy", meta = (DeprecatedFunction, DeprecationMessage = "Global profiles removed. Use per-player profile operations instead."))
    static bool GetActionBinding(const FName &ActionName, FS_InputActionBinding &OutBinding);

    UFUNCTION(BlueprintCallable, Category = "Input Binding|Legacy", meta = (DeprecatedFunction, DeprecationMessage = "Global profiles removed. Use per-player profile operations instead."))
    static void GetActionBindings(TArray<FS_InputActionBinding> &OutBindings);

    UFUNCTION(BlueprintCallable, Category = "Input Binding|Legacy", meta = (DeprecatedFunction, DeprecationMessage = "Global profiles removed. Use per-player profile operations instead."))
    static bool ClearActionBindings();

    // ==================== Axis Binding (DEPRECATED - Use Per-Player versions) ====================

    UFUNCTION(BlueprintCallable, Category = "Input Binding|Legacy", meta = (DeprecatedFunction, DeprecationMessage = "Global profiles removed. Use per-player profile operations instead."))
    static bool SetAxisBinding(const FName &AxisName, const FS_InputAxisBinding &Binding);

    UFUNCTION(BlueprintCallable, Category = "Input Binding|Legacy", meta = (DeprecatedFunction, DeprecationMessage = "Global profiles removed. Use per-player profile operations instead."))
    static bool RemoveAxisBinding(const FName &AxisName);

    UFUNCTION(BlueprintCallable, Category = "Input Binding|Legacy", meta = (DeprecatedFunction, DeprecationMessage = "Global profiles removed. Use per-player profile operations instead."))
    static bool GetAxisBinding(const FName &AxisName, FS_InputAxisBinding &OutBinding);

    UFUNCTION(BlueprintCallable, Category = "Input Binding|Legacy", meta = (DeprecatedFunction, DeprecationMessage = "Global profiles removed. Use per-player profile operations instead."))
    static void GetAxisBindings(TArray<FS_InputAxisBinding> &OutBindings);

    UFUNCTION(BlueprintCallable, Category = "Input Binding|Legacy", meta = (DeprecatedFunction, DeprecationMessage = "Global profiles removed. Use per-player profile operations instead."))
    static bool ClearAxisBindings();

    // ==================== Validation ====================

    UFUNCTION(BlueprintCallable, Category = "Input Binding|Validation")
    static bool ValidateBinding(const FS_InputActionBinding &Binding, FString &OutErrorMessage);

    UFUNCTION(BlueprintCallable, Category = "Input Binding|Validation", meta = (DeprecatedFunction, DeprecationMessage = "Use IsKeyBoundForPlayer instead"))
    static bool IsKeyBound(const FKey &Key);

    // ==================== Import / Export ====================

    UFUNCTION(BlueprintCallable, Category = "Input Binding|IO")
    static bool ExportProfile(const FString &ProfileName, const FString &ExportPath);

    UFUNCTION(BlueprintCallable, Category = "Input Binding|IO")
    static bool ImportProfile(const FString &ImportPath, FString &OutProfileName);

    // ==================== Key Utility Functions ====================

    /** Convert a string to FKey (e.g., "SpaceBar", "W", "Gamepad_FaceButton_Bottom") */
    UFUNCTION(BlueprintPure, Category = "Input Binding|Key Utility", meta = (Keywords = "string to key convert"))
    static FKey StringToKey(const FString &KeyString);

    /** Convert FKey to string */
    UFUNCTION(BlueprintPure, Category = "Input Binding|Key Utility")
    static FString KeyToString(const FKey &Key);

    /** Check if a key string is valid */
    UFUNCTION(BlueprintPure, Category = "Input Binding|Key Utility")
    static bool IsValidKeyString(const FString &KeyString);

    /** Get all available key names as strings */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Key Utility")
    static void GetAllKeyNames(TArray<FString> &OutKeyNames);

    /** Create a key binding struct from a key string */
    UFUNCTION(BlueprintPure, Category = "Input Binding|Key Utility")
    static FS_KeyBinding MakeKeyBinding(const FString &KeyString, bool bShift = false, bool bCtrl = false, bool bAlt = false, bool bCmd = false);

    /** Create an axis key binding struct from a key string */
    UFUNCTION(BlueprintPure, Category = "Input Binding|Key Utility")
    static FS_AxisKeyBinding MakeAxisKeyBinding(const FString &KeyString, float Scale = 1.0f);

    // ==================== Player Management (Multi-Player Support) ====================

    /**
     * Register a player and initialize their Enhanced Input Integration
     * This is the entry point for using P_MEIS with a player
     * @param PlayerController The player controller to register
     * @return The Integration instance for this player
     */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Player", meta = (WorldContext = "WorldContextObject"))
    static UCPP_EnhancedInputIntegration *InitializeEnhancedInputIntegration(UObject *WorldContextObject, APlayerController *PlayerController);

    /**
     * Get the Integration for a specific player
     * @param PlayerController The player controller
     * @return The Integration instance, or nullptr if not registered
     */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Player")
    static UCPP_EnhancedInputIntegration *GetIntegrationForPlayer(APlayerController *PlayerController);

    /**
     * Unregister a player and clean up their Integration
     * Call this when a player leaves or is destroyed
     * @param PlayerController The player controller to unregister
     */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Player")
    static void UnregisterPlayer(APlayerController *PlayerController);

    /**
     * Check if a player is registered
     * @param PlayerController The player controller to check
     * @return True if registered
     */
    UFUNCTION(BlueprintPure, Category = "Input Binding|Player")
    static bool IsPlayerRegistered(APlayerController *PlayerController);

    // ==================== UI / Virtual Device Injection ====================

    /** Inject an action as Started (press down) for the given local player. */
    UFUNCTION(BlueprintCallable, Category = "P_MEIS|Injection")
    static void InjectActionStarted(APlayerController *PlayerController, const FName &ActionName);

    /** Inject an action as Triggered (held / repeating) for the given local player. */
    UFUNCTION(BlueprintCallable, Category = "P_MEIS|Injection")
    static void InjectActionTriggered(APlayerController *PlayerController, const FName &ActionName);

    /** Inject an action as Completed (release) for the given local player. */
    UFUNCTION(BlueprintCallable, Category = "P_MEIS|Injection")
    static void InjectActionCompleted(APlayerController *PlayerController, const FName &ActionName);

    /** Inject a 2D axis value (e.g., virtual joystick) for the given local player. */
    UFUNCTION(BlueprintCallable, Category = "P_MEIS|Injection")
    static void InjectAxis2D(APlayerController *PlayerController, const FName &AxisName, const FVector2D &Value);

    // ==================== Dynamic Input Action Creation (Multi-Player) ====================

    /**
     * Create a dynamic Input Action at runtime for a specific player
     * @param PlayerController The player to create the action for
     * @param ActionName Name of the action
     * @param bIsAxis True for axis input (analog), false for button input
     * @return The created UInputAction
     */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Dynamic")
    static UInputAction *CreateDynamicInputAction(APlayerController *PlayerController, const FName &ActionName, bool bIsAxis = false);

    /**
     * Get a dynamically created Input Action by name
     * @param PlayerController The player who owns the action
     * @param ActionName Name of the action
     * @return The UInputAction or nullptr
     */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Dynamic")
    static UInputAction *GetDynamicInputAction(APlayerController *PlayerController, const FName &ActionName);

    /**
     * Map a key to a dynamic action for a specific player
     * @param PlayerController The player
     * @param ActionName Name of the action
     * @param Key The FKey to map
     * @return True if successful
     */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Dynamic")
    static bool MapKeyToDynamicAction(APlayerController *PlayerController, const FName &ActionName, const FKey &Key);

    /**
     * Map a key binding (with modifiers) to a dynamic action for a specific player
     * @param PlayerController The player
     * @param ActionName Name of the action
     * @param KeyBinding The key binding struct with modifiers
     * @return True if successful
     */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Dynamic")
    static bool MapKeyBindingToDynamicAction(APlayerController *PlayerController, const FName &ActionName, const FS_KeyBinding &KeyBinding);

    /**
     * Map a key string with modifiers to a dynamic action for a specific player
     * @param PlayerController The player
     * @param ActionName Name of the action
     * @param KeyString The key name as string (e.g., "SpaceBar", "W")
     * @param bShift Require Shift modifier
     * @param bCtrl Require Ctrl modifier
     * @param bAlt Require Alt modifier
     * @param bCmd Require Cmd modifier (Mac)
     * @return True if successful
     */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Dynamic", meta = (AdvancedDisplay = "bShift,bCtrl,bAlt,bCmd"))
    static bool MapKeyStringWithModifiers(APlayerController *PlayerController, const FName &ActionName,
                                          const FString &KeyString, bool bShift = false, bool bCtrl = false,
                                          bool bAlt = false, bool bCmd = false);

    /**
     * Map a key string to a dynamic action for a specific player
     * @param PlayerController The player
     * @param ActionName Name of the action
     * @param KeyString The key name as string (e.g., "SpaceBar", "W")
     * @return True if successful
     */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Dynamic")
    static bool MapKeyStringToDynamicAction(APlayerController *PlayerController, const FName &ActionName, const FString &KeyString);

    /**
     * Apply the current profile to a specific player
     * @param PlayerController The player to apply the profile to
     * @return True if successful
     */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Dynamic", meta = (DeprecatedFunction, DeprecationMessage = "Use ApplyTemplateToPlayer instead"))
    static bool ApplyProfileToPlayer(APlayerController *PlayerController);

    /**
     * Apply the current profile to all registered players
     * @return True if all players were updated successfully
     */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Dynamic", meta = (DeprecatedFunction, DeprecationMessage = "Use ApplyTemplateToPlayer for each player instead"))
    static bool ApplyProfileToAllPlayers();

    // ==================== Per-Player Profile Operations ====================

    /**
     * Load a profile template and apply it to a specific player
     * Each player has their OWN profile - this copies the template to their profile
     * @param PlayerController The player
     * @param TemplateName Name of the template to load
     * @return True if successful
     */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Player Profile")
    static bool LoadProfileForPlayer(APlayerController *PlayerController, const FString &TemplateName);

    /**
     * Save a player's current profile as a template
     * @param PlayerController The player whose profile to save
     * @param TemplateName Name for the template
     * @return True if successful
     */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Player Profile")
    static bool SaveProfileForPlayer(APlayerController *PlayerController, const FString &TemplateName);

    /**
     * Get a player's current profile
     * @param PlayerController The player
     * @return The profile (copy)
     */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Player Profile")
    static FS_InputProfile GetProfileForPlayer(APlayerController *PlayerController);

    /**
     * Apply a template to a player (copies template to their active profile)
     * @param PlayerController The player
     * @param TemplateName Name of the template
     * @return True if successful
     */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Player Profile")
    static bool ApplyTemplateToPlayer(APlayerController *PlayerController, const FString &TemplateName);

    // ==================== Profile Template Management (Global Library) ====================

    /**
     * Get list of available profile templates
     * @param OutTemplates Array to fill with template names
     */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Templates")
    static void GetAvailableProfileTemplates(TArray<FString> &OutTemplates);

    /**
     * Create a new empty profile template
     * @param TemplateName Name for the template
     * @return True if successful
     */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Templates")
    static bool CreateProfileTemplate(const FString &TemplateName);

    /**
     * Delete a profile template
     * @param TemplateName Name of the template to delete
     * @return True if successful
     */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Templates")
    static bool DeleteProfileTemplate(const FString &TemplateName);

    /**
     * Check if a template exists
     * @param TemplateName Name of the template
     * @return True if exists
     */
    UFUNCTION(BlueprintPure, Category = "Input Binding|Templates")
    static bool DoesTemplateExist(const FString &TemplateName);

    /**
     * Get the number of loaded templates
     * @return Number of templates
     */
    UFUNCTION(BlueprintPure, Category = "Input Binding|Templates")
    static int32 GetTemplateCount();

    // ==================== Per-Player Action Binding Helpers ====================

    /**
     * Check if an action exists in a player's profile
     * @param PlayerController The player
     * @param ActionName Name of the action
     * @return True if action exists
     */
    UFUNCTION(BlueprintPure, Category = "Input Binding|Player Action")
    static bool DoesActionExist(APlayerController *PlayerController, const FName &ActionName);

    /**
     * Get all keys bound to an action for a player
     * @param PlayerController The player
     * @param ActionName Name of the action
     * @return Array of bound keys
     */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Player Action")
    static TArray<FKey> GetKeysForAction(APlayerController *PlayerController, const FName &ActionName);

    /**
     * Get the primary (first) key bound to an action
     * @param PlayerController The player
     * @param ActionName Name of the action
     * @return The primary key, or Invalid if not found
     */
    UFUNCTION(BlueprintPure, Category = "Input Binding|Player Action")
    static FKey GetPrimaryKeyForAction(APlayerController *PlayerController, const FName &ActionName);

    /**
     * Get all actions that use a specific key (reverse lookup)
     * @param PlayerController The player
     * @param Key The key to look up
     * @return Array of action names
     */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Player Action")
    static TArray<FName> GetActionsForKey(APlayerController *PlayerController, const FKey &Key);

    /**
     * Set/Replace the primary key for an action (index 0)
     * @param PlayerController The player
     * @param ActionName Name of the action
     * @param Key The key to set
     * @return True if successful
     */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Player Action")
    static bool SetPrimaryKeyForAction(APlayerController *PlayerController, const FName &ActionName, const FKey &Key);

    /**
     * Add a key to an existing action (append)
     * @param PlayerController The player
     * @param ActionName Name of the action
     * @param Key The key to add
     * @return True if successful
     */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Player Action")
    static bool AddKeyToAction(APlayerController *PlayerController, const FName &ActionName, const FKey &Key);

    /**
     * Remove a specific key from an action
     * @param PlayerController The player
     * @param ActionName Name of the action
     * @param Key The key to remove
     * @return True if removed
     */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Player Action")
    static bool RemoveKeyFromAction(APlayerController *PlayerController, const FName &ActionName, const FKey &Key);

    /**
     * Clear all keys from an action (keeps action, removes all bindings)
     * @param PlayerController The player
     * @param ActionName Name of the action
     * @return True if successful
     */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Player Action")
    static bool ClearActionKeys(APlayerController *PlayerController, const FName &ActionName);

    /**
     * Check if a key is bound to any action for a player
     * @param PlayerController The player
     * @param Key The key to check
     * @return True if bound
     */
    UFUNCTION(BlueprintPure, Category = "Input Binding|Player Action")
    static bool IsKeyBoundForPlayer(APlayerController *PlayerController, const FKey &Key);

    // ==================== Per-Player Axis Binding Helpers ====================

    /**
     * Get the sensitivity for an axis binding
     * @param PlayerController The player
     * @param AxisName Name of the axis
     * @return Sensitivity value (1.0 default if not found)
     */
    UFUNCTION(BlueprintPure, Category = "Input Binding|Player Axis")
    static float GetAxisSensitivity(APlayerController *PlayerController, const FName &AxisName);

    /**
     * Set the sensitivity for an axis binding
     * @param PlayerController The player
     * @param AxisName Name of the axis
     * @param Sensitivity New sensitivity value
     * @return True if successful
     */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Player Axis")
    static bool SetAxisSensitivity(APlayerController *PlayerController, const FName &AxisName, float Sensitivity);

    /**
     * Get the dead zone for an axis binding
     * @param PlayerController The player
     * @param AxisName Name of the axis
     * @return DeadZone value (0.2 default if not found)
     */
    UFUNCTION(BlueprintPure, Category = "Input Binding|Player Axis")
    static float GetAxisDeadZone(APlayerController *PlayerController, const FName &AxisName);

    /**
     * Set the dead zone for an axis binding
     * @param PlayerController The player
     * @param AxisName Name of the axis
     * @param DeadZone New dead zone value
     * @return True if successful
     */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Player Axis")
    static bool SetAxisDeadZone(APlayerController *PlayerController, const FName &AxisName, float DeadZone);

    // ==================== Dynamic Input Action Helpers ====================

    /**
     * Check if a dynamic Input Action exists for a player
     * @param PlayerController The player
     * @param ActionName Name of the action
     * @return True if exists
     */
    UFUNCTION(BlueprintPure, Category = "Input Binding|Dynamic")
    static bool DoesInputActionExist(APlayerController *PlayerController, const FName &ActionName);

    /**
     * Get the Input Mapping Context for a player
     * @param PlayerController The player
     * @return The mapping context, or nullptr
     */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Dynamic")
    static UInputMappingContext *GetInputMappingContext(APlayerController *PlayerController);

    // ==================== Player Info Helpers ====================

    /**
     * Get the number of registered players
     * @return Number of registered players
     */
    UFUNCTION(BlueprintPure, Category = "Input Binding|Player")
    static int32 GetRegisteredPlayerCount();

    /**
     * Get the template name that a player loaded from
     * @param PlayerController The player
     * @return Template name, or empty string if not loaded from template
     */
    UFUNCTION(BlueprintPure, Category = "Input Binding|Player")
    static FString GetPlayerLoadedTemplateName(APlayerController *PlayerController);

    /**
     * Check if a player has modified their profile since loading
     * @param PlayerController The player
     * @return True if profile appears modified
     */
    UFUNCTION(BlueprintPure, Category = "Input Binding|Player")
    static bool HasPlayerModifiedProfile(APlayerController *PlayerController);

    // ==================== Key Utility Functions (Extended) ====================

    /**
     * Get all keyboard keys
     * @return Array of all keyboard keys
     */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Key Utility")
    static TArray<FKey> GetAllKeyboardKeys();

    /**
     * Get all gamepad keys/buttons
     * @return Array of all gamepad keys
     */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Key Utility")
    static TArray<FKey> GetAllGamepadKeys();

    /**
     * Get all mouse keys/buttons
     * @return Array of all mouse keys
     */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Key Utility")
    static TArray<FKey> GetAllMouseKeys();

    /**
     * Convert an FKey to a display-friendly string
     * @param Key The key to convert
     * @return Display string (e.g., "Space Bar", "Left Mouse Button")
     */
    UFUNCTION(BlueprintPure, Category = "Input Binding|Key Utility")
    static FString KeyToDisplayString(const FKey &Key);

    // ==================== Async Action Utilities ====================

    /**
     * Stop an async "Wait For Input Action" node from listening
     * Call this to manually cancel an async action that is waiting for input events
     * @param AsyncAction The async action to cancel (returned from Wait For Input Action node)
     */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Async")
    static void StopWaitingForInputAction(UAsyncAction_WaitForInputAction *AsyncAction);

    // ==================== Section 0.9: Dynamic Modifiers & Triggers ====================

    /**
     * Set the dead zone for an action's modifiers
     * @param PlayerController The player
     * @param ActionName Name of the action
     * @param LowerThreshold Input below this is ignored (0.0-1.0)
     * @param UpperThreshold Input above this is clamped to 1.0 (default 1.0)
     * @return True if successful
     */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Modifiers")
    static bool SetActionDeadZone(APlayerController *PlayerController, const FName &ActionName, float LowerThreshold, float UpperThreshold = 1.0f);

    /**
     * Set the sensitivity (scale) for an action
     * @param PlayerController The player
     * @param ActionName Name of the action
     * @param Sensitivity Multiplier for input values
     * @return True if successful
     */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Modifiers")
    static bool SetActionSensitivity(APlayerController *PlayerController, const FName &ActionName, float Sensitivity);

    /**
     * Set sensitivity per axis for an action
     * @param PlayerController The player
     * @param ActionName Name of the action
     * @param Sensitivity Multiplier per axis (X, Y, Z)
     * @return True if successful
     */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Modifiers")
    static bool SetActionSensitivityPerAxis(APlayerController *PlayerController, const FName &ActionName, FVector Sensitivity);

    /**
     * Set invert Y axis for an action (common for "Invert Look" option)
     * @param PlayerController The player
     * @param ActionName Name of the action
     * @param bInvert Whether to invert the Y axis
     * @return True if successful
     */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Modifiers")
    static bool SetActionInvertY(APlayerController *PlayerController, const FName &ActionName, bool bInvert);

    /**
     * Set a hold trigger on a key mapping (requires holding for duration)
     * @param PlayerController The player
     * @param ActionName Name of the action
     * @param Key The key to modify
     * @param HoldTime How long to hold before triggering (seconds)
     * @return True if successful
     */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Triggers")
    static bool SetKeyHoldTrigger(APlayerController *PlayerController, const FName &ActionName, const FKey &Key, float HoldTime);

    /**
     * Set a tap trigger on a key mapping (quick press and release)
     * @param PlayerController The player
     * @param ActionName Name of the action
     * @param Key The key to modify
     * @param MaxTapTime Maximum time between press and release for tap (seconds)
     * @return True if successful
     */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Triggers")
    static bool SetKeyTapTrigger(APlayerController *PlayerController, const FName &ActionName, const FKey &Key, float MaxTapTime);

    /**
     * Clear all triggers from a key mapping (revert to default behavior)
     * @param PlayerController The player
     * @param ActionName Name of the action
     * @param Key The key to modify
     * @return True if successful
     */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Triggers")
    static bool ClearKeyTriggers(APlayerController *PlayerController, const FName &ActionName, const FKey &Key);

    /**
     * Clear all modifiers from an action
     * @param PlayerController The player
     * @param ActionName Name of the action
     * @return True if successful
     */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Modifiers")
    static bool ClearActionModifiers(APlayerController *PlayerController, const FName &ActionName);

    // ==================== Helper Functions ======================================

    UFUNCTION(BlueprintPure, Category = "Input Binding|Utility")
    static UCPP_InputBindingManager *GetInputBindingManager();

    UFUNCTION(BlueprintPure, Category = "Input Binding|Utility")
    static FString GetProfileDirectory();

    UFUNCTION(BlueprintCallable, Category = "Input Binding|Utility")
    static bool ProfileExists(const FString &ProfileName);

private:
    static UCPP_InputBindingManager *GetManager();
};
