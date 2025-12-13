/*
 * @Author: Punal Manalan
 * @Description: Input Validator Implementation
 * @Date: 06/12/2025
 */

#include "Validation/CPP_InputValidator.h"

bool UCPP_InputValidator::ValidateKeyBinding(const FS_KeyBinding& KeyBinding, FString& OutErrorMessage)
{
if (!IsValidKey(KeyBinding.Key))
{
OutErrorMessage = FString::Printf(TEXT("Invalid key: %s"), *KeyBinding.Key.ToString());
return false;
}

if (KeyBinding.Value < -1.0f || KeyBinding.Value > 1.0f)
{
OutErrorMessage = TEXT("Key value must be between -1.0 and 1.0");
return false;
}

return true;
}

bool UCPP_InputValidator::ValidateAxisKeyBinding(const FS_AxisKeyBinding& AxisBinding, FString& OutErrorMessage)
{
if (!IsValidKey(AxisBinding.Key))
{
OutErrorMessage = FString::Printf(TEXT("Invalid axis key: %s"), *AxisBinding.Key.ToString());
return false;
}

if (AxisBinding.Scale < -10.0f || AxisBinding.Scale > 10.0f)
{
OutErrorMessage = TEXT("Axis scale must be between -10.0 and 10.0");
return false;
}

return true;
}

bool UCPP_InputValidator::ValidateActionBinding(const FS_InputActionBinding& ActionBinding, FString& OutErrorMessage)
{
if (ActionBinding.InputActionName.IsNone())
{
OutErrorMessage = TEXT("Action name cannot be empty");
return false;
}

if (ActionBinding.KeyBindings.Num() == 0)
{
OutErrorMessage = TEXT("Action must have at least one key binding");
return false;
}

for (const FS_KeyBinding& KeyBinding : ActionBinding.KeyBindings)
{
if (!ValidateKeyBinding(KeyBinding, OutErrorMessage))
{
return false;
}
}

return true;
}

bool UCPP_InputValidator::ValidateAxisBinding(const FS_InputAxisBinding& AxisBinding, FString& OutErrorMessage)
{
if (AxisBinding.InputAxisName.IsNone())
{
OutErrorMessage = TEXT("Axis name cannot be empty");
return false;
}

if (AxisBinding.DeadZone < 0.0f || AxisBinding.DeadZone > 1.0f)
{
OutErrorMessage = TEXT("DeadZone must be between 0.0 and 1.0");
return false;
}

if (AxisBinding.Sensitivity < 0.1f || AxisBinding.Sensitivity > 10.0f)
{
OutErrorMessage = TEXT("Sensitivity must be between 0.1 and 10.0");
return false;
}

for (const FS_AxisKeyBinding& AxisKeyBinding : AxisBinding.AxisBindings)
{
if (!ValidateAxisKeyBinding(AxisKeyBinding, OutErrorMessage))
{
return false;
}
}

return true;
}

bool UCPP_InputValidator::IsValidKey(const FKey& Key)
{
return !Key.IsValid() == false;
}

bool UCPP_InputValidator::IsValidGamepadKey(const FKey& Key)
{
const FString KeyName = Key.ToString();
return KeyName.Contains(TEXT("Gamepad")) || KeyName.Contains(TEXT("XBox"));
}

bool UCPP_InputValidator::DetectConflicts(const TArray<FS_InputActionBinding>& ActionBindings,
TArray<TPair<FName, FName>>& OutConflicts)
{
OutConflicts.Empty();

for (int32 i = 0; i < ActionBindings.Num(); ++i)
{
for (int32 j = i + 1; j < ActionBindings.Num(); ++j)
{
const FS_InputActionBinding& BindingA = ActionBindings[i];
const FS_InputActionBinding& BindingB = ActionBindings[j];

for (const FS_KeyBinding& KeyA : BindingA.KeyBindings)
{
for (const FS_KeyBinding& KeyB : BindingB.KeyBindings)
{
if (KeyA.Key == KeyB.Key && KeyA.bCtrl == KeyB.bCtrl && 
    KeyA.bShift == KeyB.bShift && KeyA.bAlt == KeyB.bAlt && 
    KeyA.bCmd == KeyB.bCmd)
{
OutConflicts.Add(TPair<FName, FName>(BindingA.InputActionName, BindingB.InputActionName));
}
}
}
}
}

return OutConflicts.Num() > 0;
}
