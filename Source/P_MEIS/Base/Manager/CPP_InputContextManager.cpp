/*
 * @Author: Punal Manalan
 * @Description: Input Context Manager Implementation
 * @Date: 06/12/2025
 */

#include "Manager/CPP_InputContextManager.h"
#include "Manager/CPP_InputBindingManager.h"

bool UCPP_InputContextManager::SetInputContext(EInputContext NewContext)
{
if (NewContext == CurrentContext)
{
return false;
}

CurrentContext = NewContext;

// Find and apply context profile
for (const FS_ContextBinding& Binding : ContextBindings)
{
if (Binding.Context == CurrentContext && Binding.bEnabled)
{
if (BindingManager)
{
UE_LOG(LogTemp, Log, TEXT("P_MEIS: Switching to context %d"), static_cast<int32>(CurrentContext));
return true;
}
}
}

return false;
}

bool UCPP_InputContextManager::RegisterContextProfile(EInputContext Context, const FS_InputProfile& Profile, float Priority)
{
FS_ContextBinding NewBinding;
NewBinding.Context = Context;
NewBinding.ContextProfile = Profile;
NewBinding.Priority = Priority;
NewBinding.bEnabled = true;

ContextBindings.Add(NewBinding);
UE_LOG(LogTemp, Log, TEXT("P_MEIS: Registered context profile for context %d"), static_cast<int32>(Context));

return true;
}

bool UCPP_InputContextManager::GetContextProfile(EInputContext Context, FS_InputProfile& OutProfile)
{
for (const FS_ContextBinding& Binding : ContextBindings)
{
if (Binding.Context == Context && Binding.bEnabled)
{
OutProfile = Binding.ContextProfile;
return true;
}
}

return false;
}

void UCPP_InputContextManager::ListContexts(TArray<uint8>& OutContexts)
{
OutContexts.Empty();
for (const FS_ContextBinding& Binding : ContextBindings)
{
if (Binding.bEnabled)
{
OutContexts.Add(static_cast<uint8>(Binding.Context));
}
}
}
