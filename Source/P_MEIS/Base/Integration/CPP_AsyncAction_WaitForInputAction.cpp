/*
 * @Author: Punal Manalan
 * @Description: Async Blueprint Action Implementation for Input Action events
 * @Date: 07/12/2025
 */

#include "Integration/CPP_AsyncAction_WaitForInputAction.h"
#include "Integration/CPP_EnhancedInputIntegration.h"
#include "Manager/CPP_InputBindingManager.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"

UAsyncAction_WaitForInputAction *UAsyncAction_WaitForInputAction::WaitForInputAction(
    UObject *WorldContextObject,
    APlayerController *PlayerController,
    FName ActionName,
    bool bOnlyTriggerOnce)
{
    // Validate inputs
    if (!PlayerController)
    {
        UE_LOG(LogTemp, Warning, TEXT("P_MEIS Async: WaitForInputAction called with null PlayerController"));
        return nullptr;
    }

    if (ActionName.IsNone())
    {
        UE_LOG(LogTemp, Warning, TEXT("P_MEIS Async: WaitForInputAction called with empty ActionName"));
        return nullptr;
    }

    // Create the async action
    UAsyncAction_WaitForInputAction *Action = NewObject<UAsyncAction_WaitForInputAction>();
    Action->PlayerControllerPtr = PlayerController;
    Action->ActionName = ActionName;
    Action->bOnlyTriggerOnce = bOnlyTriggerOnce;
    Action->bIsActive = false;
    Action->bHasTriggered = false;

    // Register with game instance to prevent garbage collection
    Action->RegisterWithGameInstance(WorldContextObject);

    return Action;
}

void UAsyncAction_WaitForInputAction::Activate()
{
    if (bIsActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("P_MEIS Async: WaitForInputAction already active for action '%s'"), *ActionName.ToString());
        return;
    }

    RegisterWithIntegration();
}

void UAsyncAction_WaitForInputAction::Cancel()
{
    if (!bIsActive)
    {
        return;
    }

    UnregisterFromIntegration();

    // Broadcast the OnStopped delegate so Blueprint knows the action was stopped
    OnStopped.Broadcast();

    UE_LOG(LogTemp, Log, TEXT("P_MEIS Async: Canceled WaitForInputAction for '%s'"), *ActionName.ToString());
}

void UAsyncAction_WaitForInputAction::BeginDestroy()
{
    // Make sure we unregister when destroyed
    if (bIsActive)
    {
        UnregisterFromIntegration();
    }

    Super::BeginDestroy();
}

void UAsyncAction_WaitForInputAction::RegisterWithIntegration()
{
    APlayerController *PC = PlayerControllerPtr.Get();
    if (!PC)
    {
        UE_LOG(LogTemp, Warning, TEXT("P_MEIS Async: PlayerController is no longer valid"));
        return;
    }

    // Get the manager
    UCPP_InputBindingManager *Manager = GEngine->GetEngineSubsystem<UCPP_InputBindingManager>();
    if (!Manager)
    {
        UE_LOG(LogTemp, Error, TEXT("P_MEIS Async: Failed to get InputBindingManager"));
        return;
    }

    // Get integration for this player
    UCPP_EnhancedInputIntegration *Integration = Manager->GetIntegrationForPlayer(PC);
    if (!Integration)
    {
        UE_LOG(LogTemp, Warning, TEXT("P_MEIS Async: No integration found for player. Call InitializeEnhancedInputIntegration first."));
        return;
    }

    IntegrationPtr = Integration;

    // Register with the integration
    Integration->RegisterAsyncListener(this);
    bIsActive = true;

    UE_LOG(LogTemp, Log, TEXT("P_MEIS Async: Started listening for action '%s'"), *ActionName.ToString());
}

void UAsyncAction_WaitForInputAction::UnregisterFromIntegration()
{
    bIsActive = false;

    UCPP_EnhancedInputIntegration *Integration = IntegrationPtr.Get();
    if (Integration)
    {
        Integration->UnregisterAsyncListener(this);
    }

    IntegrationPtr.Reset();
}

void UAsyncAction_WaitForInputAction::HandleInputEvent(FName InActionName, ETriggerEvent TriggerEvent, const FInputActionValue &Value)
{
    // Filter by action name
    if (InActionName != ActionName)
    {
        return;
    }

    // If we've already triggered once and bOnlyTriggerOnce is true, ignore further events
    if (bOnlyTriggerOnce && bHasTriggered)
    {
        return;
    }

    // Broadcast to appropriate output pin based on trigger event type
    switch (TriggerEvent)
    {
    case ETriggerEvent::Triggered:
        OnTriggered.Broadcast(Value);
        bHasTriggered = true;

        // If only trigger once and this was the main Triggered event, cancel
        if (bOnlyTriggerOnce)
        {
            Cancel();
        }
        break;

    case ETriggerEvent::Started:
        OnStarted.Broadcast(Value);
        break;

    case ETriggerEvent::Ongoing:
        OnOngoing.Broadcast(Value);
        break;

    case ETriggerEvent::Completed:
        OnCompleted.Broadcast(Value);
        break;

    case ETriggerEvent::Canceled:
        OnCanceled.Broadcast(Value);
        break;

    default:
        break;
    }
}
