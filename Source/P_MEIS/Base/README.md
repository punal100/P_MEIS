# P_MEIS Base Implementation

Core implementation code for the **Modular Enhanced Input System (P_MEIS)** - A Modular-style dynamic input binding system for Unreal Engine 5.

## 📁 Folder Structure

```
Base/
├── InputBinding/          # Core data structures
│   ├── FS_InputActionBinding.h     # Discrete action key mappings
│   ├── FS_InputAxisBinding.h       # Analog axis input mappings
│   ├── FS_InputModifier.h          # Input modifiers (curves, dead zones, etc.)
│   └── FS_InputProfile.h           # Complete profile snapshots
├── Manager/               # Core management system
│   ├── CPP_InputBindingManager.h   # Engine subsystem (singleton)
│   ├── CPP_InputBindingManager.cpp # Full implementation
│   ├── CPP_BPL_InputBinding.h      # Blueprint Function Library
│   └── CPP_BPL_InputBinding.cpp    # BPL implementation
├── Storage/               # Profile persistence layer
│   ├── CPP_InputProfileStorage.h   # File I/O and JSON serialization
│   └── CPP_InputProfileStorage.cpp # Storage implementation
├── Validation/            # Input validation system
│   ├── CPP_InputValidator.h        # Validation utilities
│   └── CPP_InputValidator.cpp      # Validator implementation
├── Integration/           # Enhanced Input System bridge
│   ├── CPP_EnhancedInputIntegration.h   # Integration wrapper
│   └── CPP_EnhancedInputIntegration.cpp # Integration implementation
└── README.md              # This file
```

## 🏗️ Architecture Overview

### **Phase 1: Core Architecture** ✅ (Complete)

#### Data Structures (InputBinding/)

- **FS_InputActionBinding** - Discrete action mappings with key modifiers (Shift, Ctrl, Alt, Cmd)
- **FS_InputAxisBinding** - Analog axis mappings with dead zone, sensitivity, invert flags
- **FS_InputModifier** - Input modifier types: DeadZone, ResponseCurve, Negate, Swizzle, Clamp, Scale
- **FS_InputProfile** - Complete profile snapshot containing all bindings and metadata

#### Manager System (Manager/)

- **UCPP_InputBindingManager** - Engine subsystem providing:
  - Profile management (Load, Save, Create, Delete, Rename, Duplicate, Reset)
  - Action/Axis binding operations
  - Validation and conflict detection
  - Event delegates for binding changes
  - Blueprint-callable functions for full BP access

#### Public API (Manager/)

- **UCPP_BPL_InputBinding** - Blueprint Function Library exposing:
  - 30+ Blueprint-callable functions
  - All manager operations accessible to Blueprints
  - Profile management and I/O operations
  - Validation and conflict detection

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

**Type:** Engine Subsystem (Singleton)

**Public Functions:**

```cpp
// Profile Operations
bool LoadProfile(const FName& ProfileName);
bool SaveProfile(const FName& ProfileName);
bool CreateProfile(const FName& ProfileName);
bool DeleteProfile(const FName& ProfileName);
bool RenameProfile(const FName& OldName, const FName& NewName);
bool DuplicateProfile(const FName& Source, const FName& NewName);
bool ResetToDefaults();
void GetAvailableProfiles(TArray<FName>& OutProfiles);

// Action Binding Operations
bool SetActionBinding(const FName& ActionName, const FS_InputActionBinding& Binding);
bool RemoveActionBinding(const FName& ActionName);
bool GetActionBinding(const FName& ActionName, FS_InputActionBinding& OutBinding);
void GetActionBindings(TArray<FS_InputActionBinding>& OutBindings);
bool ClearActionBindings();

// Axis Binding Operations
bool SetAxisBinding(const FName& AxisName, const FS_InputAxisBinding& Binding);
bool RemoveAxisBinding(const FName& AxisName);
bool GetAxisBinding(const FName& AxisName, FS_InputAxisBinding& OutBinding);
void GetAxisBindings(TArray<FS_InputAxisBinding>& OutBindings);
bool ClearAxisBindings();

// Validation
bool ValidateBinding(const FS_InputActionBinding& Binding, FString& OutErrorMessage);
void GetConflictingBindings(TArray<TPair<FName, FName>>& OutConflicts);
bool IsKeyBound(const FKey& Key);
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

**Type:** UObject Bridge

**Functions:**

```cpp
bool ApplyProfile(const FS_InputProfile& Profile);
bool ApplyActionBinding(const FS_InputActionBinding& ActionBinding);
bool ApplyAxisBinding(const FS_InputAxisBinding& AxisBinding);
void SetPlayerController(APlayerController* InPlayerController);
```

## 📊 Data Structures

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

Access the system via `UCPP_BPL_InputBinding` Blueprint Function Library:

```cpp
// Load a profile
UCPP_BPL_InputBinding::LoadProfile("MyProfile");

// Get all action bindings
TArray<FS_InputActionBinding> Bindings;
UCPP_BPL_InputBinding::GetActionBindings(Bindings);

// Check for conflicts
TArray<TPair<FName, FName>> Conflicts;
UCPP_BPL_InputBinding::GetConflictingBindings(Conflicts);

// Save current profile
UCPP_BPL_InputBinding::SaveProfile("MyProfile");

// Export profile to file
UCPP_BPL_InputBinding::ExportProfile("MyProfile", "/Path/To/Export.json");
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
- **Phase 4:** Advanced conflict resolution and context-aware bindings
- **Phase 5:** Enhanced Input System deep integration
- **Phase 6-8:** Advanced features, testing, and documentation

## 👨‍💻 Development Guidelines

1. **Modular Design** - Each module is independent and testable
2. **Blueprint Exposure** - All public functions use `UFUNCTION(BlueprintCallable)`
3. **Error Handling** - Validation and logging for all operations
4. **Delegates** - Event-based communication between systems
5. **Documentation** - Code comments follow Doxygen format for API docs

---

**Author:** Punal Manalan  
**Version:** 1.0 (Phase 1-2 Complete)  
**Last Updated:** December 6, 2025
