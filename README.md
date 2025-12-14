# P_MEIS - Modular Enhanced Input System

A powerful, fully dynamic input binding system for Unreal Engine 5 that creates Input Actions, Mapping Contexts, and key bindings **entirely at runtime** - no editor assets required!

For a quick project-oriented checklist, see [GUIDE.md](./GUIDE.md).

## 🌟 Key Features

- **100% Dynamic** - Create Input Actions and Mapping Contexts at runtime via C++ or Blueprint
- **Per-Player Architecture** - Full split-screen/multiplayer support with independent bindings per player
- **Blueprint Action Binding** - Global Event Dispatchers + Async Action Nodes for per-action events
- **Modifier Key Support** - Map keys with Shift, Ctrl, Alt, Cmd combinations
- **String to FKey** - Convert key names from strings (perfect for JSON configs, UI, etc.)
- **Profile Template System** - Save/Load reusable profile templates with JSON persistence
- **Conflict Detection** - Automatic detection of duplicate key bindings per player
- **Modular-Style** - Modular input settings inspired by Modular's robust system
- **Blueprint Accessible** - Full Blueprint Function Library for all operations
- **Hot-Reload** - Change bindings without game restart

## 👤 Author

**Punal Manalan**

## 📋 Requirements

- Unreal Engine 5.5+
- Enhanced Input Plugin (enabled by default in UE5)

## 📦 Installation

1. Clone or copy this plugin to your project's `Plugins/` folder
2. Regenerate project files
3. Enable the plugin in your project settings
4. Add `"P_MEIS"` to your module's `.Build.cs` dependencies

```csharp
PublicDependencyModuleNames.AddRange(new string[] { "P_MEIS" });
```

---

## 🏗️ Architecture Overview

P_MEIS uses a **per-PlayerController architecture** - each player has their OWN profile and integration, enabling true split-screen/multiplayer support with independent key bindings.

### Project boundary (keep P_MEIS generic)

- P_MEIS should remain **action-name-agnostic**. Do not hardcode gameplay semantics (e.g. specific action names like "Sprint") inside the plugin.
- Game/UI code defines the action names it needs and uses P_MEIS purely as a binding + injection layer.

### Profile persistence (this project)

- Profiles persist as JSON under `Saved/InputProfiles/`.
- Profile data can include:
  - `ToggleModeActions`: which action names behave as toggle instead of hold
  - `bActiveActionToggles`: runtime toggle state persisted per profile

### UI input injection (UMG/mobile)

If your project has UI controls (virtual joystick/buttons), inject into the local player’s integration so gameplay listens to one unified pipeline:

- `InjectAxis2D(PC, ActionName, FVector2D)`
- `InjectActionStarted/Triggered/Completed(PC, ActionName)`

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                    UCPP_InputBindingManager                                 │
│                    (Engine Subsystem - Singleton)                           │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│   ProfileTemplates: TMap<FName, FS_InputProfile>  ← Global library (disk)   │
│   ├── "Default"     → Factory settings template                             │
│   ├── "Competitive" → Competitive preset                                    │
│   └── "Custom_*"    → User-created templates                                │
│                                                                             │
│   PlayerDataMap: TMap<APlayerController*, FS_PlayerInputData>               │
│   │                                                                         │
│   ├── Player 0 (PC0) ─────────────────────────────────────────────────┐     │
│   │   FS_PlayerInputData:                                             │     │
│   │   ├── ActiveProfile: FS_InputProfile (THIS player's bindings)     │     │
│   │   ├── Integration: UCPP_EnhancedInputIntegration*                 │     │
│   │   │     ├── MappingContext: UInputMappingContext* (unique)        │     │
│   │   │     └── InputActions: TMap<FName, UInputAction*>              │     │
│   │   └── LoadedTemplateName: FName ("Default")                       │     │
│   │                                                                   │     │
│   └── Player 1 (PC1) ─────────────────────────────────────────────────┤     │
│       FS_PlayerInputData:                                             │     │
│       ├── ActiveProfile: FS_InputProfile (DIFFERENT bindings!)        │     │
│       ├── Integration: UCPP_EnhancedInputIntegration* (SEPARATE!)     │     │
│       └── LoadedTemplateName: FName                                   │     │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

### Key Concepts

| Concept                | Description                                                                                                            |
| ---------------------- | ---------------------------------------------------------------------------------------------------------------------- |
| **ProfileTemplates**   | Global library of saved profiles stored on disk. Shared presets that can be COPIED to players.                         |
| **PlayerDataMap**      | Per-player runtime data. Each player has their OWN `ActiveProfile` and `Integration`.                                  |
| **ActiveProfile**      | A player's current key bindings (FS_InputProfile). Modifications only affect THIS player.                              |
| **Integration**        | Per-player Enhanced Input bridge (UCPP_EnhancedInputIntegration). Creates runtime UInputAction & UInputMappingContext. |
| **LoadedTemplateName** | Tracks which template was loaded (for reload/save features).                                                           |

### Load vs Save Profile

```
LOAD Profile For Player (PC, "Default"):
  1. Find template "Default" in ProfileTemplates (disk)
  2. COPY the entire template INTO PC's ActiveProfile
  3. Set PC's LoadedTemplateName = "Default"
  4. Apply bindings to PC's Enhanced Input
  → PC now has their OWN COPY - changes do NOT affect "Default" template

SAVE Profile For Player (PC, "MyBindings"):
  1. Take PC's current ActiveProfile (runtime data)
  2. COPY it as a NEW template named "MyBindings"
  3. Save to disk (ProfileTemplates + file)
  → Other players can now LOAD "MyBindings" as their starting point
```

---

## 🚀 Quick Start Guide

### From Blueprint (Recommended)

```
Event BeginPlay
    │
    ├─→ Get Player Controller (store as MyPC)
    │
    ├─→ Initialize Enhanced Input Integration (MyPC)
    │       └─→ Returns Integration (can store or get later)
    │
    ├─→ Load Profile For Player (MyPC, "Default")
    │       └─→ Copies "Default" template to MyPC's profile
    │       └─→ Automatically applies to Enhanced Input
    │
    │   OR if first time / creating new profile:
    │   ├─→ Create Dynamic Input Action (MyPC, "IA_Jump", false)
    │   ├─→ Map Key String To Dynamic Action (MyPC, "IA_Jump", "SpaceBar")
    │   └─→ Save Profile For Player (MyPC, "Default")
    │
    └─→ Get Dynamic Input Action (MyPC, "IA_Jump")
            └─→ Bind to Enhanced Input Action event
```

### From C++

```cpp
#include "Manager/CPP_BPL_InputBinding.h"

void AMyCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Get player controller
    APlayerController* PC = Cast<APlayerController>(GetController());

    // Step 1: Initialize P_MEIS for this player
    UCPP_EnhancedInputIntegration* Integration =
        UCPP_BPL_InputBinding::InitializeEnhancedInputIntegration(this, PC);

    // Step 2: Try loading existing profile, or create default
    if (!UCPP_BPL_InputBinding::LoadProfileForPlayer(PC, "Default"))
    {
        // First time - create bindings
        SetupDefaultBindings(PC, Integration);
        UCPP_BPL_InputBinding::SaveProfileForPlayer(PC, "Default");
    }

    // Step 3: Get the Input Action and bind it
    if (UInputAction* JumpAction = Integration->GetInputAction("IA_Jump"))
    {
        // Bind to Enhanced Input Component
        if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
        {
            EIC->BindAction(JumpAction, ETriggerEvent::Triggered, this, &AMyCharacter::OnJump);
        }
    }
}

void AMyCharacter::SetupDefaultBindings(APlayerController* PC, UCPP_EnhancedInputIntegration* Integration)
{
    // Create Jump action (Boolean for button press)
    Integration->CreateInputAction("IA_Jump", EInputActionValueType::Boolean);
    UCPP_BPL_InputBinding::MapKeyStringToDynamicAction(PC, "IA_Jump", "SpaceBar");
    UCPP_BPL_InputBinding::MapKeyStringToDynamicAction(PC, "IA_Jump", "Gamepad_FaceButton_Bottom");

    // Create Move action (Axis2D for analog movement)
    Integration->CreateInputAction("IA_Move", EInputActionValueType::Axis2D);
    UCPP_BPL_InputBinding::MapKeyStringToDynamicAction(PC, "IA_Move", "W");
    UCPP_BPL_InputBinding::MapKeyStringToDynamicAction(PC, "IA_Move", "Gamepad_LeftStick_2D");
}
```

---

## 🎮 Split-Screen / Multiplayer Example

Each player gets completely independent bindings:

```cpp
// Player 1 joins
UCPP_BPL_InputBinding::InitializeEnhancedInputIntegration(this, PC1);
UCPP_BPL_InputBinding::LoadProfileForPlayer(PC1, "Default");
UCPP_BPL_InputBinding::MapKeyStringToDynamicAction(PC1, "IA_Jump", "SpaceBar");  // Keyboard

// Player 2 joins - COMPLETELY SEPARATE bindings!
UCPP_BPL_InputBinding::InitializeEnhancedInputIntegration(this, PC2);
UCPP_BPL_InputBinding::LoadProfileForPlayer(PC2, "Default");  // Gets OWN copy
UCPP_BPL_InputBinding::MapKeyStringToDynamicAction(PC2, "IA_Jump", "Gamepad_FaceButton_Bottom");  // Gamepad

// Player 1 changes their jump key - Player 2 is NOT affected!
UCPP_BPL_InputBinding::MapKeyStringToDynamicAction(PC1, "IA_Jump", "E");
// Player 1: Jump = E
// Player 2: Jump = Gamepad A button (unchanged)
```

---

## 🔑 String to FKey Examples

| String                             | FKey Result                     |
| ---------------------------------- | ------------------------------- |
| `SpaceBar`                         | Space key                       |
| `W`, `A`, `S`, `D`                 | WASD keys                       |
| `LeftMouseButton`                  | Left mouse button               |
| `RightMouseButton`                 | Right mouse button              |
| `MouseScrollUp`                    | Mouse wheel up                  |
| `Gamepad_FaceButton_Bottom`        | A button (Xbox) / X button (PS) |
| `Gamepad_FaceButton_Right`         | B button (Xbox) / Circle (PS)   |
| `Gamepad_LeftStick_2D`             | Left stick (2D axis)            |
| `Gamepad_RightTriggerAxis`         | Right trigger (analog)          |
| `LeftShift`, `LeftCtrl`, `LeftAlt` | Modifier keys                   |
| `F1` through `F12`                 | Function keys                   |

---

## 🎯 Mapping Keys with Modifiers

You can map keys with modifier combinations (Shift, Ctrl, Alt, Cmd):

```cpp
// Method 1: Using MapKeyStringWithModifiers (Blueprint Friendly)
UCPP_BPL_InputBinding::MapKeyStringWithModifiers(PC, "IA_Save", "S", false, true, false, false);
// Maps Ctrl+S to Save action

// Method 2: Using FS_KeyBinding struct
FS_KeyBinding KeyBinding = UCPP_BPL_InputBinding::MakeKeyBinding("S", false, true, false, false);
UCPP_BPL_InputBinding::MapKeyBindingToDynamicAction(PC, "IA_Save", KeyBinding);

// Method 3: Map multiple keys to same action
UCPP_BPL_InputBinding::MapKeyStringToDynamicAction(PC, "IA_Jump", "SpaceBar");           // Space
UCPP_BPL_InputBinding::MapKeyStringToDynamicAction(PC, "IA_Jump", "Gamepad_FaceButton_Bottom"); // Gamepad A
UCPP_BPL_InputBinding::MapKeyStringWithModifiers(PC, "IA_Jump", "W", true, false, false, false); // Shift+W
```

---

## 🎮 Binding Events to Input Actions

After creating an Input Action, you need to bind an event callback. There are two approaches:

### Method 1: Blueprint's Built-in Enhanced Action Events (Recommended)

In Blueprint, after calling `Get Dynamic Input Action`:

1. Right-click in Event Graph
2. Search for **"Enhanced Action Events"**
3. Select your action from the dropdown
4. Connect the event to your logic

```
Get Dynamic Input Action (MyPC, "IA_Jump")
    │
    └─→ Returns UInputAction*
            │
            ▼
    Enhanced Action Event (select your action)
        ├── Triggered → Your Jump Logic
        ├── Started → (optional)
        └── Completed → (optional)
```

### Method 2: C++ Manual Binding

```cpp
// In your Character's SetupPlayerInputComponent or BeginPlay:
void AMyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    // Get the Enhanced Input Component
    UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent);
    if (!EIC) return;

    // Get your dynamic action
    APlayerController* PC = Cast<APlayerController>(GetController());
    UInputAction* JumpAction = UCPP_BPL_InputBinding::GetDynamicInputAction(PC, "IA_Jump");

    if (JumpAction)
    {
        // Bind the action to your function
        EIC->BindAction(JumpAction, ETriggerEvent::Triggered, this, &AMyCharacter::OnJump);
        EIC->BindAction(JumpAction, ETriggerEvent::Started, this, &AMyCharacter::OnJumpStarted);
        EIC->BindAction(JumpAction, ETriggerEvent::Completed, this, &AMyCharacter::OnJumpReleased);
    }
}

void AMyCharacter::OnJump(const FInputActionValue& Value)
{
    // Value.Get<bool>() for button press
    Jump();
}
```

### Method 3: Global Event Dispatchers (Approach A)

P_MEIS provides global event dispatchers on the Integration class that fire for ALL actions. Use `ActionName` to filter:

```cpp
// Get Integration and bind to global dispatchers
UCPP_EnhancedInputIntegration* Integration = UCPP_BPL_InputBinding::GetIntegrationForPlayer(PC);

// Bind to specific trigger events
Integration->OnActionTriggered.AddDynamic(this, &AMyCharacter::HandleAnyActionTriggered);
Integration->OnActionStarted.AddDynamic(this, &AMyCharacter::HandleAnyActionStarted);
Integration->OnActionCompleted.AddDynamic(this, &AMyCharacter::HandleAnyActionCompleted);

void AMyCharacter::HandleAnyActionTriggered(FName ActionName, FInputActionValue Value)
{
    if (ActionName == "IA_Jump")
    {
        Jump();
    }
    else if (ActionName == "IA_Fire")
    {
        Fire();
    }
}
```

**Available Global Dispatchers:**

| Dispatcher          | Description                                       |
| ------------------- | ------------------------------------------------- |
| `OnActionTriggered` | Main event - fires repeatedly while held for axis |
| `OnActionStarted`   | Initial press                                     |
| `OnActionOngoing`   | Held down (between Started and Triggered)         |
| `OnActionCompleted` | Released after successful trigger                 |
| `OnActionCanceled`  | Released before trigger threshold met             |

### Method 4: Async Action Nodes (Approach C) - Blueprint Recommended

Use the **"Wait For Input Action"** async node for per-action binding with multiple output pins:

```
Event BeginPlay
    │
    ├─→ Initialize Enhanced Input Integration (MyPC)
    │
    ├─→ Create Dynamic Input Action (MyPC, "IA_Jump", false)
    │
    ├─→ Map Key String To Dynamic Action (MyPC, "IA_Jump", "SpaceBar")
    │
    └─→ Wait For Input Action (MyPC, "IA_Jump", false)
            ├── On Triggered ──→ Jump()
            ├── On Started ────→ StartJumpCharge()
            ├── On Ongoing ────→ UpdateJumpCharge()
            ├── On Completed ──→ ReleaseJump()
            ├── On Canceled ───→ CancelJump()
            └── On Stopped ────→ CleanupJumpUI()  // When listener is stopped
```

**Output Pins:**

| Pin          | Description                                                    |
| ------------ | -------------------------------------------------------------- |
| On Triggered | Fires when action is TRIGGERED (main event)                    |
| On Started   | Fires when action is STARTED (initial press)                   |
| On Ongoing   | Fires when action is ONGOING (held down)                       |
| On Completed | Fires when action is COMPLETED (released after trigger)        |
| On Canceled  | Fires when action is CANCELED (released before trigger)        |
| On Stopped   | Fires when async node is STOPPED via Cancel() or StopWaiting() |

**Canceling an Async Action:**

```
// Store reference to async action
Wait For Input Action (MyPC, "IA_Jump") → Store as "JumpListener"

// Later, cancel it when needed (fires OnStopped pin)
Stop Waiting For Input Action (JumpListener)
```

**C++ Usage:**

```cpp
// Create async action and store reference
UAsyncAction_WaitForInputAction* JumpListener =
    UAsyncAction_WaitForInputAction::WaitForInputAction(this, MyPC, FName("IA_Jump"), false);

// Bind to specific events
JumpListener->OnTriggered.AddDynamic(this, &AMyCharacter::OnJumpTriggered);
JumpListener->OnStarted.AddDynamic(this, &AMyCharacter::OnJumpStarted);
JumpListener->OnCompleted.AddDynamic(this, &AMyCharacter::OnJumpCompleted);
JumpListener->OnStopped.AddDynamic(this, &AMyCharacter::OnJumpListenerStopped);

// Activate the listener
JumpListener->Activate();

// Later, cancel if needed (will fire OnStopped)
JumpListener->Cancel();

// Or use BPL function (will also fire OnStopped)
UCPP_BPL_InputBinding::StopWaitingForInputAction(JumpListener);
```

### Which Approach Should I Use?

| Scenario                        | Recommended Approach   |
| ------------------------------- | ---------------------- |
| Simple game, few actions        | A (Global Dispatchers) |
| Complex game, many actions      | C (Async Nodes)        |
| Centralized input manager class | A                      |
| Per-widget/per-actor input      | C                      |
| Both at same time               | ✅ Works fine!         |

**Note:** Both approaches can be used simultaneously - they both receive events from the same internal routing system.

---

## 🎛️ Dynamic Input Modifiers & Triggers

P_MEIS provides full runtime control over UE5 Enhanced Input **Modifiers** (process input values) and **Triggers** (control when actions fire). This enables features like:

- **Dead Zones** - Ignore small stick movements
- **Sensitivity** - Scale input per-axis (mouse sensitivity, stick sensitivity)
- **Invert Y** - Flip vertical look axis
- **Hold/Tap/Pulse** - Different trigger behaviors at runtime
- **Chord Actions** - Require modifier keys (Sprint+Jump)

### Modifier Types

| Modifier Type              | Description                                            |
| -------------------------- | ------------------------------------------------------ |
| `DeadZone`                 | Ignore input below threshold (great for analog sticks) |
| `Scale`                    | Multiply input per-axis (sensitivity)                  |
| `Negate`                   | Invert axes (Invert Y look)                            |
| `Swizzle`                  | Reorder axes (swap X/Y)                                |
| `ResponseCurveExponential` | Non-linear response curve for more control             |
| `SmoothDelta`              | Smooth input over time                                 |
| `FOVScaling`               | Adjust sensitivity based on camera FOV                 |
| `ToWorldSpace`             | Transform input to world space                         |
| `ScaleByDeltaTime`         | Frame-rate independent scaling                         |

### Trigger Types

| Trigger Type     | Description                                          |
| ---------------- | ---------------------------------------------------- |
| `Down`           | Fire continuously while input is held (default)      |
| `Pressed`        | Fire once on initial press                           |
| `Released`       | Fire once on release                                 |
| `Hold`           | Fire after holding for specified duration            |
| `HoldAndRelease` | Fire on release after holding for specified duration |
| `Tap`            | Fire if pressed and released quickly                 |
| `Pulse`          | Fire repeatedly at interval while held               |
| `ChordAction`    | Require another action to be active (modifier keys)  |

### Quick Setup Examples

**Setting Dead Zone and Sensitivity:**

```cpp
// Blueprint-friendly: Set action-level dead zone
UCPP_BPL_InputBinding::SetActionDeadZone(PC, "IA_Move", 0.2f, 1.0f, EP_MEIS_DeadZoneType::Radial);

// Set sensitivity (scale) per axis
UCPP_BPL_InputBinding::SetActionSensitivityPerAxis(PC, "IA_Look", FVector(2.0f, 1.5f, 1.0f));

// Invert Y axis for "Invert Look" option
UCPP_BPL_InputBinding::SetActionInvertY(PC, "IA_Look", true);
```

**Setting Hold Trigger for Charge Attack:**

```cpp
// Make action require 0.5 second hold
UCPP_BPL_InputBinding::SetKeyHoldTrigger(PC, "IA_ChargeAttack", "LeftMouseButton", 0.5f, true);

// Set tap trigger for quick actions
UCPP_BPL_InputBinding::SetKeyTapTrigger(PC, "IA_Dodge", "SpaceBar", 0.2f);
```

### Advanced Modifier Configuration

Use `FS_InputModifierConfig` for full control:

```cpp
// Create a comprehensive modifier config
FS_InputModifierConfig DeadZoneConfig = FS_InputModifierConfig::MakeDeadZone(
    0.15f,  // Lower threshold
    1.0f,   // Upper threshold
    EP_MEIS_DeadZoneType::Radial
);

// Add to action
UCPP_BPL_InputBinding::AddModifierToAction(PC, "IA_Move", DeadZoneConfig);

// Create sensitivity modifier
FS_InputModifierConfig SensitivityConfig = FS_InputModifierConfig::MakeScale(FVector(2.0f, 2.0f, 1.0f));
UCPP_BPL_InputBinding::AddModifierToAction(PC, "IA_Look", SensitivityConfig);

// Create exponential response curve (more precision near center)
FS_InputModifierConfig CurveConfig = FS_InputModifierConfig::MakeResponseCurve(FVector(2.0f, 2.0f, 1.0f));
UCPP_BPL_InputBinding::AddModifierToAction(PC, "IA_Look", CurveConfig);
```

### Per-Key Mapping Modifiers

Override modifiers for specific key mappings (different settings for mouse vs gamepad):

```cpp
// Mouse look: high sensitivity
FS_InputModifierConfig MouseSensitivity = FS_InputModifierConfig::MakeScale(FVector(3.0f, 2.0f, 1.0f));
UCPP_BPL_InputBinding::AddModifierToKeyMapping(PC, "IA_Look", "Mouse2D", MouseSensitivity);

// Gamepad look: lower sensitivity with dead zone
FS_InputModifierConfig GamepadSensitivity = FS_InputModifierConfig::MakeScale(FVector(1.5f, 1.5f, 1.0f));
UCPP_BPL_InputBinding::AddModifierToKeyMapping(PC, "IA_Look", "Gamepad_RightStick_2D", GamepadSensitivity);

FS_InputModifierConfig GamepadDeadZone = FS_InputModifierConfig::MakeDeadZone(0.2f);
UCPP_BPL_InputBinding::AddModifierToKeyMapping(PC, "IA_Look", "Gamepad_RightStick_2D", GamepadDeadZone);
```

### Advanced Trigger Configuration

Use `FS_InputTriggerConfig` for full control:

```cpp
// Create hold trigger config
FS_InputTriggerConfig HoldConfig = FS_InputTriggerConfig::MakeHold(
    1.0f,   // Hold for 1 second
    true    // One-shot (fires once when threshold met)
);
UCPP_BPL_InputBinding::AddTriggerToKeyMapping(PC, "IA_Interact", "E", HoldConfig);

// Create pulse trigger for rapid fire
FS_InputTriggerConfig PulseConfig = FS_InputTriggerConfig::MakePulse(
    0.1f,   // Fire every 0.1 seconds
    0,      // Unlimited pulses
    true    // Fire immediately on press
);
UCPP_BPL_InputBinding::AddTriggerToKeyMapping(PC, "IA_Fire", "LeftMouseButton", PulseConfig);

// Create chord trigger (requires Sprint to be held)
FS_InputTriggerConfig ChordConfig = FS_InputTriggerConfig::MakeChord(FName("IA_Sprint"));
UCPP_BPL_InputBinding::AddTriggerToKeyMapping(PC, "IA_Slide", "LeftControl", ChordConfig);
```

### Modifier Helper Functions (Static Constructors)

`FS_InputModifierConfig` provides convenient static constructors:

| Function                           | Description                          |
| ---------------------------------- | ------------------------------------ |
| `MakeDeadZone(Lower, Upper, Type)` | Create dead zone modifier            |
| `MakeScale(FVector)`               | Create per-axis sensitivity modifier |
| `MakeUniformScale(float)`          | Create uniform sensitivity modifier  |
| `MakeNegate(X, Y, Z)`              | Create axis inversion modifier       |
| `MakeInvertY()`                    | Convenience for Y-axis inversion     |
| `MakeSwizzle(Order)`               | Create axis reorder modifier         |
| `MakeResponseCurve(FVector)`       | Create exponential response curve    |

### Trigger Helper Functions (Static Constructors)

`FS_InputTriggerConfig` provides convenient static constructors:

| Function                            | Description                   |
| ----------------------------------- | ----------------------------- |
| `MakeDown(Actuation)`               | Fire while held (default)     |
| `MakePressed(Actuation)`            | Fire once on press            |
| `MakeReleased(Actuation)`           | Fire once on release          |
| `MakeHold(Time, bOneShot)`          | Fire after holding            |
| `MakeHoldAndRelease(Time)`          | Fire on release after holding |
| `MakeTap(MaxTime)`                  | Fire on quick press-release   |
| `MakePulse(Interval, Limit, Start)` | Fire repeatedly while held    |
| `MakeChord(ActionName)`             | Require another action        |

---

## 📁 Folder Structure

```
P_MEIS/
├── P_MEIS.uplugin              # Plugin descriptor
├── README.md                   # This file
├── Resources/
│   └── Icon128.png             # Plugin icon
└── Source/P_MEIS/
    ├── P_MEIS.Build.cs         # Build configuration
    ├── Base/                   # Core implementation
    │   ├── InputBinding/       # Data structures
    │   │   ├── FS_InputActionBinding.h
    │   │   ├── FS_InputAxisBinding.h
    │   │   ├── FS_InputModifier.h           # Modifier configs (DeadZone, Scale, Negate, etc.)
    │   │   ├── FS_InputTriggerConfig.h      # NEW: Trigger configs (Hold, Tap, Pulse, etc.)
    │   │   ├── FS_InputProfile.h
    │   │   └── FS_PlayerInputData.h         # Per-player data struct
    │   ├── Manager/            # Core systems
    │   │   ├── CPP_InputBindingManager.h/cpp  # Central manager
    │   │   ├── CPP_BPL_InputBinding.h/cpp     # Blueprint library
    │   │   ├── CPP_InputContextManager.h/cpp
    │   │   ├── CPP_InputMacroSystem.h/cpp
    │   │   ├── CPP_InputAnalytics.h/cpp
    │   │   └── CPP_InputAccessibility.h/cpp
    │   ├── Storage/            # Profile persistence
    │   │   └── CPP_InputProfileStorage.h/cpp
    │   ├── Validation/         # Input validation
    │   │   └── CPP_InputValidator.h/cpp
    │   └── Integration/        # Enhanced Input bridge
    │       ├── CPP_EnhancedInputIntegration.h/cpp      # Integration wrapper + Modifier/Trigger management
    │       └── CPP_AsyncAction_WaitForInputAction.h/cpp # Async Blueprint node (Approach C)
    ├── Private/
    │   └── P_MEIS.cpp
    └── Public/
        └── P_MEIS.h
```

---

## 📚 Blueprint Function Library Reference

### Player Management (Multi-Player)

| Function                                 | Description                          |
| ---------------------------------------- | ------------------------------------ |
| `InitializeEnhancedInputIntegration(PC)` | Register player, returns Integration |
| `GetIntegrationForPlayer(PC)`            | Get player's Integration             |
| `UnregisterPlayer(PC)`                   | Clean up when player leaves          |
| `IsPlayerRegistered(PC)`                 | Check if player is registered        |

### Per-Player Profile Operations

| Function                                  | Description                         |
| ----------------------------------------- | ----------------------------------- |
| `LoadProfileForPlayer(PC, TemplateName)`  | Load template INTO player's profile |
| `SaveProfileForPlayer(PC, TemplateName)`  | Save player's profile AS a template |
| `GetProfileForPlayer(PC)`                 | Get player's current profile        |
| `ApplyTemplateToPlayer(PC, TemplateName)` | Apply template to player            |

### Profile Template Management (Global)

| Function                         | Description               |
| -------------------------------- | ------------------------- |
| `GetAvailableProfileTemplates()` | List all saved templates  |
| `CreateProfileTemplate(Name)`    | Create new empty template |
| `DeleteProfileTemplate(Name)`    | Delete template from disk |

### Dynamic Input Actions (Per-Player)

| Function                                                          | Description                       |
| ----------------------------------------------------------------- | --------------------------------- |
| `CreateDynamicInputAction(PC, Name, IsAxis)`                      | Create Input Action at runtime    |
| `GetDynamicInputAction(PC, Name)`                                 | Get existing Input Action         |
| `MapKeyToDynamicAction(PC, Name, Key)`                            | Map FKey to action                |
| `MapKeyStringToDynamicAction(PC, Name, KeyString)`                | Map key string to action          |
| `MapKeyBindingToDynamicAction(PC, Name, KeyBinding)`              | Map FS_KeyBinding with modifiers  |
| `MapKeyStringWithModifiers(PC, Name, Key, Shift, Ctrl, Alt, Cmd)` | Map key string with modifier keys |

### Key Utilities (Static)

| Function                                           | Description                     |
| -------------------------------------------------- | ------------------------------- |
| `StringToKey(String)`                              | Convert string to FKey          |
| `KeyToString(Key)`                                 | Convert FKey to string          |
| `IsValidKeyString(String)`                         | Check if key string is valid    |
| `GetAllKeyNames()`                                 | Get list of all valid key names |
| `MakeKeyBinding(KeyString, Shift, Ctrl, Alt, Cmd)` | Create key binding struct       |

### Modifiers & Triggers (Per-Player)

| Function                                                        | Description                               |
| --------------------------------------------------------------- | ----------------------------------------- |
| `AddModifierToAction(PC, ActionName, ModifierConfig)`           | Add modifier to action (all key mappings) |
| `RemoveModifierFromAction(PC, ActionName, ModifierType)`        | Remove modifier type from action          |
| `ClearActionModifiers(PC, ActionName)`                          | Remove all modifiers from action          |
| `GetActionModifiers(PC, ActionName)`                            | Get current action modifiers              |
| `AddModifierToKeyMapping(PC, ActionName, KeyString, Config)`    | Add modifier to specific key mapping      |
| `RemoveModifierFromKeyMapping(PC, ActionName, KeyString, Type)` | Remove modifier from key mapping          |
| `ClearKeyMappingModifiers(PC, ActionName, KeyString)`           | Clear all modifiers from key mapping      |
| `AddTriggerToKeyMapping(PC, ActionName, KeyString, Config)`     | Add trigger to key mapping                |
| `RemoveTriggerFromKeyMapping(PC, ActionName, KeyString, Type)`  | Remove trigger from key mapping           |
| `ClearKeyMappingTriggers(PC, ActionName, KeyString)`            | Clear all triggers from key mapping       |
| `SetKeyMappingTrigger(PC, ActionName, KeyString, Config)`       | Replace all triggers with single trigger  |

### Modifier Convenience Functions (Per-Player)

| Function                                                   | Description                     |
| ---------------------------------------------------------- | ------------------------------- |
| `SetActionDeadZone(PC, ActionName, Lower, Upper, Type)`    | Set dead zone for action        |
| `SetActionSensitivity(PC, ActionName, Sensitivity)`        | Set uniform sensitivity         |
| `SetActionSensitivityPerAxis(PC, ActionName, ScaleVec)`    | Set per-axis sensitivity        |
| `SetActionInvertY(PC, ActionName, bInvert)`                | Enable/disable Y-axis inversion |
| `SetKeyHoldTrigger(PC, ActionName, KeyString, Time, Once)` | Set hold trigger on key mapping |
| `SetKeyTapTrigger(PC, ActionName, KeyString, MaxTime)`     | Set tap trigger on key mapping  |

### Async Action Binding (Per-Action Events)

| Function                                        | Description                               |
| ----------------------------------------------- | ----------------------------------------- |
| `WaitForInputAction(PC, ActionName, bOnlyOnce)` | Async node with 6 output pins             |
| `StopWaitingForInputAction(AsyncAction)`        | Cancel async action (fires OnStopped pin) |
| `TryBindPendingActions(PC)`                     | Bind actions queued before InputComponent |

### Global Event Dispatchers (On Integration)

| Dispatcher          | Description                          |
| ------------------- | ------------------------------------ |
| `OnActionTriggered` | Fires for all actions when TRIGGERED |
| `OnActionStarted`   | Fires for all actions when STARTED   |
| `OnActionOngoing`   | Fires for all actions when ONGOING   |
| `OnActionCompleted` | Fires for all actions when COMPLETED |
| `OnActionCanceled`  | Fires for all actions when CANCELED  |

---

## 💾 Profile Storage

Profiles are saved as JSON in: `[Project]/Saved/InputProfiles/`

```json
{
  "ProfileName": "Default",
  "Description": "Default input configuration",
  "ActionBindings": [
    {
      "InputActionName": "IA_Jump",
      "DisplayName": "Jump",
      "Category": "Movement",
      "KeyBindings": [
        { "Key": "SpaceBar", "bShift": false, "bCtrl": false, "bAlt": false },
        {
          "Key": "Gamepad_FaceButton_Bottom",
          "bShift": false,
          "bCtrl": false,
          "bAlt": false
        }
      ]
    }
  ],
  "AxisBindings": [
    {
      "InputAxisName": "IA_Move",
      "DisplayName": "Move",
      "Category": "Movement",
      "AxisKeyBindings": [{ "Key": "Gamepad_LeftStick_2D", "Scale": 1.0 }],
      "DeadZone": 0.2,
      "Sensitivity": 1.0
    }
  ]
}
```

---

## ⚡ Advanced Features

- **Dynamic Input Modifiers** - Runtime control of DeadZone, Sensitivity, Invert Y, Response Curves, FOV Scaling
- **Dynamic Input Triggers** - Runtime control of Hold, Tap, Pulse, Chord triggers per key mapping
- **Blueprint Action Binding** - Two approaches: Global Dispatchers (A) and Async Nodes (C) for per-action events
- **Modifier Key Bindings** - Support for Shift+Key, Ctrl+Key, Alt+Key combinations via UInputTriggerChordAction
- **Deferred Binding** - Actions queued when InputComponent not ready; bind later with TryBindPendingActions()
- **Context-Aware Bindings** - Different bindings for Menu/Gameplay/Cutscene/Vehicle
- **Macro System** - Record and playback input sequences
- **Input Analytics** - Track most used keys, suggest unused keys
- **Accessibility** - Large text, high contrast, key hold/toggle options
- **Conflict Detection** - Automatic duplicate key warning per player
- **Hot-Reload** - Change bindings at runtime without restart

---

## 📊 Status

| Phase                      | Status         |
| -------------------------- | -------------- |
| Core Architecture          | ✅ Complete    |
| Per-Player Architecture    | ✅ Complete    |
| Data Persistence           | ✅ Complete    |
| UI/UX Widgets              | 🔄 In Progress |
| Validation                 | ✅ Complete    |
| Enhanced Input Integration | ✅ Complete    |
| Split-Screen Support       | ✅ Complete    |
| Advanced Features          | ✅ Complete    |
| Testing                    | ✅ Complete    |
| Documentation              | ✅ Complete    |

---

## 📜 License

See LICENSE file for details.

---

**Version:** 3.2  
**Last Updated:** December 7, 2025
