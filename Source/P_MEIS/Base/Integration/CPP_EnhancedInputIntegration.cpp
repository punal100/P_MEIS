/*
 * @Author: Punal Manalan
 * @Description: Enhanced Input Integration Implementation - Full dynamic input system
 * @Date: 06/12/2025
 */

#include "Integration/CPP_EnhancedInputIntegration.h"
#include "Integration/CPP_AsyncAction_WaitForInputAction.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "InputModifiers.h"
#include "InputTriggers.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"

// ==================== Profile Application ====================

bool UCPP_EnhancedInputIntegration::ApplyProfile(const FS_InputProfile &Profile)
{
    if (!EnsureMappingContext())
    {
        UE_LOG(LogTemp, Warning, TEXT("P_MEIS: Failed to create mapping context"));
        return false;
    }

    // Clear existing mappings
    MappingContext->UnmapAll();
    CreatedInputActions.Empty();

    // Apply all action bindings
    for (const FS_InputActionBinding &ActionBinding : Profile.ActionBindings)
    {
        if (!ApplyActionBinding(ActionBinding))
        {
            UE_LOG(LogTemp, Warning, TEXT("P_MEIS: Failed to apply action binding: %s"), *ActionBinding.InputActionName.ToString());
        }
    }

    // Apply all axis bindings
    for (const FS_InputAxisBinding &AxisBinding : Profile.AxisBindings)
    {
        if (!ApplyAxisBinding(AxisBinding))
        {
            UE_LOG(LogTemp, Warning, TEXT("P_MEIS: Failed to apply axis binding: %s"), *AxisBinding.InputAxisName.ToString());
        }
    }

    // Apply to player
    return ApplyMappingContextToPlayer();
}

bool UCPP_EnhancedInputIntegration::ApplyActionBinding(const FS_InputActionBinding &ActionBinding)
{
    if (!ActionBinding.bEnabled)
    {
        return false;
    }

    if (!EnsureMappingContext())
    {
        return false;
    }

    // Create or get the Input Action
    UInputAction *Action = CreateInputAction(ActionBinding.InputActionName, EInputActionValueType::Boolean);
    if (!Action)
    {
        UE_LOG(LogTemp, Error, TEXT("P_MEIS: Failed to create Input Action: %s"), *ActionBinding.InputActionName.ToString());
        return false;
    }

    // Set action description
    Action->ActionDescription = ActionBinding.DisplayName;

    // Map all keys to this action
    for (const FS_KeyBinding &KeyBinding : ActionBinding.KeyBindings)
    {
        if (KeyBinding.Key.IsValid())
        {
            FEnhancedActionKeyMapping &Mapping = MappingContext->MapKey(Action, KeyBinding.Key);

            // TODO: Add modifier keys support (Shift, Ctrl, Alt, Cmd) via UInputTriggerChordAction
            UE_LOG(LogTemp, Log, TEXT("P_MEIS: Mapped key '%s' to action '%s'"),
                   *KeyBinding.Key.ToString(), *ActionBinding.InputActionName.ToString());
        }
    }

    return true;
}

bool UCPP_EnhancedInputIntegration::ApplyAxisBinding(const FS_InputAxisBinding &AxisBinding)
{
    if (!AxisBinding.bEnabled)
    {
        return false;
    }

    if (!EnsureMappingContext())
    {
        return false;
    }

    // Determine value type based on axis type
    // For 2D movement, use Axis2D; for single axis, use Axis1D
    EInputActionValueType ValueType = EInputActionValueType::Axis1D;

    // Create or get the Input Action
    UInputAction *Action = CreateInputAction(AxisBinding.InputAxisName, ValueType);
    if (!Action)
    {
        UE_LOG(LogTemp, Error, TEXT("P_MEIS: Failed to create Input Action for axis: %s"), *AxisBinding.InputAxisName.ToString());
        return false;
    }

    // Set action description
    Action->ActionDescription = AxisBinding.DisplayName;

    // Apply axis modifiers to the action
    ApplyAxisModifiers(Action, AxisBinding);

    // Map all keys/axes to this action
    for (const FS_AxisKeyBinding &KeyBinding : AxisBinding.AxisBindings)
    {
        if (KeyBinding.Key.IsValid())
        {
            FEnhancedActionKeyMapping &Mapping = MappingContext->MapKey(Action, KeyBinding.Key);

            // Apply scale modifier if not 1.0
            if (!FMath::IsNearlyEqual(KeyBinding.Scale, 1.0f))
            {
                UInputModifierScalar *ScaleModifier = NewObject<UInputModifierScalar>(Action);
                ScaleModifier->Scalar = FVector(KeyBinding.Scale, KeyBinding.Scale, KeyBinding.Scale);
                Mapping.Modifiers.Add(ScaleModifier);
            }

            // Apply negate if scale is negative or invert is set
            if (KeyBinding.Scale < 0.0f || AxisBinding.bInvert)
            {
                UInputModifierNegate *NegateModifier = NewObject<UInputModifierNegate>(Action);
                Mapping.Modifiers.Add(NegateModifier);
            }

            UE_LOG(LogTemp, Log, TEXT("P_MEIS: Mapped axis key '%s' (scale: %.2f) to action '%s'"),
                   *KeyBinding.Key.ToString(), KeyBinding.Scale, *AxisBinding.InputAxisName.ToString());
        }
    }

    return true;
}

// ==================== Player Controller ====================

void UCPP_EnhancedInputIntegration::SetPlayerController(APlayerController *InPlayerController)
{
    PlayerController = InPlayerController;

    // If we have a mapping context, apply it to the new player controller
    if (MappingContext && PlayerController)
    {
        ApplyMappingContextToPlayer();
    }
}

// ==================== Dynamic Input Action Creation ====================

UInputAction *UCPP_EnhancedInputIntegration::CreateInputAction(const FName &ActionName, EInputActionValueType ValueType)
{
    // Check if action already exists
    if (UInputAction **ExistingAction = CreatedInputActions.Find(ActionName))
    {
        return *ExistingAction;
    }

    // Create new Input Action
    UInputAction *NewAction = NewObject<UInputAction>(this, UInputAction::StaticClass(), ActionName);
    if (!NewAction)
    {
        UE_LOG(LogTemp, Error, TEXT("P_MEIS: Failed to create UInputAction object"));
        return nullptr;
    }

    // Configure the action
    NewAction->ValueType = ValueType;
    NewAction->bConsumeInput = true;
    NewAction->bTriggerWhenPaused = false;

    // Store in our map
    CreatedInputActions.Add(ActionName, NewAction);

    UE_LOG(LogTemp, Log, TEXT("P_MEIS: Created dynamic Input Action: %s (ValueType: %d)"),
           *ActionName.ToString(), static_cast<int32>(ValueType));

    return NewAction;
}

UInputAction *UCPP_EnhancedInputIntegration::GetInputAction(const FName &ActionName) const
{
    if (const UInputAction *const *Action = CreatedInputActions.Find(ActionName))
    {
        return const_cast<UInputAction *>(*Action);
    }
    return nullptr;
}

bool UCPP_EnhancedInputIntegration::HasInputAction(const FName &ActionName) const
{
    return CreatedInputActions.Contains(ActionName);
}

void UCPP_EnhancedInputIntegration::GetAllInputActions(TArray<UInputAction *> &OutActions) const
{
    CreatedInputActions.GenerateValueArray(OutActions);
}

// ==================== Key Mapping ====================

bool UCPP_EnhancedInputIntegration::MapKeyToAction(const FName &ActionName, const FKey &Key)
{
    if (!Key.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("P_MEIS: Invalid key provided for action: %s"), *ActionName.ToString());
        return false;
    }

    if (!EnsureMappingContext())
    {
        return false;
    }

    // Get or create the action
    UInputAction *Action = GetInputAction(ActionName);
    if (!Action)
    {
        Action = CreateInputAction(ActionName, EInputActionValueType::Boolean);
    }

    if (!Action)
    {
        return false;
    }

    // Map the key
    MappingContext->MapKey(Action, Key);

    // Refresh mapping context if we have a player
    if (PlayerController)
    {
        ApplyMappingContextToPlayer();

        // Bind action events so dispatchers fire (Approach A & C)
        BindActionEvents(ActionName);
    }

    UE_LOG(LogTemp, Log, TEXT("P_MEIS: Mapped key '%s' to action '%s'"), *Key.ToString(), *ActionName.ToString());
    return true;
}

bool UCPP_EnhancedInputIntegration::MapKeyBindingToAction(const FName &ActionName, const FS_KeyBinding &KeyBinding)
{
    if (!KeyBinding.Key.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("P_MEIS: Invalid key in KeyBinding for action: %s"), *ActionName.ToString());
        return false;
    }

    if (!EnsureMappingContext())
    {
        return false;
    }

    // Get or create the action
    UInputAction *Action = GetInputAction(ActionName);
    if (!Action)
    {
        Action = CreateInputAction(ActionName, EInputActionValueType::Boolean);
    }

    if (!Action)
    {
        return false;
    }

    // Map the key
    FEnhancedActionKeyMapping &Mapping = MappingContext->MapKey(Action, KeyBinding.Key);

    // Add modifier key triggers if any modifiers are specified
    // For modifier keys, we use chord triggers
    if (KeyBinding.bShift || KeyBinding.bCtrl || KeyBinding.bAlt || KeyBinding.bCmd)
    {
        // Add chord modifiers for each required modifier key
        if (KeyBinding.bShift)
        {
            // Add a chord trigger that requires Shift to be held
            UInputTriggerChordAction *ChordTrigger = NewObject<UInputTriggerChordAction>(this);
            // Create a shift action if needed
            UInputAction *ShiftAction = GetInputAction(FName("ModifierShift"));
            if (!ShiftAction)
            {
                ShiftAction = CreateInputAction(FName("ModifierShift"), EInputActionValueType::Boolean);
                // Map both shift keys to the modifier action
                MappingContext->MapKey(ShiftAction, EKeys::LeftShift);
                MappingContext->MapKey(ShiftAction, EKeys::RightShift);
            }
            ChordTrigger->ChordAction = ShiftAction;
            Mapping.Triggers.Add(ChordTrigger);
        }

        if (KeyBinding.bCtrl)
        {
            UInputTriggerChordAction *ChordTrigger = NewObject<UInputTriggerChordAction>(this);
            UInputAction *CtrlAction = GetInputAction(FName("ModifierCtrl"));
            if (!CtrlAction)
            {
                CtrlAction = CreateInputAction(FName("ModifierCtrl"), EInputActionValueType::Boolean);
                MappingContext->MapKey(CtrlAction, EKeys::LeftControl);
                MappingContext->MapKey(CtrlAction, EKeys::RightControl);
            }
            ChordTrigger->ChordAction = CtrlAction;
            Mapping.Triggers.Add(ChordTrigger);
        }

        if (KeyBinding.bAlt)
        {
            UInputTriggerChordAction *ChordTrigger = NewObject<UInputTriggerChordAction>(this);
            UInputAction *AltAction = GetInputAction(FName("ModifierAlt"));
            if (!AltAction)
            {
                AltAction = CreateInputAction(FName("ModifierAlt"), EInputActionValueType::Boolean);
                MappingContext->MapKey(AltAction, EKeys::LeftAlt);
                MappingContext->MapKey(AltAction, EKeys::RightAlt);
            }
            ChordTrigger->ChordAction = AltAction;
            Mapping.Triggers.Add(ChordTrigger);
        }

        if (KeyBinding.bCmd)
        {
            UInputTriggerChordAction *ChordTrigger = NewObject<UInputTriggerChordAction>(this);
            UInputAction *CmdAction = GetInputAction(FName("ModifierCmd"));
            if (!CmdAction)
            {
                CmdAction = CreateInputAction(FName("ModifierCmd"), EInputActionValueType::Boolean);
                MappingContext->MapKey(CmdAction, EKeys::LeftCommand);
                MappingContext->MapKey(CmdAction, EKeys::RightCommand);
            }
            ChordTrigger->ChordAction = CmdAction;
            Mapping.Triggers.Add(ChordTrigger);
        }

        UE_LOG(LogTemp, Log, TEXT("P_MEIS: Mapped key '%s' with modifiers (Shift:%d Ctrl:%d Alt:%d Cmd:%d) to action '%s'"),
               *KeyBinding.Key.ToString(), KeyBinding.bShift, KeyBinding.bCtrl, KeyBinding.bAlt, KeyBinding.bCmd, *ActionName.ToString());
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("P_MEIS: Mapped key '%s' to action '%s'"), *KeyBinding.Key.ToString(), *ActionName.ToString());
    }

    // Refresh mapping context
    if (PlayerController)
    {
        ApplyMappingContextToPlayer();

        // Bind action events so dispatchers fire (Approach A & C)
        BindActionEvents(ActionName);
    }

    return true;
}

bool UCPP_EnhancedInputIntegration::MapKeyStringWithModifiers(const FName &ActionName, const FString &KeyString,
                                                              bool bShift, bool bCtrl, bool bAlt, bool bCmd)
{
    FKey Key = StringToKey(KeyString);
    if (!Key.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("P_MEIS: Invalid key string '%s' for action: %s"), *KeyString, *ActionName.ToString());
        return false;
    }

    FS_KeyBinding KeyBinding;
    KeyBinding.Key = Key;
    KeyBinding.bShift = bShift;
    KeyBinding.bCtrl = bCtrl;
    KeyBinding.bAlt = bAlt;
    KeyBinding.bCmd = bCmd;

    return MapKeyBindingToAction(ActionName, KeyBinding);
}

bool UCPP_EnhancedInputIntegration::UnmapKeyFromAction(const FName &ActionName, const FKey &Key)
{
    if (!MappingContext)
    {
        return false;
    }

    UInputAction *Action = GetInputAction(ActionName);
    if (!Action)
    {
        return false;
    }

    MappingContext->UnmapKey(Action, Key);

    // Refresh mapping context
    if (PlayerController)
    {
        ApplyMappingContextToPlayer();
    }

    UE_LOG(LogTemp, Log, TEXT("P_MEIS: Unmapped key '%s' from action '%s'"), *Key.ToString(), *ActionName.ToString());
    return true;
}

bool UCPP_EnhancedInputIntegration::UnmapAllKeysFromAction(const FName &ActionName)
{
    if (!MappingContext)
    {
        return false;
    }

    UInputAction *Action = GetInputAction(ActionName);
    if (!Action)
    {
        return false;
    }

    MappingContext->UnmapAllKeysFromAction(Action);

    // Refresh mapping context
    if (PlayerController)
    {
        ApplyMappingContextToPlayer();
    }

    UE_LOG(LogTemp, Log, TEXT("P_MEIS: Unmapped all keys from action '%s'"), *ActionName.ToString());
    return true;
}

void UCPP_EnhancedInputIntegration::ClearAllMappings()
{
    if (MappingContext)
    {
        MappingContext->UnmapAll();
    }

    CreatedInputActions.Empty();

    if (PlayerController)
    {
        if (ULocalPlayer *LocalPlayer = PlayerController->GetLocalPlayer())
        {
            if (UEnhancedInputLocalPlayerSubsystem *Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
            {
                Subsystem->ClearAllMappings();
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("P_MEIS: Cleared all mappings"));
}

// ==================== FKey from String ====================

FKey UCPP_EnhancedInputIntegration::StringToKey(const FString &KeyString)
{
    // FKey constructor accepts FName/string directly
    FKey Key(*KeyString);

    if (!Key.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("P_MEIS: Invalid key string: '%s'"), *KeyString);
    }

    return Key;
}

FString UCPP_EnhancedInputIntegration::KeyToString(const FKey &Key)
{
    return Key.ToString();
}

bool UCPP_EnhancedInputIntegration::IsValidKeyString(const FString &KeyString)
{
    FKey Key(*KeyString);
    return Key.IsValid();
}

void UCPP_EnhancedInputIntegration::GetAllKeyNames(TArray<FString> &OutKeyNames)
{
    OutKeyNames.Empty();

    // Common keyboard keys
    OutKeyNames.Add(TEXT("A"));
    OutKeyNames.Add(TEXT("B"));
    OutKeyNames.Add(TEXT("C"));
    OutKeyNames.Add(TEXT("D"));
    OutKeyNames.Add(TEXT("E"));
    OutKeyNames.Add(TEXT("F"));
    OutKeyNames.Add(TEXT("G"));
    OutKeyNames.Add(TEXT("H"));
    OutKeyNames.Add(TEXT("I"));
    OutKeyNames.Add(TEXT("J"));
    OutKeyNames.Add(TEXT("K"));
    OutKeyNames.Add(TEXT("L"));
    OutKeyNames.Add(TEXT("M"));
    OutKeyNames.Add(TEXT("N"));
    OutKeyNames.Add(TEXT("O"));
    OutKeyNames.Add(TEXT("P"));
    OutKeyNames.Add(TEXT("Q"));
    OutKeyNames.Add(TEXT("R"));
    OutKeyNames.Add(TEXT("S"));
    OutKeyNames.Add(TEXT("T"));
    OutKeyNames.Add(TEXT("U"));
    OutKeyNames.Add(TEXT("V"));
    OutKeyNames.Add(TEXT("W"));
    OutKeyNames.Add(TEXT("X"));
    OutKeyNames.Add(TEXT("Y"));
    OutKeyNames.Add(TEXT("Z"));

    // Numbers
    OutKeyNames.Add(TEXT("Zero"));
    OutKeyNames.Add(TEXT("One"));
    OutKeyNames.Add(TEXT("Two"));
    OutKeyNames.Add(TEXT("Three"));
    OutKeyNames.Add(TEXT("Four"));
    OutKeyNames.Add(TEXT("Five"));
    OutKeyNames.Add(TEXT("Six"));
    OutKeyNames.Add(TEXT("Seven"));
    OutKeyNames.Add(TEXT("Eight"));
    OutKeyNames.Add(TEXT("Nine"));

    // Special keys
    OutKeyNames.Add(TEXT("SpaceBar"));
    OutKeyNames.Add(TEXT("Enter"));
    OutKeyNames.Add(TEXT("Escape"));
    OutKeyNames.Add(TEXT("Tab"));
    OutKeyNames.Add(TEXT("BackSpace"));
    OutKeyNames.Add(TEXT("LeftShift"));
    OutKeyNames.Add(TEXT("RightShift"));
    OutKeyNames.Add(TEXT("LeftControl"));
    OutKeyNames.Add(TEXT("RightControl"));
    OutKeyNames.Add(TEXT("LeftAlt"));
    OutKeyNames.Add(TEXT("RightAlt"));
    OutKeyNames.Add(TEXT("CapsLock"));

    // Arrow keys
    OutKeyNames.Add(TEXT("Up"));
    OutKeyNames.Add(TEXT("Down"));
    OutKeyNames.Add(TEXT("Left"));
    OutKeyNames.Add(TEXT("Right"));

    // Function keys
    OutKeyNames.Add(TEXT("F1"));
    OutKeyNames.Add(TEXT("F2"));
    OutKeyNames.Add(TEXT("F3"));
    OutKeyNames.Add(TEXT("F4"));
    OutKeyNames.Add(TEXT("F5"));
    OutKeyNames.Add(TEXT("F6"));
    OutKeyNames.Add(TEXT("F7"));
    OutKeyNames.Add(TEXT("F8"));
    OutKeyNames.Add(TEXT("F9"));
    OutKeyNames.Add(TEXT("F10"));
    OutKeyNames.Add(TEXT("F11"));
    OutKeyNames.Add(TEXT("F12"));

    // Mouse
    OutKeyNames.Add(TEXT("LeftMouseButton"));
    OutKeyNames.Add(TEXT("RightMouseButton"));
    OutKeyNames.Add(TEXT("MiddleMouseButton"));
    OutKeyNames.Add(TEXT("ThumbMouseButton"));
    OutKeyNames.Add(TEXT("ThumbMouseButton2"));
    OutKeyNames.Add(TEXT("MouseScrollUp"));
    OutKeyNames.Add(TEXT("MouseScrollDown"));
    OutKeyNames.Add(TEXT("MouseX"));
    OutKeyNames.Add(TEXT("MouseY"));

    // Gamepad buttons
    OutKeyNames.Add(TEXT("Gamepad_FaceButton_Bottom"));
    OutKeyNames.Add(TEXT("Gamepad_FaceButton_Right"));
    OutKeyNames.Add(TEXT("Gamepad_FaceButton_Left"));
    OutKeyNames.Add(TEXT("Gamepad_FaceButton_Top"));
    OutKeyNames.Add(TEXT("Gamepad_LeftShoulder"));
    OutKeyNames.Add(TEXT("Gamepad_RightShoulder"));
    OutKeyNames.Add(TEXT("Gamepad_LeftTrigger"));
    OutKeyNames.Add(TEXT("Gamepad_RightTrigger"));
    OutKeyNames.Add(TEXT("Gamepad_Special_Left"));
    OutKeyNames.Add(TEXT("Gamepad_Special_Right"));
    OutKeyNames.Add(TEXT("Gamepad_LeftThumbstick"));
    OutKeyNames.Add(TEXT("Gamepad_RightThumbstick"));
    OutKeyNames.Add(TEXT("Gamepad_DPad_Up"));
    OutKeyNames.Add(TEXT("Gamepad_DPad_Down"));
    OutKeyNames.Add(TEXT("Gamepad_DPad_Left"));
    OutKeyNames.Add(TEXT("Gamepad_DPad_Right"));

    // Gamepad axes
    OutKeyNames.Add(TEXT("Gamepad_LeftX"));
    OutKeyNames.Add(TEXT("Gamepad_LeftY"));
    OutKeyNames.Add(TEXT("Gamepad_RightX"));
    OutKeyNames.Add(TEXT("Gamepad_RightY"));
    OutKeyNames.Add(TEXT("Gamepad_Left2D"));
    OutKeyNames.Add(TEXT("Gamepad_Right2D"));
}

// ==================== Mapping Context ====================

bool UCPP_EnhancedInputIntegration::RefreshMappingContext()
{
    return ApplyMappingContextToPlayer();
}

// ==================== Private Helpers ====================

bool UCPP_EnhancedInputIntegration::EnsureMappingContext()
{
    if (!MappingContext)
    {
        MappingContext = NewObject<UInputMappingContext>(this, UInputMappingContext::StaticClass(), FName(TEXT("P_MEIS_DynamicMappingContext")));
        if (!MappingContext)
        {
            UE_LOG(LogTemp, Error, TEXT("P_MEIS: Failed to create UInputMappingContext"));
            return false;
        }

        UE_LOG(LogTemp, Log, TEXT("P_MEIS: Created dynamic mapping context"));
    }
    return true;
}

bool UCPP_EnhancedInputIntegration::ApplyMappingContextToPlayer()
{
    if (!PlayerController)
    {
        UE_LOG(LogTemp, Warning, TEXT("P_MEIS: No PlayerController set"));
        return false;
    }

    if (!MappingContext)
    {
        UE_LOG(LogTemp, Warning, TEXT("P_MEIS: No MappingContext created"));
        return false;
    }

    ULocalPlayer *LocalPlayer = PlayerController->GetLocalPlayer();
    if (!LocalPlayer)
    {
        UE_LOG(LogTemp, Warning, TEXT("P_MEIS: No LocalPlayer found"));
        return false;
    }

    UEnhancedInputLocalPlayerSubsystem *Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
    if (!Subsystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("P_MEIS: EnhancedInputLocalPlayerSubsystem not found"));
        return false;
    }

    // Remove existing P_MEIS context if present, then re-add
    Subsystem->RemoveMappingContext(MappingContext);
    Subsystem->AddMappingContext(MappingContext, 0);

    UE_LOG(LogTemp, Log, TEXT("P_MEIS: Applied mapping context to player"));
    return true;
}

void UCPP_EnhancedInputIntegration::ApplyAxisModifiers(UInputAction *Action, const FS_InputAxisBinding &AxisBinding)
{
    if (!Action)
    {
        return;
    }

    // Clear existing modifiers
    Action->Modifiers.Empty();

    // Add dead zone modifier
    if (AxisBinding.DeadZone > 0.0f)
    {
        UInputModifierDeadZone *DeadZoneModifier = NewObject<UInputModifierDeadZone>(Action);
        DeadZoneModifier->LowerThreshold = AxisBinding.DeadZone;
        DeadZoneModifier->UpperThreshold = 1.0f;
        DeadZoneModifier->Type = EDeadZoneType::Radial;
        Action->Modifiers.Add(DeadZoneModifier);
    }

    // Add sensitivity/scale modifier
    if (!FMath::IsNearlyEqual(AxisBinding.Sensitivity, 1.0f))
    {
        UInputModifierScalar *ScaleModifier = NewObject<UInputModifierScalar>(Action);
        ScaleModifier->Scalar = FVector(AxisBinding.Sensitivity, AxisBinding.Sensitivity, AxisBinding.Sensitivity);
        Action->Modifiers.Add(ScaleModifier);
    }

    // Add negate modifier if inverted
    if (AxisBinding.bInvert)
    {
        UInputModifierNegate *NegateModifier = NewObject<UInputModifierNegate>(Action);
        Action->Modifiers.Add(NegateModifier);
    }
}

// ==================== Action Event Binding (Approach A & C Support) ====================

bool UCPP_EnhancedInputIntegration::BindActionEvents(const FName &ActionName)
{
    if (!PlayerController)
    {
        UE_LOG(LogTemp, Warning, TEXT("P_MEIS: Cannot bind action events - no PlayerController set"));
        PendingBindActions.Add(ActionName);
        return false;
    }

    UInputAction *Action = GetInputAction(ActionName);
    if (!Action)
    {
        UE_LOG(LogTemp, Warning, TEXT("P_MEIS: Cannot bind action events - action '%s' not found"), *ActionName.ToString());
        return false;
    }

    // Check if already bound
    if (BoundActions.Contains(ActionName))
    {
        UE_LOG(LogTemp, Log, TEXT("P_MEIS: Action '%s' already bound"), *ActionName.ToString());
        PendingBindActions.Remove(ActionName); // Remove from pending if present
        return true;
    }

    APawn *Pawn = PlayerController->GetPawn();
    if (!Pawn)
    {
        UE_LOG(LogTemp, Warning, TEXT("P_MEIS: Cannot bind action events for '%s' - no Pawn found (will retry later)"), *ActionName.ToString());
        PendingBindActions.Add(ActionName);
        return false;
    }

    UEnhancedInputComponent *EnhancedInputComponent = Cast<UEnhancedInputComponent>(Pawn->InputComponent);
    if (!EnhancedInputComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("P_MEIS: Cannot bind action events for '%s' - no EnhancedInputComponent found (will retry later)"), *ActionName.ToString());
        PendingBindActions.Add(ActionName);
        return false;
    }

    // Bind all trigger events for this action
    // We pass the ActionName as a payload so we know which action triggered in the callback

    EnhancedInputComponent->BindAction(Action, ETriggerEvent::Triggered, this,
                                       &UCPP_EnhancedInputIntegration::OnActionTriggeredInternal, ActionName);

    EnhancedInputComponent->BindAction(Action, ETriggerEvent::Started, this,
                                       &UCPP_EnhancedInputIntegration::OnActionStartedInternal, ActionName);

    EnhancedInputComponent->BindAction(Action, ETriggerEvent::Ongoing, this,
                                       &UCPP_EnhancedInputIntegration::OnActionOngoingInternal, ActionName);

    EnhancedInputComponent->BindAction(Action, ETriggerEvent::Completed, this,
                                       &UCPP_EnhancedInputIntegration::OnActionCompletedInternal, ActionName);

    EnhancedInputComponent->BindAction(Action, ETriggerEvent::Canceled, this,
                                       &UCPP_EnhancedInputIntegration::OnActionCanceledInternal, ActionName);

    BoundActions.Add(ActionName);
    PendingBindActions.Remove(ActionName); // Remove from pending once bound successfully

    UE_LOG(LogTemp, Log, TEXT("P_MEIS: Bound all trigger events for action '%s'"), *ActionName.ToString());
    return true;
}

void UCPP_EnhancedInputIntegration::BindAllActionEvents()
{
    for (const auto &ActionPair : CreatedInputActions)
    {
        BindActionEvents(ActionPair.Key);
    }
}

int32 UCPP_EnhancedInputIntegration::TryBindPendingActions()
{
    if (PendingBindActions.Num() == 0)
    {
        UE_LOG(LogTemp, Log, TEXT("P_MEIS: No pending actions to bind"));
        return 0;
    }

    // Copy the set since BindActionEvents modifies PendingBindActions
    TSet<FName> ActionsToTry = PendingBindActions;
    int32 BoundCount = 0;

    UE_LOG(LogTemp, Log, TEXT("P_MEIS: Attempting to bind %d pending actions..."), ActionsToTry.Num());

    for (const FName &ActionName : ActionsToTry)
    {
        if (BindActionEvents(ActionName))
        {
            BoundCount++;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("P_MEIS: Successfully bound %d/%d pending actions. %d still pending."),
           BoundCount, ActionsToTry.Num(), PendingBindActions.Num());

    return BoundCount;
}

// ==================== Async Listener Management (Approach C) ====================

void UCPP_EnhancedInputIntegration::RegisterAsyncListener(UAsyncAction_WaitForInputAction *Listener)
{
    if (!Listener)
    {
        return;
    }

    // Add to listeners if not already present
    if (!AsyncListeners.Contains(Listener))
    {
        AsyncListeners.Add(Listener);
        UE_LOG(LogTemp, Verbose, TEXT("P_MEIS: Registered async listener for action '%s'"), *Listener->GetActionName().ToString());
    }
}

void UCPP_EnhancedInputIntegration::UnregisterAsyncListener(UAsyncAction_WaitForInputAction *Listener)
{
    if (!Listener)
    {
        return;
    }

    AsyncListeners.Remove(Listener);
    UE_LOG(LogTemp, Verbose, TEXT("P_MEIS: Unregistered async listener for action '%s'"), *Listener->GetActionName().ToString());
}

void UCPP_EnhancedInputIntegration::NotifyAsyncListeners(FName ActionName, ETriggerEvent TriggerEvent, const FInputActionValue &Value)
{
    // Create a copy of the array in case listeners unregister themselves during iteration
    TArray<UAsyncAction_WaitForInputAction *> ListenersCopy = AsyncListeners;

    for (UAsyncAction_WaitForInputAction *Listener : ListenersCopy)
    {
        if (Listener && Listener->IsActive())
        {
            Listener->HandleInputEvent(ActionName, TriggerEvent, Value);
        }
    }
}

// ==================== Internal Event Handlers ====================

void UCPP_EnhancedInputIntegration::HandleActionEvent(const FInputActionInstance &ActionInstance, FName ActionName)
{
    // This is a generic handler - not used directly, but available for future use
    FInputActionValue Value = ActionInstance.GetValue();

    // Broadcast legacy delegate
    OnDynamicInputAction.Broadcast(ActionName, Value);
}

void UCPP_EnhancedInputIntegration::OnActionTriggeredInternal(const FInputActionInstance &ActionInstance, FName ActionName)
{
    FInputActionValue Value = ActionInstance.GetValue();

    // Approach A: Broadcast to global dispatcher
    OnActionTriggered.Broadcast(ActionName, Value);

    // Legacy support
    OnDynamicInputAction.Broadcast(ActionName, Value);

    // Approach C: Notify async listeners
    NotifyAsyncListeners(ActionName, ETriggerEvent::Triggered, Value);

    UE_LOG(LogTemp, Verbose, TEXT("P_MEIS: Action '%s' TRIGGERED"), *ActionName.ToString());
}

void UCPP_EnhancedInputIntegration::OnActionStartedInternal(const FInputActionInstance &ActionInstance, FName ActionName)
{
    FInputActionValue Value = ActionInstance.GetValue();

    // Approach A: Broadcast to global dispatcher
    OnActionStarted.Broadcast(ActionName, Value);

    // Approach C: Notify async listeners
    NotifyAsyncListeners(ActionName, ETriggerEvent::Started, Value);

    UE_LOG(LogTemp, Verbose, TEXT("P_MEIS: Action '%s' STARTED"), *ActionName.ToString());
}

void UCPP_EnhancedInputIntegration::OnActionOngoingInternal(const FInputActionInstance &ActionInstance, FName ActionName)
{
    FInputActionValue Value = ActionInstance.GetValue();

    // Approach A: Broadcast to global dispatcher
    OnActionOngoing.Broadcast(ActionName, Value);

    // Approach C: Notify async listeners
    NotifyAsyncListeners(ActionName, ETriggerEvent::Ongoing, Value);

    // Note: Ongoing fires very frequently, so we use Verbose level
    UE_LOG(LogTemp, VeryVerbose, TEXT("P_MEIS: Action '%s' ONGOING"), *ActionName.ToString());
}

void UCPP_EnhancedInputIntegration::OnActionCompletedInternal(const FInputActionInstance &ActionInstance, FName ActionName)
{
    FInputActionValue Value = ActionInstance.GetValue();

    // Approach A: Broadcast to global dispatcher
    OnActionCompleted.Broadcast(ActionName, Value);

    // Approach C: Notify async listeners
    NotifyAsyncListeners(ActionName, ETriggerEvent::Completed, Value);

    UE_LOG(LogTemp, Verbose, TEXT("P_MEIS: Action '%s' COMPLETED"), *ActionName.ToString());
}

void UCPP_EnhancedInputIntegration::OnActionCanceledInternal(const FInputActionInstance &ActionInstance, FName ActionName)
{
    FInputActionValue Value = ActionInstance.GetValue();

    // Approach A: Broadcast to global dispatcher
    OnActionCanceled.Broadcast(ActionName, Value);

    // Approach C: Notify async listeners
    NotifyAsyncListeners(ActionName, ETriggerEvent::Canceled, Value);

    UE_LOG(LogTemp, Verbose, TEXT("P_MEIS: Action '%s' CANCELED"), *ActionName.ToString());
}
