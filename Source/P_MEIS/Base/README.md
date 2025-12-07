# P_MEIS Base Implementation

Core implementation code for the **Modular Enhanced Input System (P_MEIS)** - A Modular-style dynamic input binding system for Unreal Engine 5 with **per-player architecture** for split-screen multiplayer support.

## 📁 Folder Structure

```
Base/
├── InputBinding/          # Core data structures
│   ├── FS_InputActionBinding.h     # Discrete action key mappings
│   ├── FS_InputAxisBinding.h       # Analog axis input mappings
│   ├── FS_InputModifier.h          # Input modifiers (curves, dead zones, etc.)
│   ├── FS_InputProfile.h           # Complete profile snapshots
│   └── FS_PlayerInputData.h        # Per-player runtime data (Profile + Integration)
├── Manager/               # Core management system
│   ├── CPP_InputBindingManager.h   # Engine subsystem (singleton) with per-player TMap
│   ├── CPP_InputBindingManager.cpp # Full implementation + ALL helper functions
│   ├── CPP_BPL_InputBinding.h      # Blueprint Function Library
│   └── CPP_BPL_InputBinding.cpp    # BPL implementation + ALL wrapper functions
├── Storage/               # Profile persistence layer
│   ├── CPP_InputProfileStorage.h   # File I/O and JSON serialization
│   └── CPP_InputProfileStorage.cpp # Storage implementation
├── Validation/            # Input validation system
│   ├── CPP_InputValidator.h        # Validation utilities
│   └── CPP_InputValidator.cpp      # Validator implementation
├── Integration/           # Enhanced Input System bridge
│   ├── CPP_EnhancedInputIntegration.h   # Integration wrapper (per-player) + Global Event Dispatchers
│   ├── CPP_EnhancedInputIntegration.cpp # Integration implementation
│   ├── CPP_AsyncAction_WaitForInputAction.h   # Async Blueprint node (Approach C)
│   └── CPP_AsyncAction_WaitForInputAction.cpp # Async action implementation
└── README.md              # This file
```

## 🏗️ Architecture Overview

### **Per-Player Architecture** ✅ (Core Principle)

Each `APlayerController` has their OWN:

- **ActiveProfile** (`FS_InputProfile`) - This player's key bindings
- **Integration** (`UCPP_EnhancedInputIntegration*`) - This player's Enhanced Input bridge
- **LoadedTemplateName** - Which template they loaded from

**Key Data Structure:**

```cpp
TMap<APlayerController*, FS_PlayerInputData> PlayerDataMap;  // Per-player runtime data
TMap<FName, FS_InputProfile> ProfileTemplates;               // Global template library (disk)
```

### **Phase 0-1: Core Architecture** ✅ (Complete)

#### Data Structures (InputBinding/)

- **FS_InputActionBinding** - Discrete action mappings with key modifiers (Shift, Ctrl, Alt, Cmd)
- **FS_InputAxisBinding** - Analog axis mappings with dead zone, sensitivity, invert flags
- **FS_InputModifier** - Input modifier types: DeadZone, ResponseCurve, Negate, Swizzle, Clamp, Scale
- **FS_InputProfile** - Complete profile snapshot containing all bindings and metadata
- **FS_PlayerInputData** - Per-player runtime data (Profile + Integration reference)

#### Manager System (Manager/)

- **UCPP_InputBindingManager** - Engine subsystem providing:
  - **Per-Player TMap** for split-screen support
  - Profile template management (Load, Save, Create, Delete)
  - Per-player action/axis binding operations
  - Comprehensive helper API (~40+ functions)
  - Validation and conflict detection
  - Event delegates for binding changes

#### Public API (Manager/)

- **UCPP_BPL_InputBinding** - Blueprint Function Library exposing:
  - 50+ Blueprint-callable functions
  - ALL manager operations accessible to Blueprints
  - Per-player profile management
  - Key category utilities (Keyboard, Gamepad, Mouse)
  - Full validation and conflict detection

### **Phase 2: Data Persistence** ✅ (Complete)

#### Storage System (Storage/)

- **UCPP_InputProfileStorage** - Static utility for:
  - JSON serialization of profiles
  - File I/O with error handling
  - Profile discovery and enumeration
  - Import/Export functionality
  - Complete action/axis/modifier serialization

## 🔧 Key Classes & Their Responsibilities

### UCPP_InputBindingManager

**Type:** Engine Subsystem (Singleton) with Per-Player TMap

**Core Data:**

```cpp
// Global template library (saved to disk)
TMap<FName, FS_InputProfile> ProfileTemplates;

// Per-player runtime data
TMap<APlayerController*, FS_PlayerInputData> PlayerDataMap;
```

**Player Management Functions:**

```cpp
UCPP_EnhancedInputIntegration* RegisterPlayer(APlayerController* PC);
void UnregisterPlayer(APlayerController* PC);
UCPP_EnhancedInputIntegration* GetIntegrationForPlayer(APlayerController* PC);
FS_InputProfile& GetProfileForPlayer(APlayerController* PC);
bool IsPlayerRegistered(APlayerController* PC) const;
int32 GetRegisteredPlayerCount() const;
TArray<APlayerController*> GetAllRegisteredPlayers();
```

**Template Management Functions:**

```cpp
bool LoadProfileTemplate(const FName& TemplateName);
bool SaveProfileTemplate(const FName& TemplateName, const FS_InputProfile& Profile);
bool CreateProfileTemplate(const FName& TemplateName);
bool DeleteProfileTemplate(const FName& TemplateName);
void GetAvailableTemplates(TArray<FName>& OutTemplates);
bool DoesTemplateExist(const FName& TemplateName) const;
int32 GetTemplateCount() const;
```

**Per-Player Profile Operations:**

```cpp
bool ApplyTemplateToPlayer(APlayerController* PC, const FName& TemplateName);
bool SavePlayerProfileAsTemplate(APlayerController* PC, const FName& TemplateName);
FName GetPlayerLoadedTemplateName(APlayerController* PC) const;
bool HasPlayerModifiedProfile(APlayerController* PC) const;
```

**Action Binding Functions:**

```cpp
// Getters
bool GetPlayerActionBinding(APlayerController* PC, const FName& ActionName, FS_InputActionBinding& OutBinding);
TArray<FS_InputActionBinding> GetPlayerActionBindings(APlayerController* PC);
TArray<FKey> GetKeysForAction(APlayerController* PC, const FName& ActionName);
FKey GetPrimaryKeyForAction(APlayerController* PC, const FName& ActionName);
TArray<FName> GetActionsForKey(APlayerController* PC, const FKey& Key);
bool DoesActionExist(APlayerController* PC, const FName& ActionName) const;
int32 GetKeyCountForAction(APlayerController* PC, const FName& ActionName) const;

// Setters
bool SetPlayerActionBinding(APlayerController* PC, const FName& ActionName, const FS_InputActionBinding& Binding);
bool SetPrimaryKeyForAction(APlayerController* PC, const FName& ActionName, const FKey& Key);
bool AddKeyToAction(APlayerController* PC, const FName& ActionName, const FKey& Key);
bool RemoveKeyFromAction(APlayerController* PC, const FName& ActionName, const FKey& Key);
bool ClearActionKeys(APlayerController* PC, const FName& ActionName);
bool RemovePlayerActionBinding(APlayerController* PC, const FName& ActionName);
bool SwapActionBindings(APlayerController* PC, const FName& ActionA, const FName& ActionB);
```

**Axis Binding Functions:**

```cpp
// Getters
bool GetPlayerAxisBinding(APlayerController* PC, const FName& AxisName, FS_InputAxisBinding& OutBinding);
TArray<FS_InputAxisBinding> GetPlayerAxisBindings(APlayerController* PC);
float GetAxisSensitivity(APlayerController* PC, const FName& AxisName) const;
float GetAxisDeadZone(APlayerController* PC, const FName& AxisName) const;

// Setters
bool SetPlayerAxisBinding(APlayerController* PC, const FName& AxisName, const FS_InputAxisBinding& Binding);
bool SetAxisSensitivity(APlayerController* PC, const FName& AxisName, float Sensitivity);
bool SetAxisDeadZone(APlayerController* PC, const FName& AxisName, float DeadZone);
```

**Validation Functions:**

```cpp
bool ValidateBinding(const FS_InputActionBinding& Binding, FString& OutErrorMessage);
void GetConflictingBindingsForPlayer(APlayerController* PC, TArray<TPair<FName, FName>>& OutConflicts);
bool IsKeyBoundForPlayer(APlayerController* PC, const FKey& Key);
```

**Delegates:**

```cpp
FOnInputBindingChanged - Broadcast when bindings change
FOnConflictDetected - Broadcast when key conflicts found
FOnProfileLoaded - Broadcast when profile loads
```

### UCPP_InputProfileStorage

**Type:** Static Utility Class

**Main Operations:**

```cpp
// Profile File Management
bool SaveProfile(const FS_InputProfile& Profile);
bool LoadProfile(const FName& ProfileName, FS_InputProfile& OutProfile);
bool DeleteProfile(const FName& ProfileName);
bool ProfileExists(const FName& ProfileName);
void GetAvailableProfiles(TArray<FName>& OutProfiles);

// Import/Export
bool ExportProfile(const FS_InputProfile& Profile, const FString& FilePath);
bool ImportProfile(const FString& FilePath, FS_InputProfile& OutProfile);

// JSON Serialization
FString SerializeProfileToJson(const FS_InputProfile& Profile);
bool DeserializeProfileFromJson(const FString& JsonString, FS_InputProfile& OutProfile);

// Utility
FString GetProfileDirectory();
FString GetProfileFilePath(const FName& ProfileName);
```

### UCPP_BPL_InputBinding

**Type:** Blueprint Function Library (Static)

All functions route through Manager - NO cached/static state.

**Player Management:**

```cpp
static UCPP_EnhancedInputIntegration* InitializeEnhancedInputIntegration(APlayerController* PC);
static UCPP_EnhancedInputIntegration* GetIntegrationForPlayer(APlayerController* PC);
static void UnregisterPlayer(APlayerController* PC);
static bool IsPlayerRegistered(APlayerController* PC);
static int32 GetRegisteredPlayerCount();
```

**Dynamic Input Actions:**

```cpp
static UInputAction* CreateDynamicInputAction(APlayerController* PC, const FName& ActionName, bool bIsAxis);
static UInputAction* GetDynamicInputAction(APlayerController* PC, const FName& ActionName);
static bool MapKeyToDynamicAction(APlayerController* PC, const FName& ActionName, const FKey& Key);
static bool MapKeyStringToDynamicAction(APlayerController* PC, const FName& ActionName, const FString& KeyString);
static bool DoesInputActionExist(APlayerController* PC, const FString& ActionName);
static UInputMappingContext* GetInputMappingContext(APlayerController* PC);
```

**Profile Operations (Per-Player):**

```cpp
static bool LoadProfileForPlayer(APlayerController* PC, const FString& TemplateName);
static bool SaveProfileForPlayer(APlayerController* PC, const FString& TemplateName);
static FS_InputProfile GetProfileForPlayer(APlayerController* PC);
static bool ApplyTemplateToPlayer(APlayerController* PC, const FString& TemplateName);
static FString GetPlayerLoadedTemplateName(APlayerController* PC);
static bool HasPlayerModifiedProfile(APlayerController* PC);
```

**Template Management (Global):**

```cpp
static TArray<FString> GetAvailableProfileTemplates();
static bool CreateProfileTemplate(const FString& TemplateName);
static bool DeleteProfileTemplate(const FString& TemplateName);
static bool DoesTemplateExist(const FString& TemplateName);
static int32 GetTemplateCount();
```

**Action Binding Operations:**

```cpp
// Getters
static TArray<FKey> GetKeysForAction(APlayerController* PC, const FString& ActionName);
static FKey GetPrimaryKeyForAction(APlayerController* PC, const FString& ActionName);
static TArray<FString> GetActionsForKey(APlayerController* PC, const FKey& Key);
static bool DoesActionExist(APlayerController* PC, const FString& ActionName);

// Setters
static bool SetPrimaryKeyForAction(APlayerController* PC, const FString& ActionName, const FKey& Key);
static bool AddKeyToAction(APlayerController* PC, const FString& ActionName, const FKey& Key);
static bool RemoveKeyFromAction(APlayerController* PC, const FString& ActionName, const FKey& Key);
static bool ClearActionKeys(APlayerController* PC, const FString& ActionName);
static bool SwapActionBindings(APlayerController* PC, const FString& ActionA, const FString& ActionB);
```

**Axis Binding Operations:**

```cpp
static float GetAxisSensitivity(APlayerController* PC, const FString& AxisName);
static bool SetAxisSensitivity(APlayerController* PC, const FString& AxisName, float Sensitivity);
static float GetAxisDeadZone(APlayerController* PC, const FString& AxisName);
static bool SetAxisDeadZone(APlayerController* PC, const FString& AxisName, float DeadZone);
```

**Validation & Conflicts:**

```cpp
static TArray<FName> GetConflictingBindings(APlayerController* PC);
static bool IsKeyBoundForPlayer(APlayerController* PC, const FKey& Key);
```

**Key Utilities (Static - No PC needed):**

```cpp
static FKey StringToKey(const FString& KeyString);
static FString KeyToString(const FKey& Key);
static FString KeyToDisplayString(const FKey& Key);
static bool IsValidKeyString(const FString& KeyString);
static TArray<FString> GetAllKeyNames();
static TArray<FKey> GetAllKeyboardKeys();
static TArray<FKey> GetAllGamepadKeys();
static TArray<FKey> GetAllMouseKeys();
```

### UCPP_InputValidator

**Type:** Static Utility Class

**Validation Functions:**

```cpp
bool ValidateKeyBinding(const FS_KeyBinding& KeyBinding, FString& OutErrorMessage);
bool ValidateAxisKeyBinding(const FS_AxisKeyBinding& AxisBinding, FString& OutErrorMessage);
bool ValidateActionBinding(const FS_InputActionBinding& Binding, FString& OutErrorMessage);
bool ValidateAxisBinding(const FS_InputAxisBinding& Binding, FString& OutErrorMessage);
bool IsValidKey(const FKey& Key);
bool IsValidGamepadKey(const FKey& Key);
bool DetectConflicts(const TArray<FS_InputActionBinding>& Bindings, TArray<TPair<FName, FName>>& OutConflicts);
```

### UCPP_EnhancedInputIntegration

**Type:** UObject Bridge - **100% Dynamic Input System**

**Core Functions:**

```cpp
// Profile Application
bool ApplyProfile(const FS_InputProfile& Profile);
bool ApplyActionBinding(const FS_InputActionBinding& ActionBinding);
bool ApplyAxisBinding(const FS_InputAxisBinding& AxisBinding);

// Player Controller
void SetPlayerController(APlayerController* InPlayerController);
APlayerController* GetPlayerController() const;

// Dynamic Input Action Creation (NEW!)
UInputAction* CreateInputAction(const FName& ActionName, EInputActionValueType ValueType = EInputActionValueType::Boolean);
UInputAction* GetInputAction(const FName& ActionName) const;
bool HasInputAction(const FName& ActionName) const;
void GetAllInputActions(TArray<UInputAction*>& OutActions) const;

// Key Mapping (NEW!)
bool MapKeyToAction(const FName& ActionName, const FKey& Key);
bool UnmapKeyFromAction(const FName& ActionName, const FKey& Key);
bool UnmapAllKeysFromAction(const FName& ActionName);
void ClearAllMappings();

// String to FKey Conversion (NEW!)
static FKey StringToKey(const FString& KeyString);
static FString KeyToString(const FKey& Key);
static bool IsValidKeyString(const FString& KeyString);
static void GetAllKeyNames(TArray<FString>& OutKeyNames);

// Mapping Context
UInputMappingContext* GetMappingContext() const;
bool RefreshMappingContext();
```

**Events:**

```cpp
FOnDynamicInputAction OnDynamicInputAction; // Broadcast when any dynamic action triggers
```

## 📊 Data Structures

### FS_PlayerInputData

Per-player runtime data (stored in Manager's PlayerDataMap):

- `FS_InputProfile ActiveProfile` - This player's current key bindings (their OWN copy)
- `UCPP_EnhancedInputIntegration* Integration` - This player's Enhanced Input bridge
- `FName LoadedTemplateName` - Which template they loaded from (for save/reload)

### FS_InputProfile

Complete snapshot of input configuration:

- `FName ProfileName` - Profile identifier
- `FText ProfileDescription` - User-friendly description
- `TArray<FS_InputActionBinding> ActionBindings` - All action mappings
- `TArray<FS_InputAxisBinding> AxisBindings` - All axis mappings
- `TArray<FS_InputModifier> Modifiers` - Global input modifiers
- `FDateTime Timestamp` - Creation/modification time
- `int32 Version` - Profile version for migration
- `FString CreatedBy` - Profile creator
- `bool bIsDefault` - Is this the default profile?
- `bool bIsCompetitive` - Competitive mode profile?

### FS_InputActionBinding

Discrete action mapping with key modifiers:

- `FName InputActionName` - Action identifier
- `FText DisplayName` - UI-friendly name
- `FName Category` - Organization category
- `TArray<FS_KeyBinding> KeyBindings` - Array of key bindings
- `float Priority` - Binding priority/weight
- `bool bEnabled` - Is binding active?

### FS_InputAxisBinding

Analog axis mapping with sensitivity:

- `FName InputAxisName` - Axis identifier
- `FName Category` - Organization category
- `TArray<FS_AxisKeyBinding> AxisBindings` - Axis key bindings
- `float DeadZone` - Dead zone threshold (0.0-1.0)
- `float Sensitivity` - Sensitivity multiplier (0.1-10.0)
- `bool bInvert` - Invert axis direction?
- `bool bEnabled` - Is binding active?

## 🎮 Usage from Blueprints

Access the system via `UCPP_BPL_InputBinding` Blueprint Function Library.

**IMPORTANT:** All operations require a `PlayerController` reference - there's no global state!

### Basic Setup Flow:

```cpp
// 1. Get Player Controller reference (from Character BeginPlay)
APlayerController* MyPC = GetController();

// 2. Initialize player (creates Integration + empty Profile)
UCPP_EnhancedInputIntegration* Integration = UCPP_BPL_InputBinding::InitializeEnhancedInputIntegration(MyPC);

// 3. Load a template profile for this player
UCPP_BPL_InputBinding::LoadProfileForPlayer(MyPC, "Default");

// 4. Create dynamic input actions
UInputAction* JumpAction = UCPP_BPL_InputBinding::CreateDynamicInputAction(MyPC, "IA_Jump", false);

// 5. Map keys to actions
UCPP_BPL_InputBinding::MapKeyStringToDynamicAction(MyPC, "IA_Jump", "SpaceBar");

// 6. Save player's profile as a template (optional)
UCPP_BPL_InputBinding::SaveProfileForPlayer(MyPC, "Player1_Custom");
```

### Rebinding Keys at Runtime:

```cpp
// Get current keys for action
TArray<FKey> JumpKeys = UCPP_BPL_InputBinding::GetKeysForAction(MyPC, "IA_Jump");

// Set new primary key
UCPP_BPL_InputBinding::SetPrimaryKeyForAction(MyPC, "IA_Jump", EKeys::E);

// Add secondary key
UCPP_BPL_InputBinding::AddKeyToAction(MyPC, "IA_Jump", EKeys::Gamepad_FaceButton_Bottom);

// Swap two action bindings
UCPP_BPL_InputBinding::SwapActionBindings(MyPC, "IA_Jump", "IA_Interact");
```

### Axis Binding Operations:

```cpp
// Get/Set sensitivity
float Sensitivity = UCPP_BPL_InputBinding::GetAxisSensitivity(MyPC, "IA_Look");
UCPP_BPL_InputBinding::SetAxisSensitivity(MyPC, "IA_Look", 1.5f);

// Get/Set dead zone
float DeadZone = UCPP_BPL_InputBinding::GetAxisDeadZone(MyPC, "IA_Move");
UCPP_BPL_InputBinding::SetAxisDeadZone(MyPC, "IA_Move", 0.15f);
```

### Utility Functions:

```cpp
// Get all keys by category (for UI dropdowns)
TArray<FKey> KeyboardKeys = UCPP_BPL_InputBinding::GetAllKeyboardKeys();
TArray<FKey> GamepadKeys = UCPP_BPL_InputBinding::GetAllGamepadKeys();
TArray<FKey> MouseKeys = UCPP_BPL_InputBinding::GetAllMouseKeys();

// Convert key to display string
FString DisplayName = UCPP_BPL_InputBinding::KeyToDisplayString(EKeys::SpaceBar); // "Space Bar"

// Check for conflicts
TArray<FName> Conflicts = UCPP_BPL_InputBinding::GetConflictingBindings(MyPC);
```

## 💾 Profile Storage Format

Profiles are stored as JSON in: `[ProjectRoot]/Saved/InputProfiles/`

**Example Profile JSON:**

```json
{
  "ProfileName": "Default",
  "ProfileDescription": "Default Input Profile",
  "CreatedBy": "System",
  "Version": 1,
  "bIsDefault": true,
  "bIsCompetitive": false,
  "ActionBindings": [
    {
      "InputActionName": "Jump",
      "DisplayName": "Jump",
      "Category": "Movement",
      "Description": "Jump action",
      "Priority": 1.0,
      "bEnabled": true
    }
  ],
  "AxisBindings": [
    {
      "InputAxisName": "MoveForward",
      "Category": "Movement",
      "DeadZone": 0.2,
      "Sensitivity": 1.0,
      "bInvert": false,
      "bEnabled": true
    }
  ],
  "Modifiers": []
}
```

## 🔍 Validation System

The validator ensures:

- ✅ All keys are valid UE5 keys
- ✅ Axis ranges are within acceptable bounds (DeadZone 0.0-1.0, Sensitivity 0.1-10.0)
- ✅ No duplicate key bindings (conflict detection)
- ✅ Gamepad compatibility checking
- ✅ All required fields are populated

## 🚀 Integration with Enhanced Input System

The `UCPP_EnhancedInputIntegration` class bridges P_MEIS with UE5's Enhanced Input System:

1. **Profile Application** - Converts P_MEIS profiles to Enhanced Input mappings
2. **Dynamic Remapping** - Hot-reload input bindings without restart
3. **Player Controller Integration** - Per-player input configuration
4. **Modifier Application** - Enhanced Input modifiers applied automatically

## 🎯 Blueprint Action Binding (Section 0.8)

P_MEIS provides two approaches for binding Blueprint events to dynamically created Input Actions:

### Approach A: Global Event Dispatchers

Use global dispatchers on the Integration class - ideal for simple games or centralized input handling.

**Events Available on Integration:**

- `OnActionTriggered` - Fires when action is TRIGGERED (main event)
- `OnActionStarted` - Fires when action is STARTED (initial press)
- `OnActionOngoing` - Fires when action is ONGOING (held down)
- `OnActionCompleted` - Fires when action is COMPLETED (released after trigger)
- `OnActionCanceled` - Fires when action is CANCELED (released before trigger)

**Blueprint Usage (Approach A):**

```
Event BeginPlay
    │
    ├─→ Initialize Enhanced Input Integration (MyPC) → Store as "MyIntegration"
    │
    ├─→ Create Dynamic Input Action (MyPC, "IA_Jump", false)
    │
    ├─→ Map Key String To Dynamic Action (MyPC, "IA_Jump", "SpaceBar")
    │
    └─→ MyIntegration → Bind Event to OnActionTriggered
                              │
                              └─→ Custom Event: OnAnyAction(ActionName, Value)
                                        │
                                        └─→ Switch on ActionName
                                                ├── "IA_Jump" → Jump()
                                                ├── "IA_Fire" → Fire()
                                                └── "IA_Move" → Move(Value)
```

**C++ Usage (Approach A):**

```cpp
// Get integration and bind to global dispatcher
UCPP_EnhancedInputIntegration* Integration = UCPP_BPL_InputBinding::GetIntegrationForPlayer(MyPC);
if (Integration)
{
    Integration->OnActionTriggered.AddDynamic(this, &AMyCharacter::HandleAnyActionTriggered);
}

// Handler function - filter by action name
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

### Approach C: Async Action Nodes (Wait For Input Action)

Use async Blueprint nodes for per-action binding - ideal for complex games or per-widget/per-actor input handling.

**Blueprint Node: "Wait For Input Action"**

- Creates a node with multiple output execution pins
- Filters by specific ActionName automatically
- Optional `bOnlyTriggerOnce` parameter
- Has `OnStopped` output pin that fires when the node is canceled

**Output Pins:**

| Pin          | Description                                                                  |
| ------------ | ---------------------------------------------------------------------------- |
| On Triggered | Fires when action is TRIGGERED (main event)                                  |
| On Started   | Fires when action is STARTED (initial press)                                 |
| On Ongoing   | Fires when action is ONGOING (held down)                                     |
| On Completed | Fires when action is COMPLETED (released after trigger)                      |
| On Canceled  | Fires when action is CANCELED (released before trigger)                      |
| On Stopped   | Fires when async node is STOPPED via Cancel() or StopWaitingForInputAction() |

**Blueprint Usage (Approach C):**

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

**Canceling an Async Action:**

```
// Store reference to async action
Wait For Input Action (MyPC, "IA_Jump") → Store as "JumpListener"

// Later, cancel it when needed
Stop Waiting For Input Action (JumpListener)
```

**C++ Usage (Approach C):**

```cpp
// Create async action and store reference
UAsyncAction_WaitForInputAction* JumpListener =
    UAsyncAction_WaitForInputAction::WaitForInputAction(this, MyPC, FName("IA_Jump"), false);

// Bind to specific events
JumpListener->OnTriggered.AddDynamic(this, &AMyCharacter::OnJumpTriggered);
JumpListener->OnStarted.AddDynamic(this, &AMyCharacter::OnJumpStarted);
JumpListener->OnCompleted.AddDynamic(this, &AMyCharacter::OnJumpCompleted);
JumpListener->OnStopped.AddDynamic(this, &AMyCharacter::OnJumpListenerStopped);  // NEW!

// Activate the listener
JumpListener->Activate();

// Later, cancel if needed (will fire OnStopped)
JumpListener->Cancel();

// Or use BPL function (will also fire OnStopped)
UCPP_BPL_InputBinding::StopWaitingForInputAction(JumpListener);
```

### Which Approach Should I Use?

| Scenario                   | Recommended Approach   |
| -------------------------- | ---------------------- |
| Simple game, few actions   | A (Global Dispatchers) |
| Complex game, many actions | C (Async Nodes)        |
| Centralized input manager  | A                      |
| Per-widget/per-actor input | C                      |
| Both at same time          | ✅ Works fine!         |

**Note:** Both approaches can be used simultaneously - they both receive events from the same internal routing system.

## 📋 Naming Conventions

All classes follow strict UE5 naming standards:

- **Classes:** `UCPP_*` (e.g., `UCPP_InputBindingManager`)
- **Structures:** `FS_*` (e.g., `FS_InputActionBinding`)
- **Enums:** `E*` (e.g., `EInputModifierType`)
- **Blueprint Function Libraries:** `UCPP_BPL_*`
- **Subsystems:** Extend `UEngineSubsystem`

## 🔗 Include Paths

Files in this Base folder are accessible via:

```cpp
#include "InputBinding/FS_InputProfile.h"
#include "Manager/CPP_InputBindingManager.h"
#include "Storage/CPP_InputProfileStorage.h"
#include "Validation/CPP_InputValidator.h"
#include "Integration/CPP_EnhancedInputIntegration.h"
```

## 📚 Next Phases

- **Phase 3:** UI/UX Widgets (WBP_InputSettings_Main, WBP_InputBinding_Row, etc.)
- **Phase 7:** Testing and performance benchmarks
- **Phase 8:** Additional documentation and examples

## 👨‍💻 Development Guidelines

1. **Per-Player Design** - All operations require PlayerController, no global state
2. **Modular Design** - Each module is independent and testable
3. **Blueprint Exposure** - All public functions use `UFUNCTION(BlueprintCallable)`
4. **Error Handling** - Validation and logging for all operations
5. **Delegates** - Event-based communication between systems
6. **Documentation** - Code comments follow Doxygen format for API docs

---

**Author:** Punal Manalan  
**Version:** 3.1 (Phase 0-6 Complete + Section 0.8 Blueprint Action Binding with OnStopped pin)  
**Last Updated:** December 7, 2025
