/*
 * @Author: Punal Manalan
 * @Description: Blueprint Function Library Implementation
 * @Date: 06/12/2025
 */

#include "Manager/CPP_BPL_InputBinding.h"
#include "Manager/CPP_InputBindingManager.h"
#include "Storage/CPP_InputProfileStorage.h"

UCPP_InputBindingManager *UCPP_BPL_InputBinding::GetManager()
{
    return GEngine ? GEngine->GetEngineSubsystem<UCPP_InputBindingManager>() : nullptr;
}

bool UCPP_BPL_InputBinding::LoadProfile(const FString &ProfileName)
{
    if (UCPP_InputBindingManager *Manager = GetManager())
    {
        return Manager->LoadProfile(FName(*ProfileName));
    }
    return false;
}

bool UCPP_BPL_InputBinding::SaveProfile(const FString &ProfileName)
{
    if (UCPP_InputBindingManager *Manager = GetManager())
    {
        return Manager->SaveProfile(FName(*ProfileName));
    }
    return false;
}

bool UCPP_BPL_InputBinding::CreateProfile(const FString &ProfileName)
{
    if (UCPP_InputBindingManager *Manager = GetManager())
    {
        return Manager->CreateProfile(FName(*ProfileName));
    }
    return false;
}

bool UCPP_BPL_InputBinding::DeleteProfile(const FString &ProfileName)
{
    if (UCPP_InputBindingManager *Manager = GetManager())
    {
        return Manager->DeleteProfile(FName(*ProfileName));
    }
    return false;
}

bool UCPP_BPL_InputBinding::RenameProfile(const FString &OldName, const FString &NewName)
{
    if (UCPP_InputBindingManager *Manager = GetManager())
    {
        return Manager->RenameProfile(FName(*OldName), FName(*NewName));
    }
    return false;
}

bool UCPP_BPL_InputBinding::DuplicateProfile(const FString &SourceProfile, const FString &NewProfile)
{
    if (UCPP_InputBindingManager *Manager = GetManager())
    {
        return Manager->DuplicateProfile(FName(*SourceProfile), FName(*NewProfile));
    }
    return false;
}

bool UCPP_BPL_InputBinding::ResetToDefaults()
{
    if (UCPP_InputBindingManager *Manager = GetManager())
    {
        return Manager->ResetToDefaults();
    }
    return false;
}

void UCPP_BPL_InputBinding::GetAvailableProfiles(TArray<FString> &OutProfiles)
{
    OutProfiles.Empty();
    TArray<FName> ProfileNames;

    if (UCPP_InputBindingManager *Manager = GetManager())
    {
        Manager->GetAvailableProfiles(ProfileNames);
        for (const FName &Name : ProfileNames)
        {
            OutProfiles.Add(Name.ToString());
        }
    }
}

bool UCPP_BPL_InputBinding::SetActionBinding(const FName &ActionName, const FS_InputActionBinding &Binding)
{
    if (UCPP_InputBindingManager *Manager = GetManager())
    {
        return Manager->SetActionBinding(ActionName, Binding);
    }
    return false;
}

bool UCPP_BPL_InputBinding::RemoveActionBinding(const FName &ActionName)
{
    if (UCPP_InputBindingManager *Manager = GetManager())
    {
        return Manager->RemoveActionBinding(ActionName);
    }
    return false;
}

bool UCPP_BPL_InputBinding::GetActionBinding(const FName &ActionName, FS_InputActionBinding &OutBinding)
{
    if (UCPP_InputBindingManager *Manager = GetManager())
    {
        return Manager->GetActionBinding(ActionName, OutBinding);
    }
    return false;
}

void UCPP_BPL_InputBinding::GetActionBindings(TArray<FS_InputActionBinding> &OutBindings)
{
    if (UCPP_InputBindingManager *Manager = GetManager())
    {
        Manager->GetActionBindings(OutBindings);
    }
}

bool UCPP_BPL_InputBinding::ClearActionBindings()
{
    if (UCPP_InputBindingManager *Manager = GetManager())
    {
        return Manager->ClearActionBindings();
    }
    return false;
}

bool UCPP_BPL_InputBinding::SetAxisBinding(const FName &AxisName, const FS_InputAxisBinding &Binding)
{
    if (UCPP_InputBindingManager *Manager = GetManager())
    {
        return Manager->SetAxisBinding(AxisName, Binding);
    }
    return false;
}

bool UCPP_BPL_InputBinding::RemoveAxisBinding(const FName &AxisName)
{
    if (UCPP_InputBindingManager *Manager = GetManager())
    {
        return Manager->RemoveAxisBinding(AxisName);
    }
    return false;
}

bool UCPP_BPL_InputBinding::GetAxisBinding(const FName &AxisName, FS_InputAxisBinding &OutBinding)
{
    if (UCPP_InputBindingManager *Manager = GetManager())
    {
        return Manager->GetAxisBinding(AxisName, OutBinding);
    }
    return false;
}

void UCPP_BPL_InputBinding::GetAxisBindings(TArray<FS_InputAxisBinding> &OutBindings)
{
    if (UCPP_InputBindingManager *Manager = GetManager())
    {
        Manager->GetAxisBindings(OutBindings);
    }
}

bool UCPP_BPL_InputBinding::ClearAxisBindings()
{
    if (UCPP_InputBindingManager *Manager = GetManager())
    {
        return Manager->ClearAxisBindings();
    }
    return false;
}

bool UCPP_BPL_InputBinding::ValidateBinding(const FS_InputActionBinding &Binding, FString &OutErrorMessage)
{
    if (UCPP_InputBindingManager *Manager = GetManager())
    {
        return Manager->ValidateBinding(Binding, OutErrorMessage);
    }
    OutErrorMessage = TEXT("Manager not available");
    return false;
}

void UCPP_BPL_InputBinding::GetConflictingBindings(TArray<TPair<FName, FName>> &OutConflicts)
{
    if (UCPP_InputBindingManager *Manager = GetManager())
    {
        Manager->GetConflictingBindings(OutConflicts);
    }
}

bool UCPP_BPL_InputBinding::IsKeyBound(const FKey &Key)
{
    if (UCPP_InputBindingManager *Manager = GetManager())
    {
        return Manager->IsKeyBound(Key);
    }
    return false;
}

bool UCPP_BPL_InputBinding::ExportProfile(const FString &ProfileName, const FString &ExportPath)
{
    if (UCPP_InputBindingManager *Manager = GetManager())
    {
        return Manager->ExportProfile(FName(*ProfileName), ExportPath);
    }
    return false;
}

bool UCPP_BPL_InputBinding::ImportProfile(const FString &ImportPath, FString &OutProfileName)
{
    if (UCPP_InputBindingManager *Manager = GetManager())
    {
        FS_InputProfile Profile;
        bool bSuccess = UCPP_InputProfileStorage::ImportProfile(ImportPath, Profile);
        if (bSuccess)
        {
            OutProfileName = Profile.ProfileName.ToString();
            return true;
        }
    }
    return false;
}

UCPP_InputBindingManager *UCPP_BPL_InputBinding::GetInputBindingManager()
{
    return GetManager();
}

FString UCPP_BPL_InputBinding::GetProfileDirectory()
{
    return UCPP_InputProfileStorage::GetProfileDirectory();
}

bool UCPP_BPL_InputBinding::ProfileExists(const FString &ProfileName)
{
    return UCPP_InputProfileStorage::ProfileExists(FName(*ProfileName));
}
