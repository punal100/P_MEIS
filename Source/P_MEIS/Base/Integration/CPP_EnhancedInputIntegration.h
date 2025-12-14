/*
 * @Author: Punal Manalan
 * @Description: Enhanced Input Integration - Bridges P_MEIS with UE5 Enhanced Input System
 *               Supports dynamic creation of Input Actions, Mapping Contexts, and key bindings at runtime
 *               Includes Blueprint-friendly event dispatchers for action binding
 *               Section 0.9: Added dynamic modifier and trigger management
 * @Date: 06/12/2025
 * @Version: 3.0 - Added dynamic modifier/trigger management (Section 0.9)
 */

#pragma once

#include "CoreMinimal.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "InputActionValue.h"
#include "InputBinding/FS_InputProfile.h"
#include "InputBinding/FS_InputActionBinding.h"
#include "InputBinding/FS_InputModifier.h"
#include "InputBinding/FS_InputTriggerConfig.h"
#include "CPP_EnhancedInputIntegration.generated.h"

class APlayerController;
class UEnhancedInputComponent;
class UInputMappingContext;
class UInputAction;
class UInputModifier;
class UInputTrigger;
class UInputModifierDeadZone;
class UInputModifierNegate;
class UInputModifierScalar;

// ==================== Delegate Declarations ====================

// Legacy delegate for backward compatibility
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDynamicInputAction, FName, ActionName, FInputActionValue, Value);

// New global event dispatchers (Approach A) - Per trigger type
// These fire for ALL actions, Blueprint can filter by ActionName
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInputActionEvent, FName, ActionName, FInputActionValue, Value);

// Forward declaration for async action
class UAsyncAction_WaitForInputAction;

/**
 * Integration layer with UE5 Enhanced Input System
 * Creates Input Actions, Mapping Contexts, and key bindings dynamically at runtime
 */
UCLASS(BlueprintType)
class P_MEIS_API UCPP_EnhancedInputIntegration : public UObject
{
    GENERATED_BODY()

public:
    // ==================== Profile Application ====================

    /** Apply an entire input profile - creates all Input Actions and mappings */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Integration")
    bool ApplyProfile(const FS_InputProfile &Profile);

    /** Apply a single action binding - creates Input Action if needed */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Integration")
    bool ApplyActionBinding(const FS_InputActionBinding &ActionBinding);

    /** Apply a single axis binding - creates Input Action if needed */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Integration")
    bool ApplyAxisBinding(const FS_InputAxisBinding &AxisBinding);

    // ==================== Player Controller ====================

    /** Set the player controller to apply inputs to */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Integration")
    void SetPlayerController(APlayerController *InPlayerController);

    /** Get the current player controller */
    UFUNCTION(BlueprintPure, Category = "Input Binding|Integration")
    APlayerController *GetPlayerController() const { return PlayerController; }

    // ==================== Dynamic Input Action Creation ====================

    /** Create a new Input Action dynamically at runtime */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Integration")
    UInputAction *CreateInputAction(const FName &ActionName, EInputActionValueType ValueType = EInputActionValueType::Boolean);

    /** Get an existing Input Action by name, or nullptr if not found */
    UFUNCTION(BlueprintPure, Category = "Input Binding|Integration")
    UInputAction *GetInputAction(const FName &ActionName) const;

    /** Check if an Input Action exists */
    UFUNCTION(BlueprintPure, Category = "Input Binding|Integration")
    bool HasInputAction(const FName &ActionName) const;

    /** Get all created Input Actions */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Integration")
    void GetAllInputActions(TArray<UInputAction *> &OutActions) const;

    // ==================== Key Mapping ====================

    /** Map a key to an action (creates action if needed) */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Integration")
    bool MapKeyToAction(const FName &ActionName, const FKey &Key);

    /**
     * Map a key with modifiers to an action
     * @param ActionName Name of the action
     * @param KeyBinding The key binding with modifiers (Shift, Ctrl, Alt, Cmd)
     * @return True if successful
     */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Integration")
    bool MapKeyBindingToAction(const FName &ActionName, const FS_KeyBinding &KeyBinding);

    /**
     * Map a key string with modifiers to an action
     * @param ActionName Name of the action
     * @param KeyString The key as string (e.g., "SpaceBar", "W")
     * @param bShift Require Shift modifier
     * @param bCtrl Require Ctrl modifier
     * @param bAlt Require Alt modifier
     * @param bCmd Require Cmd modifier (Mac)
     * @return True if successful
     */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Integration")
    bool MapKeyStringWithModifiers(const FName &ActionName, const FString &KeyString,
                                   bool bShift = false, bool bCtrl = false,
                                   bool bAlt = false, bool bCmd = false);

    /** Unmap a key from an action */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Integration")
    bool UnmapKeyFromAction(const FName &ActionName, const FKey &Key);

    /** Unmap all keys from an action */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Integration")
    bool UnmapAllKeysFromAction(const FName &ActionName);

    /** Clear all mappings */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Integration")
    void ClearAllMappings();

    // ==================== FKey from String ====================

    /** Convert a string to FKey (e.g., "SpaceBar", "Gamepad_FaceButton_Bottom", "W") */
    UFUNCTION(BlueprintPure, Category = "Input Binding|Utility")
    static FKey StringToKey(const FString &KeyString);

    /** Convert FKey to string */
    UFUNCTION(BlueprintPure, Category = "Input Binding|Utility")
    static FString KeyToString(const FKey &Key);

    /** Check if a key string is valid */
    UFUNCTION(BlueprintPure, Category = "Input Binding|Utility")
    static bool IsValidKeyString(const FString &KeyString);

    /** Get all available key names as strings */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Utility")
    static void GetAllKeyNames(TArray<FString> &OutKeyNames);

    // ==================== Mapping Context ====================

    /** Get the dynamically created mapping context */
    UFUNCTION(BlueprintPure, Category = "Input Binding|Integration")
    UInputMappingContext *GetMappingContext() const { return MappingContext; }

    /** Refresh the mapping context (reapply to player) */
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Integration")
    bool RefreshMappingContext();

    // ==================== Events (Legacy) ====================

    /** Broadcast when any dynamic input action is triggered (Legacy - use OnActionTriggered instead) */
    UPROPERTY(BlueprintAssignable, Category = "Input Binding|Events")
    FOnDynamicInputAction OnDynamicInputAction;

    // ==================== Global Event Dispatchers (Approach A) ====================
    // These events fire for ALL actions - use ActionName parameter to filter in Blueprint
    // Example: Bind to OnActionTriggered, then use "Switch on Name" to route to specific logic

    /** Fires when any action is TRIGGERED (main event - fires repeatedly while held for axis) */
    UPROPERTY(BlueprintAssignable, Category = "P_MEIS|Action Events")
    FOnInputActionEvent OnActionTriggered;

    /** Fires when any action is STARTED (initial press) */
    UPROPERTY(BlueprintAssignable, Category = "P_MEIS|Action Events")
    FOnInputActionEvent OnActionStarted;

    /** Fires when any action is ONGOING (held down, between Started and Triggered) */
    UPROPERTY(BlueprintAssignable, Category = "P_MEIS|Action Events")
    FOnInputActionEvent OnActionOngoing;

    /** Fires when any action is COMPLETED (released after successful trigger) */
    UPROPERTY(BlueprintAssignable, Category = "P_MEIS|Action Events")
    FOnInputActionEvent OnActionCompleted;

    /** Fires when any action is CANCELED (released before trigger threshold met) */
    UPROPERTY(BlueprintAssignable, Category = "P_MEIS|Action Events")
    FOnInputActionEvent OnActionCanceled;

    // ==================== UI / Virtual Device Injection ====================

    /** Inject an action as STARTED (press down) for this player (local-only). */
    UFUNCTION(BlueprintCallable, Category = "P_MEIS|Injection")
    void InjectActionStarted(const FName &ActionName);

    /** Inject an action as TRIGGERED (held / repeating) for this player (local-only). */
    UFUNCTION(BlueprintCallable, Category = "P_MEIS|Injection")
    void InjectActionTriggered(const FName &ActionName);

    /** Inject an action as COMPLETED (release) for this player (local-only). */
    UFUNCTION(BlueprintCallable, Category = "P_MEIS|Injection")
    void InjectActionCompleted(const FName &ActionName);

    /** Inject a 2D axis value (e.g., virtual joystick) for this player (local-only). */
    UFUNCTION(BlueprintCallable, Category = "P_MEIS|Injection")
    void InjectAxis2D(const FName &AxisName, const FVector2D &Value);

    // ==================== Async Action Listener Management (Approach C) ====================

    /** Register an async action listener for specific action events */
    void RegisterAsyncListener(UAsyncAction_WaitForInputAction *Listener);

    /** Unregister an async action listener */
    void UnregisterAsyncListener(UAsyncAction_WaitForInputAction *Listener);

    // ==================== Action Event Binding ====================

    /**
     * Bind all trigger events for a specific action to the internal handler
     * This is called automatically when creating actions, but can be called manually
     * to rebind after the player's EnhancedInputComponent changes
     */
    UFUNCTION(BlueprintCallable, Category = "P_MEIS|Action Events")
    bool BindActionEvents(const FName &ActionName);

    /**
     * Bind all trigger events for all created actions
     * Call this after SetupInputComponent or if bindings are lost
     */
    UFUNCTION(BlueprintCallable, Category = "P_MEIS|Action Events")
    void BindAllActionEvents();

    /**
     * Try to bind any pending actions that couldn't be bound earlier
     * Call this from Character BeginPlay or after Pawn possession
     * @return Number of actions successfully bound
     */
    UFUNCTION(BlueprintCallable, Category = "P_MEIS|Action Events")
    int32 TryBindPendingActions();

    /**
     * Check if there are pending actions waiting to be bound
     */
    UFUNCTION(BlueprintPure, Category = "P_MEIS|Action Events")
    bool HasPendingActions() const { return PendingBindActions.Num() > 0; }

    // ==================== Section 0.9: Dynamic Input Modifiers ====================

    /**
     * Create a new Input Action with modifiers and triggers pre-configured
     * @param ActionName Unique name for the action
     * @param ValueType The type of value this action produces (Bool, Axis1D, Axis2D, Axis3D)
     * @param Modifiers Array of modifier configurations to apply to the action
     * @param Triggers Array of trigger configurations (applied to default mappings)
     * @return The created Input Action, or nullptr on failure
     */
    UFUNCTION(BlueprintCallable, Category = "P_MEIS|Dynamic Actions")
    UInputAction *CreateDynamicInputActionWithModifiers(
        const FName &ActionName,
        EInputActionValueType ValueType,
        const TArray<FS_InputModifierConfig> &Modifiers,
        const TArray<FS_InputTriggerConfig> &Triggers);

    // ==================== Action-Level Modifiers ====================
    // These modifiers apply to ALL key mappings for the action

    /**
     * Add a modifier to an Input Action (applies to all key mappings)
     * @param ActionName Name of the action to modify
     * @param ModifierConfig Configuration for the modifier to add
     * @return True if successful
     */
    UFUNCTION(BlueprintCallable, Category = "P_MEIS|Modifiers")
    bool AddModifierToAction(const FName &ActionName, const FS_InputModifierConfig &ModifierConfig);

    /**
     * Remove a modifier type from an Input Action
     * @param ActionName Name of the action
     * @param ModifierType Type of modifier to remove
     * @return True if a modifier was removed
     */
    UFUNCTION(BlueprintCallable, Category = "P_MEIS|Modifiers")
    bool RemoveModifierFromAction(const FName &ActionName, EInputModifierType ModifierType);

    /**
     * Clear all modifiers from an Input Action
     * @param ActionName Name of the action
     * @return True if successful
     */
    UFUNCTION(BlueprintCallable, Category = "P_MEIS|Modifiers")
    bool ClearActionModifiers(const FName &ActionName);

    /**
     * Get all modifiers currently on an Input Action
     * @param ActionName Name of the action
     * @return Array of modifier configurations
     */
    UFUNCTION(BlueprintCallable, Category = "P_MEIS|Modifiers")
    TArray<FS_InputModifierConfig> GetActionModifiers(const FName &ActionName);

    // ==================== Key Mapping-Level Modifiers ====================
    // These modifiers apply only to a specific key mapping

    /**
     * Add a modifier to a specific key mapping (overrides action-level for this key)
     * @param ActionName Name of the action
     * @param Key The key to add the modifier to
     * @param ModifierConfig Configuration for the modifier to add
     * @return True if successful
     */
    UFUNCTION(BlueprintCallable, Category = "P_MEIS|Modifiers")
    bool AddModifierToKeyMapping(const FName &ActionName, const FKey &Key, const FS_InputModifierConfig &ModifierConfig);

    /**
     * Remove a modifier type from a specific key mapping
     * @param ActionName Name of the action
     * @param Key The key to modify
     * @param ModifierType Type of modifier to remove
     * @return True if a modifier was removed
     */
    UFUNCTION(BlueprintCallable, Category = "P_MEIS|Modifiers")
    bool RemoveModifierFromKeyMapping(const FName &ActionName, const FKey &Key, EInputModifierType ModifierType);

    /**
     * Clear all modifiers from a specific key mapping
     * @param ActionName Name of the action
     * @param Key The key to clear modifiers from
     * @return True if successful
     */
    UFUNCTION(BlueprintCallable, Category = "P_MEIS|Modifiers")
    bool ClearKeyMappingModifiers(const FName &ActionName, const FKey &Key);

    /**
     * Get all modifiers on a specific key mapping
     * @param ActionName Name of the action
     * @param Key The key to query
     * @return Array of modifier configurations
     */
    UFUNCTION(BlueprintCallable, Category = "P_MEIS|Modifiers")
    TArray<FS_InputModifierConfig> GetKeyMappingModifiers(const FName &ActionName, const FKey &Key);

    // ==================== Key Mapping-Level Triggers ====================
    // Triggers determine WHEN an action fires

    /**
     * Add a trigger to a specific key mapping
     * @param ActionName Name of the action
     * @param Key The key to add the trigger to
     * @param TriggerConfig Configuration for the trigger to add
     * @return True if successful
     */
    UFUNCTION(BlueprintCallable, Category = "P_MEIS|Triggers")
    bool AddTriggerToKeyMapping(const FName &ActionName, const FKey &Key, const FS_InputTriggerConfig &TriggerConfig);

    /**
     * Remove a trigger type from a specific key mapping
     * @param ActionName Name of the action
     * @param Key The key to modify
     * @param TriggerType Type of trigger to remove
     * @return True if a trigger was removed
     */
    UFUNCTION(BlueprintCallable, Category = "P_MEIS|Triggers")
    bool RemoveTriggerFromKeyMapping(const FName &ActionName, const FKey &Key, EInputTriggerType TriggerType);

    /**
     * Clear all triggers from a specific key mapping
     * @param ActionName Name of the action
     * @param Key The key to clear triggers from
     * @return True if successful
     */
    UFUNCTION(BlueprintCallable, Category = "P_MEIS|Triggers")
    bool ClearKeyMappingTriggers(const FName &ActionName, const FKey &Key);

    /**
     * Set a single trigger on a key mapping (clears existing triggers first)
     * @param ActionName Name of the action
     * @param Key The key to set the trigger on
     * @param TriggerConfig Configuration for the trigger
     * @return True if successful
     */
    UFUNCTION(BlueprintCallable, Category = "P_MEIS|Triggers")
    bool SetKeyMappingTrigger(const FName &ActionName, const FKey &Key, const FS_InputTriggerConfig &TriggerConfig);

    // ==================== Convenience Functions ====================
    // Quick methods for common modifier/trigger operations

    /**
     * Set dead zone for an action (all axes)
     * @param ActionName Name of the action
     * @param LowerThreshold Input below this is ignored (0.0-1.0)
     * @param UpperThreshold Input above this is clamped to 1.0
     * @return True if successful
     */
    UFUNCTION(BlueprintCallable, Category = "P_MEIS|Modifiers|Quick")
    bool SetActionDeadZone(const FName &ActionName, float LowerThreshold, float UpperThreshold = 1.0f);

    /**
     * Set sensitivity (scale) for an action (uniform across all axes)
     * @param ActionName Name of the action
     * @param Sensitivity Multiplier for input values
     * @return True if successful
     */
    UFUNCTION(BlueprintCallable, Category = "P_MEIS|Modifiers|Quick")
    bool SetActionSensitivity(const FName &ActionName, float Sensitivity);

    /**
     * Set sensitivity (scale) per axis for an action
     * @param ActionName Name of the action
     * @param Sensitivity Multiplier per axis (X, Y, Z)
     * @return True if successful
     */
    UFUNCTION(BlueprintCallable, Category = "P_MEIS|Modifiers|Quick")
    bool SetActionSensitivityPerAxis(const FName &ActionName, FVector Sensitivity);

    /**
     * Set invert Y axis for an action (common for "Invert Look" option)
     * @param ActionName Name of the action
     * @param bInvert Whether to invert the Y axis
     * @return True if successful
     */
    UFUNCTION(BlueprintCallable, Category = "P_MEIS|Modifiers|Quick")
    bool SetActionInvertY(const FName &ActionName, bool bInvert);

    /**
     * Set a hold trigger on a key mapping
     * @param ActionName Name of the action
     * @param Key The key to modify
     * @param HoldTime How long to hold before triggering (seconds)
     * @return True if successful
     */
    UFUNCTION(BlueprintCallable, Category = "P_MEIS|Triggers|Quick")
    bool SetKeyHoldTrigger(const FName &ActionName, const FKey &Key, float HoldTime);

    /**
     * Set a tap trigger on a key mapping
     * @param ActionName Name of the action
     * @param Key The key to modify
     * @param MaxTapTime Maximum time between press and release for tap (seconds)
     * @return True if successful
     */
    UFUNCTION(BlueprintCallable, Category = "P_MEIS|Triggers|Quick")
    bool SetKeyTapTrigger(const FName &ActionName, const FKey &Key, float MaxTapTime);

    // ==================== Modifier/Trigger Factory Functions ====================
    // These create UE5 objects from P_MEIS config structs

    /**
     * Create a UInputModifier from a configuration struct
     * @param ModifierConfig The configuration to use
     * @param Outer The outer object for the created modifier (usually the action or mapping context)
     * @return The created UInputModifier, or nullptr if invalid config
     */
    UFUNCTION(BlueprintCallable, Category = "P_MEIS|Factory")
    static UInputModifier *CreateUInputModifier(const FS_InputModifierConfig &ModifierConfig, UObject *Outer);

    /**
     * Create a UInputTrigger from a configuration struct
     * @param TriggerConfig The configuration to use
     * @param Outer The outer object for the created trigger
     * @param Integration Integration instance for resolving chord action references (optional)
     * @return The created UInputTrigger, or nullptr if invalid config
     */
    UFUNCTION(BlueprintCallable, Category = "P_MEIS|Factory")
    static UInputTrigger *CreateUInputTrigger(const FS_InputTriggerConfig &TriggerConfig, UObject *Outer, UCPP_EnhancedInputIntegration *Integration = nullptr);

private:
    UPROPERTY()
    APlayerController *PlayerController;

    UPROPERTY()
    UInputMappingContext *MappingContext;

    /** Map of action names to their dynamically created Input Actions */
    UPROPERTY()
    TMap<FName, UInputAction *> CreatedInputActions;

    /** Set of registered async action listeners (Approach C)
     *  Note: Not a UPROPERTY because UAsyncAction_WaitForInputAction is forward-declared only
     *  These are weak references - async actions manage their own lifetime */
    TArray<UAsyncAction_WaitForInputAction *> AsyncListeners;

    /** Track which actions have been bound to avoid duplicate bindings */
    TSet<FName> BoundActions;

    /** Actions that couldn't be bound because EnhancedInputComponent wasn't ready
     *  These will be bound when TryBindPendingActions() is called */
    TSet<FName> PendingBindActions;

    /** Create or get the mapping context */
    bool EnsureMappingContext();

    /** Apply the mapping context to the player's Enhanced Input Subsystem */
    bool ApplyMappingContextToPlayer();

    /** Internal helper to create modifiers for axis bindings */
    void ApplyAxisModifiers(UInputAction *Action, const FS_InputAxisBinding &AxisBinding);

    /** Find a key mapping in the mapping context for a given action and key */
    FEnhancedActionKeyMapping *FindKeyMapping(const FName &ActionName, const FKey &Key);

    /** Convert P_MEIS DeadZoneType to UE5 EDeadZoneType */
    static EDeadZoneType ConvertDeadZoneType(EP_MEIS_DeadZoneType Type);

    /** Convert P_MEIS SwizzleOrder to UE5 EInputAxisSwizzle */
    static EInputAxisSwizzle ConvertSwizzleOrder(EP_MEIS_SwizzleOrder Order);

    /** Convert P_MEIS SmoothingMethod to UE5 ENormalizeInputSmoothingType */
    static ENormalizeInputSmoothingType ConvertSmoothingMethod(EP_MEIS_SmoothingMethod Method);

    // ==================== Internal Event Handlers ====================
    // These are bound to Enhanced Input and route events to dispatchers

    /** Internal handler for all action events - routes to appropriate dispatcher */
    void HandleActionEvent(const FInputActionInstance &ActionInstance, FName ActionName);

    /** Individual trigger event handlers */
    void OnActionTriggeredInternal(const FInputActionInstance &ActionInstance, FName ActionName);
    void OnActionStartedInternal(const FInputActionInstance &ActionInstance, FName ActionName);
    void OnActionOngoingInternal(const FInputActionInstance &ActionInstance, FName ActionName);
    void OnActionCompletedInternal(const FInputActionInstance &ActionInstance, FName ActionName);
    void OnActionCanceledInternal(const FInputActionInstance &ActionInstance, FName ActionName);

    /** Notify all async listeners about an action event */
    void NotifyAsyncListeners(FName ActionName, ETriggerEvent TriggerEvent, const FInputActionValue &Value);
};
