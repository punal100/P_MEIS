/*
 * @Author: Punal Manalan
 * @Description: Enhanced Input Integration - Bridges P_MEIS with UE5 Enhanced Input System
 *               Supports dynamic creation of Input Actions, Mapping Contexts, and key bindings at runtime
 *               Includes Blueprint-friendly event dispatchers for action binding
 * @Date: 06/12/2025
 * @Version: 2.0 - Added global event dispatchers (Approach A) and async node support (Approach C)
 */

#pragma once

#include "CoreMinimal.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "InputActionValue.h"
#include "InputBinding/FS_InputProfile.h"
#include "InputBinding/FS_InputActionBinding.h"
#include "CPP_EnhancedInputIntegration.generated.h"

class APlayerController;
class UEnhancedInputComponent;
class UInputMappingContext;
class UInputAction;
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
