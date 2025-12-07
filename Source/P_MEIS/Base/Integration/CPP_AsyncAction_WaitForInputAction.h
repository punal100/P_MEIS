/*
 * @Author: Punal Manalan
 * @Description: Async Blueprint Action for waiting on specific Input Action events
 *               Implements Approach C from Section 0.8 of PLAN.md
 *               Provides per-action event binding with multiple output execution pins
 * @Date: 07/12/2025
 * @Version: 1.0
 */

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "InputActionValue.h"
#include "InputTriggers.h"
#include "CPP_AsyncAction_WaitForInputAction.generated.h"

class APlayerController;
class UCPP_EnhancedInputIntegration;

// Delegate for async action output pins
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInputActionEventPin, FInputActionValue, Value);

// Delegate for when the async action is stopped
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAsyncActionStopped);

/**
 * Async Blueprint action that waits for a specific Input Action to fire
 *
 * This provides a cleaner Blueprint interface than global dispatchers when you need
 * to handle specific actions. Each node instance listens for one action.
 *
 * Blueprint Usage:
 *   Wait For Input Action (PlayerController, "IA_Jump")
 *       ├── On Triggered → [Jump Logic]
 *       ├── On Started → [Charge Jump]
 *       ├── On Completed → [Release Jump]
 *       └── On Canceled → [Cancel Jump]
 */
UCLASS(meta = (HideThen = true))
class P_MEIS_API UAsyncAction_WaitForInputAction : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    /**
     * Wait for a specific Input Action to fire events
     *
     * @param WorldContextObject World context for the async action
     * @param PlayerController The player controller to listen for input from
     * @param ActionName Name of the action to listen for (e.g., "IA_Jump")
     * @param bOnlyTriggerOnce If true, the node will only fire once then stop listening
     * @return The async action instance
     */
    UFUNCTION(BlueprintCallable, Category = "P_MEIS|Async",
              meta = (BlueprintInternalUseOnly = "true",
                      WorldContext = "WorldContextObject",
                      DisplayName = "Wait For Input Action"))
    static UAsyncAction_WaitForInputAction *WaitForInputAction(
        UObject *WorldContextObject,
        APlayerController *PlayerController,
        FName ActionName,
        bool bOnlyTriggerOnce = false);

    // ==================== Output Execution Pins ====================

    /** Fires when the action is TRIGGERED (main event - fires repeatedly while held for axis) */
    UPROPERTY(BlueprintAssignable)
    FOnInputActionEventPin OnTriggered;

    /** Fires when the action is STARTED (initial press) */
    UPROPERTY(BlueprintAssignable)
    FOnInputActionEventPin OnStarted;

    /** Fires when the action is ONGOING (held down) */
    UPROPERTY(BlueprintAssignable)
    FOnInputActionEventPin OnOngoing;

    /** Fires when the action is COMPLETED (released after successful trigger) */
    UPROPERTY(BlueprintAssignable)
    FOnInputActionEventPin OnCompleted;

    /** Fires when the action is CANCELED (released before trigger threshold) */
    UPROPERTY(BlueprintAssignable)
    FOnInputActionEventPin OnCanceled;

    /** Fires when the async action is stopped (via Cancel() or StopWaitingForInputAction) */
    UPROPERTY(BlueprintAssignable)
    FOnAsyncActionStopped OnStopped;

    // ==================== Lifecycle ====================

    /** Activate the async action - starts listening for input */
    virtual void Activate() override;

    /**
     * Cancel the async action - stops listening for input
     * Call this to manually stop the node from firing events
     */
    UFUNCTION(BlueprintCallable, Category = "P_MEIS|Async")
    void Cancel();

    /** Check if the async action is currently active */
    UFUNCTION(BlueprintPure, Category = "P_MEIS|Async")
    bool IsActive() const { return bIsActive; }

    /** Get the action name this node is listening for */
    UFUNCTION(BlueprintPure, Category = "P_MEIS|Async")
    FName GetActionName() const { return ActionName; }

    // ==================== Internal - Called by Integration ====================

    /** Called by UCPP_EnhancedInputIntegration when an action event fires */
    void HandleInputEvent(FName InActionName, ETriggerEvent TriggerEvent, const FInputActionValue &Value);

protected:
    /** Clean up when the action is destroyed */
    virtual void BeginDestroy() override;

    /** Register with the integration class to receive events */
    void RegisterWithIntegration();

    /** Unregister from the integration class */
    void UnregisterFromIntegration();

private:
    /** The player controller we're listening to */
    UPROPERTY()
    TWeakObjectPtr<APlayerController> PlayerControllerPtr;

    /** The integration class for this player */
    UPROPERTY()
    TWeakObjectPtr<UCPP_EnhancedInputIntegration> IntegrationPtr;

    /** The action name to listen for */
    FName ActionName;

    /** If true, only trigger once then deactivate */
    bool bOnlyTriggerOnce;

    /** Whether the async action is currently active */
    bool bIsActive;

    /** Whether we've triggered at least once (for bOnlyTriggerOnce) */
    bool bHasTriggered;
};
