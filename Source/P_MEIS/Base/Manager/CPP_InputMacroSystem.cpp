/*
 * @Author: Punal Manalan
 * @Description: Input Macro System Implementation
 * @Date: 06/12/2025
 */

#include "Manager/CPP_InputMacroSystem.h"

bool UCPP_InputMacroSystem::RegisterMacro(const FS_InputMacro& Macro)
{
if (Macro.MacroName.IsNone() || Macro.Steps.Num() == 0)
{
UE_LOG(LogTemp, Warning, TEXT("P_MEIS: Cannot register macro - invalid name or no steps"));
return false;
}

RegisteredMacros.Add(Macro);
MacroCooldowns.Add(Macro.MacroName, 0.0f);
UE_LOG(LogTemp, Log, TEXT("P_MEIS: Registered macro: %s"), *Macro.MacroName.ToString());

return true;
}

bool UCPP_InputMacroSystem::PlayMacro(const FName& MacroName)
{
if (IsMacroPlaying(MacroName))
{
UE_LOG(LogTemp, Warning, TEXT("P_MEIS: Macro already playing: %s"), *MacroName.ToString());
return false;
}

// Check cooldown
if (MacroCooldowns.Contains(MacroName) && MacroCooldowns[MacroName] > 0.0f)
{
UE_LOG(LogTemp, Warning, TEXT("P_MEIS: Macro on cooldown: %s"), *MacroName.ToString());
return false;
}

PlayingMacros.Add(MacroName);
UE_LOG(LogTemp, Log, TEXT("P_MEIS: Playing macro: %s"), *MacroName.ToString());

return true;
}

bool UCPP_InputMacroSystem::StopMacro(const FName& MacroName)
{
if (PlayingMacros.Remove(MacroName) > 0)
{
UE_LOG(LogTemp, Log, TEXT("P_MEIS: Stopped macro: %s"), *MacroName.ToString());
return true;
}

return false;
}

bool UCPP_InputMacroSystem::DeleteMacro(const FName& MacroName)
{
auto RemoveByName = [&MacroName](const FS_InputMacro& Macro)
{
return Macro.MacroName == MacroName;
};

if (RegisteredMacros.RemoveAll(RemoveByName) > 0)
{
PlayingMacros.Remove(MacroName);
MacroCooldowns.Remove(MacroName);
UE_LOG(LogTemp, Log, TEXT("P_MEIS: Deleted macro: %s"), *MacroName.ToString());
return true;
}

return false;
}

bool UCPP_InputMacroSystem::GetMacro(const FName& MacroName, FS_InputMacro& OutMacro)
{
for (const FS_InputMacro& Macro : RegisteredMacros)
{
if (Macro.MacroName == MacroName)
{
OutMacro = Macro;
return true;
}
}

return false;
}

void UCPP_InputMacroSystem::GetAllMacros(TArray<FS_InputMacro>& OutMacros)
{
OutMacros = RegisteredMacros;
}

bool UCPP_InputMacroSystem::IsMacroPlaying(const FName& MacroName) const
{
return PlayingMacros.Contains(MacroName);
}
