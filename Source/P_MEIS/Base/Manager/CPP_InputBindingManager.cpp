/*
 * @Author: Punal Manalan
 * @Description: Input Binding Manager Implementation
 * @Date: 06/12/2025
 */

#include "Manager/CPP_InputBindingManager.h"
#include "Integration/CPP_EnhancedInputIntegration.h"
#include "Storage/CPP_InputProfileStorage.h"

void UCPP_InputBindingManager::Initialize(FSubsystemCollectionBase &Collection)
{
    Super::Initialize(Collection);

    // Initialize Enhanced Input Integration
    InputIntegration = NewObject<UCPP_EnhancedInputIntegration>();

    // Load default profile
    if (!LoadDefaultProfile())
    {
        UE_LOG(LogTemp, Warning, TEXT("P_MEIS: Failed to load default input profile"));
    }

    UE_LOG(LogTemp, Log, TEXT("P_MEIS: Input Binding Manager Initialized"));
}

void UCPP_InputBindingManager::Deinitialize()
{
    if (InputIntegration)
    {
        InputIntegration->ConditionalBeginDestroy();
        InputIntegration = nullptr;
    }

    Super::Deinitialize();
    UE_LOG(LogTemp, Log, TEXT("P_MEIS: Input Binding Manager Deinitialized"));
}

bool UCPP_InputBindingManager::LoadProfile(const FName &ProfileName)
{
    if (UCPP_InputProfileStorage::LoadProfile(ProfileName, CurrentProfile))
    {
        ApplyProfileToEnhancedInput();
        return true;
    }
    return false;
}

bool UCPP_InputBindingManager::SaveProfile(const FName &ProfileName, const FText &Description)
{
    CurrentProfile.ProfileName = ProfileName;
    if (!Description.IsEmpty())
    {
        CurrentProfile.ProfileDescription = Description;
    }
    CurrentProfile.Timestamp = FDateTime::Now();

    return UCPP_InputProfileStorage::SaveProfile(CurrentProfile);
}

bool UCPP_InputBindingManager::CreateProfile(const FName &ProfileName, const FText &Description)
{
    CurrentProfile = FS_InputProfile();
    CurrentProfile.ProfileName = ProfileName;
    CurrentProfile.ProfileDescription = Description;
    CurrentProfile.Timestamp = FDateTime::Now();
    CurrentProfile.CreatedBy = FString(TEXT("User"));

    return SaveProfile(ProfileName, Description);
}

bool UCPP_InputBindingManager::DeleteProfile(const FName &ProfileName)
{
    return UCPP_InputProfileStorage::DeleteProfile(ProfileName);
}

bool UCPP_InputBindingManager::RenameProfile(const FName &OldName, const FName &NewName)
{
    if (UCPP_InputProfileStorage::LoadProfile(OldName, CurrentProfile))
    {
        CurrentProfile.ProfileName = NewName;
        UCPP_InputProfileStorage::DeleteProfile(OldName);
        return SaveProfile(NewName);
    }
    return false;
}

bool UCPP_InputBindingManager::DuplicateProfile(const FName &SourceProfile, const FName &NewProfileName)
{
    FS_InputProfile Profile;
    if (UCPP_InputProfileStorage::LoadProfile(SourceProfile, Profile))
    {
        Profile.ProfileName = NewProfileName;
        CurrentProfile = Profile;
        return SaveProfile(NewProfileName);
    }
    return false;
}

bool UCPP_InputBindingManager::ResetToDefaults()
{
    return LoadProfile(FName(TEXT("Default")));
}

void UCPP_InputBindingManager::GetAvailableProfiles(TArray<FName> &OutProfiles)
{
    UCPP_InputProfileStorage::GetAvailableProfiles(OutProfiles);
}

bool UCPP_InputBindingManager::SetActionBinding(const FName &ActionName, const FS_InputActionBinding &Binding)
{
    // Find or create action binding
    FS_InputActionBinding *ActionBindingPtr = CurrentProfile.ActionBindings.FindByPredicate(
        [ActionName](const FS_InputActionBinding &B)
        { return B.InputActionName == ActionName; });

    if (!ActionBindingPtr)
    {
        ActionBindingPtr = &CurrentProfile.ActionBindings.Add_GetRef(Binding);
    }
    else
    {
        *ActionBindingPtr = Binding;
    }

    return true;
}

bool UCPP_InputBindingManager::RemoveActionBinding(const FName &ActionName)
{
    int32 RemovedCount = CurrentProfile.ActionBindings.RemoveAll(
        [ActionName](const FS_InputActionBinding &Binding)
        { return Binding.InputActionName == ActionName; });
    return RemovedCount > 0;
}

bool UCPP_InputBindingManager::ClearActionBindings()
{
    CurrentProfile.ActionBindings.Empty();
    return true;
}

bool UCPP_InputBindingManager::GetActionBinding(const FName &ActionName, FS_InputActionBinding &OutBinding)
{
    const FS_InputActionBinding *ActionBinding = CurrentProfile.ActionBindings.FindByPredicate(
        [ActionName](const FS_InputActionBinding &Binding)
        { return Binding.InputActionName == ActionName; });

    if (ActionBinding)
    {
        OutBinding = *ActionBinding;
        return true;
    }
    return false;
}

void UCPP_InputBindingManager::GetActionBindings(TArray<FS_InputActionBinding> &OutBindings)
{
    for (const FS_InputActionBinding &Binding : CurrentProfile.ActionBindings)
    {
        OutBindings.Add(Binding);
    }
}

bool UCPP_InputBindingManager::SetAxisBinding(const FName &AxisName, const FS_InputAxisBinding &AxisBinding)
{
    FS_InputAxisBinding *AxisBindingPtr = CurrentProfile.AxisBindings.FindByPredicate(
        [AxisName](const FS_InputAxisBinding &B)
        { return B.InputAxisName == AxisName; });

    if (!AxisBindingPtr)
    {
        AxisBindingPtr = &CurrentProfile.AxisBindings.Add_GetRef(AxisBinding);
    }
    else
    {
        *AxisBindingPtr = AxisBinding;
    }

    return true;
}

bool UCPP_InputBindingManager::RemoveAxisBinding(const FName &AxisName)
{
    int32 Index = CurrentProfile.AxisBindings.FindLastByPredicate(
        [AxisName](const FS_InputAxisBinding &Binding)
        { return Binding.InputAxisName == AxisName; });

    if (Index != INDEX_NONE)
    {
        CurrentProfile.AxisBindings.RemoveAt(Index);
        return true;
    }
    return false;
}

bool UCPP_InputBindingManager::ClearAxisBindings()
{
    CurrentProfile.AxisBindings.Empty();
    return true;
}

bool UCPP_InputBindingManager::GetAxisBinding(const FName &AxisName, FS_InputAxisBinding &OutBinding)
{
    const FS_InputAxisBinding *AxisBinding = CurrentProfile.AxisBindings.FindByPredicate(
        [AxisName](const FS_InputAxisBinding &Binding)
        { return Binding.InputAxisName == AxisName; });

    if (AxisBinding)
    {
        OutBinding = *AxisBinding;
        return true;
    }
    return false;
}

void UCPP_InputBindingManager::GetAxisBindings(TArray<FS_InputAxisBinding> &OutBindings)
{
    for (const FS_InputAxisBinding &Binding : CurrentProfile.AxisBindings)
    {
        OutBindings.Add(Binding);
    }
}

bool UCPP_InputBindingManager::ValidateBinding(const FS_InputActionBinding &Binding, FString &OutErrorMessage)
{
    // Validate the binding structure
    if (Binding.InputActionName.IsNone())
    {
        OutErrorMessage = TEXT("Action name cannot be empty");
        return false;
    }

    if (Binding.KeyBindings.IsEmpty())
    {
        OutErrorMessage = TEXT("At least one key binding is required");
        return false;
    }

    OutErrorMessage.Empty();
    return true;
}

void UCPP_InputBindingManager::GetConflictingBindings(TArray<TPair<FName, FName>> &OutConflicts)
{
    // Simple implementation - check all action bindings for conflicts
    OutConflicts.Empty();
    // Can be implemented with more complex logic if needed
}

bool UCPP_InputBindingManager::IsKeyBound(const FKey &Key)
{
    for (const FS_InputActionBinding &Binding : CurrentProfile.ActionBindings)
    {
        for (const FS_KeyBinding &KeyBinding : Binding.KeyBindings)
        {
            if (KeyBinding.Key == Key)
            {
                return true;
            }
        }
    }
    return false;
}

bool UCPP_InputBindingManager::ExportProfile(const FName &ProfileName, const FString &FilePath)
{
    FS_InputProfile Profile;
    if (UCPP_InputProfileStorage::LoadProfile(ProfileName, Profile))
    {
        return UCPP_InputProfileStorage::ExportProfile(Profile, FilePath);
    }
    return false;
}

bool UCPP_InputBindingManager::ImportProfile(const FString &FilePath, FName &OutProfileName)
{
    if (UCPP_InputProfileStorage::ImportProfile(FilePath, CurrentProfile))
    {
        OutProfileName = CurrentProfile.ProfileName;
        return SaveProfile(CurrentProfile.ProfileName);
    }
    return false;
}

bool UCPP_InputBindingManager::LoadDefaultProfile()
{
    return LoadProfile(FName(TEXT("Default")));
}

bool UCPP_InputBindingManager::ApplyProfileToEnhancedInput()
{
    if (InputIntegration)
    {
        return InputIntegration->ApplyProfile(CurrentProfile);
    }
    return false;
}

void UCPP_InputBindingManager::BroadcastBindingChanges()
{
    // Binding changed - can be expanded with delegate broadcasts in future
}
