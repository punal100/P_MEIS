/*
 * @Author: Punal Manalan
 * @Description: Enhanced Input Integration Implementation - Full dynamic input system
 * @Date: 06/12/2025
 */

#include "Integration/CPP_EnhancedInputIntegration.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "InputModifiers.h"
#include "InputTriggers.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Controller.h"
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
        UE_LOG(LogTemp, Log, TEXT("P_MEIS: ApplyProfile iterating - Axis: '%s', ValueType in Profile: %d"),
               *AxisBinding.InputAxisName.ToString(), static_cast<int32>(AxisBinding.ValueType));

        if (!ApplyAxisBinding(AxisBinding))
        {
            UE_LOG(LogTemp, Warning, TEXT("P_MEIS: Failed to apply axis binding: %s"), *AxisBinding.InputAxisName.ToString());
        }
    }

    // Apply mapping context to local player's Enhanced Input subsystem (players only)
    // For AI / non-local controllers there is no LocalPlayer subsystem, so we skip this.
    const bool bShouldApplyLocalPlayerContext = (PlayerController && PlayerController->IsLocalController());
    if (bShouldApplyLocalPlayerContext)
    {
        if (!ApplyMappingContextToPlayer())
        {
            UE_LOG(LogTemp, Warning, TEXT("P_MEIS: Failed to apply mapping context to player"));
            return false;
        }
    }

    // Bind action events AFTER mapping context is applied so callbacks work
    // This hooks up the OnActionTriggered, OnActionStarted, etc. delegates
    BindAllActionEvents();

    return true;
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

    // Use the value type specified in the binding (defaults to Axis1D for backward compatibility)
    EInputActionValueType ValueType = AxisBinding.ValueType;

    UE_LOG(LogTemp, Log, TEXT("P_MEIS: ApplyAxisBinding '%s' - AxisBinding.ValueType: %d"),
           *AxisBinding.InputAxisName.ToString(), static_cast<int32>(AxisBinding.ValueType));

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

            // Apply swizzle modifier first (YXZ swaps X and Y, so X input goes to Y output)
            if (KeyBinding.bSwizzleYXZ)
            {
                UInputModifierSwizzleAxis *SwizzleModifier = NewObject<UInputModifierSwizzleAxis>(Action);
                SwizzleModifier->Order = EInputAxisSwizzle::YXZ;
                Mapping.Modifiers.Add(SwizzleModifier);
            }

            // Apply negate if scale is negative
            if (KeyBinding.Scale < 0.0f)
            {
                UInputModifierNegate *NegateModifier = NewObject<UInputModifierNegate>(Action);
                Mapping.Modifiers.Add(NegateModifier);
            }

            // Apply scale modifier if not 1.0 (use absolute value since negate handles sign)
            float AbsScale = FMath::Abs(KeyBinding.Scale);
            if (!FMath::IsNearlyEqual(AbsScale, 1.0f))
            {
                UInputModifierScalar *ScaleModifier = NewObject<UInputModifierScalar>(Action);
                ScaleModifier->Scalar = FVector(AbsScale, AbsScale, AbsScale);
                Mapping.Modifiers.Add(ScaleModifier);
            }

            // Apply invert if set on the axis binding
            if (AxisBinding.bInvert)
            {
                UInputModifierNegate *NegateModifier = NewObject<UInputModifierNegate>(Action);
                Mapping.Modifiers.Add(NegateModifier);
            }

            UE_LOG(LogTemp, Log, TEXT("P_MEIS: Mapped axis key '%s' (scale: %.2f, swizzle: %d) to action '%s'"),
                   *KeyBinding.Key.ToString(), KeyBinding.Scale, KeyBinding.bSwizzleYXZ, *AxisBinding.InputAxisName.ToString());
        }
    }

    return true;
}

// ==================== Player Controller ====================

void UCPP_EnhancedInputIntegration::SetPlayerController(APlayerController *InPlayerController)
{
    PlayerController = InPlayerController;
    OwningController = InPlayerController;

    // If we have a mapping context, apply it to the new player controller
    if (MappingContext && PlayerController && PlayerController->IsLocalController())
    {
        ApplyMappingContextToPlayer();
    }
}

void UCPP_EnhancedInputIntegration::SetController(AController *InController)
{
    OwningController = InController;
    PlayerController = Cast<APlayerController>(InController);

    if (MappingContext && PlayerController && PlayerController->IsLocalController())
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

    // Refresh mapping context if we have a player
    if (PlayerController)
    {
        ApplyMappingContextToPlayer();
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

    if (!PlayerController->IsLocalController())
    {
        // Non-local controllers (including AI) do not have a LocalPlayer subsystem.
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
    if (!OwningController)
    {
        UE_LOG(LogTemp, Warning, TEXT("P_MEIS: Cannot bind action events - no Controller set"));
        return false;
    }

    UInputAction *Action = GetInputAction(ActionName);
    if (!Action)
    {
        UE_LOG(LogTemp, Warning, TEXT("P_MEIS: Cannot bind action events - action '%s' not found"), *ActionName.ToString());
        return false;
    }

    UEnhancedInputComponent *EnhancedInputComponent = Cast<UEnhancedInputComponent>(OwningController->InputComponent);
    if (!EnhancedInputComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("P_MEIS: Cannot bind action events for '%s' - Controller has no EnhancedInputComponent"), *ActionName.ToString());
        return false;
    }

    // If the input component changed (PIE travel / controller reinit), bindings must be rebuilt.
    if (BoundEnhancedInputComponent.Get() != EnhancedInputComponent)
    {
        BoundActions.Reset();
        PendingBindActions.Reset();
        BoundEnhancedInputComponent = EnhancedInputComponent;
    }

    // Check if already bound (for this specific component)
    if (BoundActions.Contains(ActionName))
    {
        UE_LOG(LogTemp, Log, TEXT("P_MEIS: Action '%s' already bound"), *ActionName.ToString());
        return true;
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

    UE_LOG(LogTemp, Log, TEXT("P_MEIS: Bound all trigger events for action '%s'"), *ActionName.ToString());
    return true;
}

void UCPP_EnhancedInputIntegration::BindAllActionEvents()
{
    int32 BoundCount = 0;
    int32 PendingCount = 0;

    for (const auto &ActionPair : CreatedInputActions)
    {
        if (BindActionEvents(ActionPair.Key))
        {
            BoundCount++;
        }
        else
        {
            // Add to pending for deferred binding (e.g., when Pawn not yet available)
            PendingBindActions.Add(ActionPair.Key);
            PendingCount++;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("P_MEIS: BindAllActionEvents - Bound: %d, Pending: %d"), BoundCount, PendingCount);
}

// ==================== Async Listener Management (Approach C) ====================
// TODO: Implement when UAsyncAction_WaitForInputAction class is created (Section 0.8.3 of PLAN.md)

void UCPP_EnhancedInputIntegration::RegisterAsyncListener(UAsyncAction_WaitForInputAction *Listener)
{
    // TODO: Implement async listener registration
    // Requires UAsyncAction_WaitForInputAction implementation
}

void UCPP_EnhancedInputIntegration::UnregisterAsyncListener(UAsyncAction_WaitForInputAction *Listener)
{
    // TODO: Implement async listener unregistration
    // Requires UAsyncAction_WaitForInputAction implementation
}

void UCPP_EnhancedInputIntegration::NotifyAsyncListeners(FName ActionName, ETriggerEvent TriggerEvent, const FInputActionValue &Value)
{
    // TODO: Implement async listener notification
    // Requires UAsyncAction_WaitForInputAction implementation
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

    // TODO: Approach C async listeners - uncomment when UAsyncAction_WaitForInputAction is implemented
    // NotifyAsyncListeners(ActionName, ETriggerEvent::Triggered, Value);

    UE_LOG(LogTemp, Verbose, TEXT("P_MEIS: Action '%s' TRIGGERED"), *ActionName.ToString());
}

void UCPP_EnhancedInputIntegration::OnActionStartedInternal(const FInputActionInstance &ActionInstance, FName ActionName)
{
    FInputActionValue Value = ActionInstance.GetValue();

    // Approach A: Broadcast to global dispatcher
    OnActionStarted.Broadcast(ActionName, Value);

    // TODO: Approach C async listeners - uncomment when UAsyncAction_WaitForInputAction is implemented
    // NotifyAsyncListeners(ActionName, ETriggerEvent::Started, Value);

    UE_LOG(LogTemp, Verbose, TEXT("P_MEIS: Action '%s' STARTED"), *ActionName.ToString());
}

void UCPP_EnhancedInputIntegration::OnActionOngoingInternal(const FInputActionInstance &ActionInstance, FName ActionName)
{
    FInputActionValue Value = ActionInstance.GetValue();

    // Approach A: Broadcast to global dispatcher
    OnActionOngoing.Broadcast(ActionName, Value);

    // TODO: Approach C async listeners - uncomment when UAsyncAction_WaitForInputAction is implemented
    // NotifyAsyncListeners(ActionName, ETriggerEvent::Ongoing, Value);

    // Note: Ongoing fires very frequently, so we use Verbose level
    UE_LOG(LogTemp, VeryVerbose, TEXT("P_MEIS: Action '%s' ONGOING"), *ActionName.ToString());
}

void UCPP_EnhancedInputIntegration::OnActionCompletedInternal(const FInputActionInstance &ActionInstance, FName ActionName)
{
    FInputActionValue Value = ActionInstance.GetValue();

    // Approach A: Broadcast to global dispatcher
    OnActionCompleted.Broadcast(ActionName, Value);

    // TODO: Approach C async listeners - uncomment when UAsyncAction_WaitForInputAction is implemented
    // NotifyAsyncListeners(ActionName, ETriggerEvent::Completed, Value);

    UE_LOG(LogTemp, Verbose, TEXT("P_MEIS: Action '%s' COMPLETED"), *ActionName.ToString());
}

void UCPP_EnhancedInputIntegration::OnActionCanceledInternal(const FInputActionInstance &ActionInstance, FName ActionName)
{
    FInputActionValue Value = ActionInstance.GetValue();

    // Approach A: Broadcast to global dispatcher
    OnActionCanceled.Broadcast(ActionName, Value);

    // TODO: Approach C async listeners - uncomment when UAsyncAction_WaitForInputAction is implemented
    // NotifyAsyncListeners(ActionName, ETriggerEvent::Canceled, Value);

    UE_LOG(LogTemp, Verbose, TEXT("P_MEIS: Action '%s' CANCELED"), *ActionName.ToString());
}

// ==================== UI / Virtual Device Injection ====================

void UCPP_EnhancedInputIntegration::InjectActionStarted(const FName &ActionName)
{
    if (!PlayerController || !PlayerController->IsLocalController())
    {
        return;
    }

    const FInputActionValue Value(true);
    OnActionStarted.Broadcast(ActionName, Value);
}

void UCPP_EnhancedInputIntegration::InjectActionTriggered(const FName &ActionName)
{
    if (!PlayerController || !PlayerController->IsLocalController())
    {
        return;
    }

    const FInputActionValue Value(true);
    OnActionTriggered.Broadcast(ActionName, Value);
    OnDynamicInputAction.Broadcast(ActionName, Value);
}

void UCPP_EnhancedInputIntegration::InjectActionCompleted(const FName &ActionName)
{
    if (!PlayerController || !PlayerController->IsLocalController())
    {
        return;
    }

    const FInputActionValue Value(false);
    OnActionCompleted.Broadcast(ActionName, Value);
}

void UCPP_EnhancedInputIntegration::InjectAxis2D(const FName &AxisName, const FVector2D &Value)
{
    if (!PlayerController || !PlayerController->IsLocalController())
    {
        return;
    }

    const FInputActionValue InputValue(Value);
    OnActionTriggered.Broadcast(AxisName, InputValue);
    OnDynamicInputAction.Broadcast(AxisName, InputValue);
}

// ==================== Section 0.9: Dynamic Input Modifiers & Triggers ====================
// TODO: Full implementation pending. These are stub implementations to satisfy linker.

int32 UCPP_EnhancedInputIntegration::TryBindPendingActions()
{
    int32 BoundCount = 0;
    TSet<FName> SuccessfullyBound;

    for (const FName &ActionName : PendingBindActions)
    {
        if (BindActionEvents(ActionName))
        {
            SuccessfullyBound.Add(ActionName);
            BoundCount++;
        }
    }

    // Remove successfully bound actions from pending
    for (const FName &ActionName : SuccessfullyBound)
    {
        PendingBindActions.Remove(ActionName);
    }

    if (BoundCount > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("P_MEIS: TryBindPendingActions - bound %d actions, %d still pending"), BoundCount, PendingBindActions.Num());
    }

    return BoundCount;
}

UInputAction *UCPP_EnhancedInputIntegration::CreateDynamicInputActionWithModifiers(
    const FName &ActionName,
    EInputActionValueType ValueType,
    const TArray<FS_InputModifierConfig> &Modifiers,
    const TArray<FS_InputTriggerConfig> &Triggers)
{
    // Create the base action
    UInputAction *Action = CreateInputAction(ActionName, ValueType);
    if (!Action)
    {
        return nullptr;
    }

    // Set the value type
    Action->ValueType = ValueType;

    // Add modifiers
    for (const FS_InputModifierConfig &ModConfig : Modifiers)
    {
        if (ModConfig.bEnabled)
        {
            UInputModifier *Modifier = CreateUInputModifier(ModConfig, Action);
            if (Modifier)
            {
                Action->Modifiers.Add(Modifier);
            }
        }
    }

    // Note: Triggers are typically added to key mappings, not directly to actions
    // Store them for when keys are mapped
    UE_LOG(LogTemp, Log, TEXT("P_MEIS: Created action '%s' with %d modifiers"), *ActionName.ToString(), Action->Modifiers.Num());

    return Action;
}

// ==================== Action-Level Modifier Functions ====================

bool UCPP_EnhancedInputIntegration::AddModifierToAction(const FName &ActionName, const FS_InputModifierConfig &ModifierConfig)
{
    UInputAction *Action = GetInputAction(ActionName);
    if (!Action)
    {
        UE_LOG(LogTemp, Warning, TEXT("P_MEIS: AddModifierToAction - action '%s' not found"), *ActionName.ToString());
        return false;
    }

    UInputModifier *Modifier = CreateUInputModifier(ModifierConfig, Action);
    if (!Modifier)
    {
        UE_LOG(LogTemp, Warning, TEXT("P_MEIS: AddModifierToAction - failed to create modifier"));
        return false;
    }

    Action->Modifiers.Add(Modifier);
    UE_LOG(LogTemp, Log, TEXT("P_MEIS: Added modifier to action '%s'"), *ActionName.ToString());
    return true;
}

bool UCPP_EnhancedInputIntegration::RemoveModifierFromAction(const FName &ActionName, EInputModifierType ModifierType)
{
    UInputAction *Action = GetInputAction(ActionName);
    if (!Action)
    {
        UE_LOG(LogTemp, Warning, TEXT("P_MEIS: RemoveModifierFromAction - action '%s' not found"), *ActionName.ToString());
        return false;
    }

    // TODO: Implement proper modifier type detection and removal
    // For now, this is a stub that logs the request
    UE_LOG(LogTemp, Warning, TEXT("P_MEIS: RemoveModifierFromAction - not fully implemented yet"));
    return false;
}

bool UCPP_EnhancedInputIntegration::ClearActionModifiers(const FName &ActionName)
{
    UInputAction *Action = GetInputAction(ActionName);
    if (!Action)
    {
        UE_LOG(LogTemp, Warning, TEXT("P_MEIS: ClearActionModifiers - action '%s' not found"), *ActionName.ToString());
        return false;
    }

    Action->Modifiers.Empty();
    UE_LOG(LogTemp, Log, TEXT("P_MEIS: Cleared all modifiers from action '%s'"), *ActionName.ToString());
    return true;
}

TArray<FS_InputModifierConfig> UCPP_EnhancedInputIntegration::GetActionModifiers(const FName &ActionName)
{
    TArray<FS_InputModifierConfig> Result;

    UInputAction *Action = GetInputAction(ActionName);
    if (!Action)
    {
        UE_LOG(LogTemp, Warning, TEXT("P_MEIS: GetActionModifiers - action '%s' not found"), *ActionName.ToString());
        return Result;
    }

    // TODO: Convert UInputModifier objects back to config structs
    // For now, return empty array
    UE_LOG(LogTemp, Warning, TEXT("P_MEIS: GetActionModifiers - reverse conversion not fully implemented yet"));
    return Result;
}

// ==================== Key Mapping-Level Modifier Functions ====================

bool UCPP_EnhancedInputIntegration::AddModifierToKeyMapping(const FName &ActionName, const FKey &Key, const FS_InputModifierConfig &ModifierConfig)
{
    if (!MappingContext)
    {
        UE_LOG(LogTemp, Warning, TEXT("P_MEIS: AddModifierToKeyMapping - no mapping context"));
        return false;
    }

    // TODO: Find the key mapping and add modifier
    UE_LOG(LogTemp, Warning, TEXT("P_MEIS: AddModifierToKeyMapping - not fully implemented yet"));
    return false;
}

bool UCPP_EnhancedInputIntegration::RemoveModifierFromKeyMapping(const FName &ActionName, const FKey &Key, EInputModifierType ModifierType)
{
    if (!MappingContext)
    {
        UE_LOG(LogTemp, Warning, TEXT("P_MEIS: RemoveModifierFromKeyMapping - no mapping context"));
        return false;
    }

    // TODO: Find the key mapping and remove modifier
    UE_LOG(LogTemp, Warning, TEXT("P_MEIS: RemoveModifierFromKeyMapping - not fully implemented yet"));
    return false;
}

bool UCPP_EnhancedInputIntegration::ClearKeyMappingModifiers(const FName &ActionName, const FKey &Key)
{
    if (!MappingContext)
    {
        UE_LOG(LogTemp, Warning, TEXT("P_MEIS: ClearKeyMappingModifiers - no mapping context"));
        return false;
    }

    // TODO: Find the key mapping and clear modifiers
    UE_LOG(LogTemp, Warning, TEXT("P_MEIS: ClearKeyMappingModifiers - not fully implemented yet"));
    return false;
}

TArray<FS_InputModifierConfig> UCPP_EnhancedInputIntegration::GetKeyMappingModifiers(const FName &ActionName, const FKey &Key)
{
    TArray<FS_InputModifierConfig> Result;

    if (!MappingContext)
    {
        UE_LOG(LogTemp, Warning, TEXT("P_MEIS: GetKeyMappingModifiers - no mapping context"));
        return Result;
    }

    // TODO: Find the key mapping and convert modifiers
    UE_LOG(LogTemp, Warning, TEXT("P_MEIS: GetKeyMappingModifiers - not fully implemented yet"));
    return Result;
}

// ==================== Key Mapping-Level Trigger Functions ====================

bool UCPP_EnhancedInputIntegration::AddTriggerToKeyMapping(const FName &ActionName, const FKey &Key, const FS_InputTriggerConfig &TriggerConfig)
{
    if (!MappingContext)
    {
        UE_LOG(LogTemp, Warning, TEXT("P_MEIS: AddTriggerToKeyMapping - no mapping context"));
        return false;
    }

    // TODO: Find the key mapping and add trigger
    UE_LOG(LogTemp, Warning, TEXT("P_MEIS: AddTriggerToKeyMapping - not fully implemented yet"));
    return false;
}

bool UCPP_EnhancedInputIntegration::RemoveTriggerFromKeyMapping(const FName &ActionName, const FKey &Key, EInputTriggerType TriggerType)
{
    if (!MappingContext)
    {
        UE_LOG(LogTemp, Warning, TEXT("P_MEIS: RemoveTriggerFromKeyMapping - no mapping context"));
        return false;
    }

    // TODO: Find the key mapping and remove trigger
    UE_LOG(LogTemp, Warning, TEXT("P_MEIS: RemoveTriggerFromKeyMapping - not fully implemented yet"));
    return false;
}

bool UCPP_EnhancedInputIntegration::ClearKeyMappingTriggers(const FName &ActionName, const FKey &Key)
{
    if (!MappingContext)
    {
        UE_LOG(LogTemp, Warning, TEXT("P_MEIS: ClearKeyMappingTriggers - no mapping context"));
        return false;
    }

    // TODO: Find the key mapping and clear triggers
    UE_LOG(LogTemp, Warning, TEXT("P_MEIS: ClearKeyMappingTriggers - not fully implemented yet"));
    return false;
}

bool UCPP_EnhancedInputIntegration::SetKeyMappingTrigger(const FName &ActionName, const FKey &Key, const FS_InputTriggerConfig &TriggerConfig)
{
    // Clear existing triggers and add new one
    ClearKeyMappingTriggers(ActionName, Key);
    return AddTriggerToKeyMapping(ActionName, Key, TriggerConfig);
}

// ==================== Convenience Functions ====================

bool UCPP_EnhancedInputIntegration::SetActionDeadZone(const FName &ActionName, float LowerThreshold, float UpperThreshold)
{
    FS_InputModifierConfig Config;
    Config.ModifierType = EInputModifierType::DeadZone;
    Config.DeadZoneLower = LowerThreshold;
    Config.DeadZoneUpper = UpperThreshold;
    Config.bEnabled = true;

    // Remove existing dead zone first
    RemoveModifierFromAction(ActionName, EInputModifierType::DeadZone);

    return AddModifierToAction(ActionName, Config);
}

bool UCPP_EnhancedInputIntegration::SetActionSensitivity(const FName &ActionName, float Sensitivity)
{
    return SetActionSensitivityPerAxis(ActionName, FVector(Sensitivity, Sensitivity, Sensitivity));
}

bool UCPP_EnhancedInputIntegration::SetActionSensitivityPerAxis(const FName &ActionName, FVector Sensitivity)
{
    FS_InputModifierConfig Config;
    Config.ModifierType = EInputModifierType::Scale;
    Config.ScaleVector = Sensitivity;
    Config.bEnabled = true;

    // Remove existing scale modifier first
    RemoveModifierFromAction(ActionName, EInputModifierType::Scale);

    return AddModifierToAction(ActionName, Config);
}

bool UCPP_EnhancedInputIntegration::SetActionInvertY(const FName &ActionName, bool bInvert)
{
    if (bInvert)
    {
        FS_InputModifierConfig Config;
        Config.ModifierType = EInputModifierType::Negate;
        Config.bNegateX = false;
        Config.bNegateY = true;
        Config.bNegateZ = false;
        Config.bEnabled = true;

        return AddModifierToAction(ActionName, Config);
    }
    else
    {
        return RemoveModifierFromAction(ActionName, EInputModifierType::Negate);
    }
}

bool UCPP_EnhancedInputIntegration::SetKeyHoldTrigger(const FName &ActionName, const FKey &Key, float HoldTime)
{
    FS_InputTriggerConfig Config;
    Config.TriggerType = EInputTriggerType::Hold;
    Config.HoldTimeThreshold = HoldTime;
    Config.bEnabled = true;

    return SetKeyMappingTrigger(ActionName, Key, Config);
}

bool UCPP_EnhancedInputIntegration::SetKeyTapTrigger(const FName &ActionName, const FKey &Key, float MaxTapTime)
{
    FS_InputTriggerConfig Config;
    Config.TriggerType = EInputTriggerType::Tap;
    Config.TapReleaseTimeThreshold = MaxTapTime;
    Config.bEnabled = true;

    return SetKeyMappingTrigger(ActionName, Key, Config);
}

// ==================== Factory Functions ====================

UInputModifier *UCPP_EnhancedInputIntegration::CreateUInputModifier(const FS_InputModifierConfig &ModifierConfig, UObject *Outer)
{
    if (!ModifierConfig.bEnabled)
    {
        return nullptr;
    }

    switch (ModifierConfig.ModifierType)
    {
    case EInputModifierType::DeadZone:
    {
        UInputModifierDeadZone *Modifier = NewObject<UInputModifierDeadZone>(Outer);
        Modifier->LowerThreshold = ModifierConfig.DeadZoneLower;
        Modifier->UpperThreshold = ModifierConfig.DeadZoneUpper;
        // Convert P_MEIS enum to UE5 enum
        Modifier->Type = static_cast<EDeadZoneType>(static_cast<uint8>(ModifierConfig.DeadZoneType));
        return Modifier;
    }

    case EInputModifierType::Scale:
    {
        UInputModifierScalar *Modifier = NewObject<UInputModifierScalar>(Outer);
        Modifier->Scalar = ModifierConfig.ScaleVector;
        return Modifier;
    }

    case EInputModifierType::Negate:
    {
        UInputModifierNegate *Modifier = NewObject<UInputModifierNegate>(Outer);
        Modifier->bX = ModifierConfig.bNegateX;
        Modifier->bY = ModifierConfig.bNegateY;
        Modifier->bZ = ModifierConfig.bNegateZ;
        return Modifier;
    }

    case EInputModifierType::Swizzle:
    {
        UInputModifierSwizzleAxis *Modifier = NewObject<UInputModifierSwizzleAxis>(Outer);
        // Convert P_MEIS enum to UE5 enum
        Modifier->Order = static_cast<EInputAxisSwizzle>(static_cast<uint8>(ModifierConfig.SwizzleOrder));
        return Modifier;
    }

    case EInputModifierType::ResponseCurveExponential:
    {
        UInputModifierResponseCurveExponential *Modifier = NewObject<UInputModifierResponseCurveExponential>(Outer);
        Modifier->CurveExponent = ModifierConfig.CurveExponent;
        return Modifier;
    }

    case EInputModifierType::Smooth:
    {
        UInputModifierSmooth *Modifier = NewObject<UInputModifierSmooth>(Outer);
        // TODO: Configure smoothing parameters from config
        return Modifier;
    }

    case EInputModifierType::FOVScaling:
    {
        UInputModifierFOVScaling *Modifier = NewObject<UInputModifierFOVScaling>(Outer);
        Modifier->FOVScale = ModifierConfig.FOVScale;
        return Modifier;
    }

    case EInputModifierType::ToWorldSpace:
    {
        UInputModifierToWorldSpace *Modifier = NewObject<UInputModifierToWorldSpace>(Outer);
        return Modifier;
    }

    default:
        UE_LOG(LogTemp, Warning, TEXT("P_MEIS: CreateUInputModifier - unsupported modifier type %d"), (int32)ModifierConfig.ModifierType);
        return nullptr;
    }
}

UInputTrigger *UCPP_EnhancedInputIntegration::CreateUInputTrigger(const FS_InputTriggerConfig &TriggerConfig, UObject *Outer, UCPP_EnhancedInputIntegration *Integration)
{
    if (!TriggerConfig.bEnabled)
    {
        return nullptr;
    }

    switch (TriggerConfig.TriggerType)
    {
    case EInputTriggerType::Down:
    {
        UInputTriggerDown *Trigger = NewObject<UInputTriggerDown>(Outer);
        Trigger->ActuationThreshold = TriggerConfig.ActuationThreshold;
        return Trigger;
    }

    case EInputTriggerType::Pressed:
    {
        UInputTriggerPressed *Trigger = NewObject<UInputTriggerPressed>(Outer);
        Trigger->ActuationThreshold = TriggerConfig.ActuationThreshold;
        return Trigger;
    }

    case EInputTriggerType::Released:
    {
        UInputTriggerReleased *Trigger = NewObject<UInputTriggerReleased>(Outer);
        Trigger->ActuationThreshold = TriggerConfig.ActuationThreshold;
        return Trigger;
    }

    case EInputTriggerType::Hold:
    {
        UInputTriggerHold *Trigger = NewObject<UInputTriggerHold>(Outer);
        Trigger->HoldTimeThreshold = TriggerConfig.HoldTimeThreshold;
        Trigger->bIsOneShot = TriggerConfig.bIsOneShot;
        Trigger->ActuationThreshold = TriggerConfig.ActuationThreshold;
        return Trigger;
    }

    case EInputTriggerType::HoldAndRelease:
    {
        UInputTriggerHoldAndRelease *Trigger = NewObject<UInputTriggerHoldAndRelease>(Outer);
        Trigger->HoldTimeThreshold = TriggerConfig.HoldTimeThreshold;
        Trigger->ActuationThreshold = TriggerConfig.ActuationThreshold;
        return Trigger;
    }

    case EInputTriggerType::Tap:
    {
        UInputTriggerTap *Trigger = NewObject<UInputTriggerTap>(Outer);
        Trigger->TapReleaseTimeThreshold = TriggerConfig.TapReleaseTimeThreshold;
        Trigger->ActuationThreshold = TriggerConfig.ActuationThreshold;
        return Trigger;
    }

    case EInputTriggerType::Pulse:
    {
        UInputTriggerPulse *Trigger = NewObject<UInputTriggerPulse>(Outer);
        Trigger->bTriggerOnStart = TriggerConfig.bTriggerOnStart;
        Trigger->Interval = TriggerConfig.PulseInterval;
        Trigger->TriggerLimit = TriggerConfig.PulseTriggerLimit;
        Trigger->ActuationThreshold = TriggerConfig.ActuationThreshold;
        return Trigger;
    }

    case EInputTriggerType::ChordAction:
    {
        UInputTriggerChordAction *Trigger = NewObject<UInputTriggerChordAction>(Outer);
        // Resolve chord action if integration is provided
        if (Integration && !TriggerConfig.ChordActionName.IsNone())
        {
            Trigger->ChordAction = Integration->GetInputAction(TriggerConfig.ChordActionName);
        }
        Trigger->ActuationThreshold = TriggerConfig.ActuationThreshold;
        return Trigger;
    }

    default:
        UE_LOG(LogTemp, Warning, TEXT("P_MEIS: CreateUInputTrigger - unsupported trigger type %d"), (int32)TriggerConfig.TriggerType);
        return nullptr;
    }
}
