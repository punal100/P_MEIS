/*
 * @Author: Punal Manalan
 * @Description: Input Binding Manager Implementation - Per-Player Profile + Integration Architecture
 * @Date: 06/12/2025
 */

#include "Manager/CPP_InputBindingManager.h"
#include "Integration/CPP_EnhancedInputIntegration.h"
#include "Storage/CPP_InputProfileStorage.h"
#include "Validation/CPP_InputValidator.h"
#include "GameFramework/PlayerController.h"

void UCPP_InputBindingManager::Initialize(FSubsystemCollectionBase &Collection)
{
    Super::Initialize(Collection);

    // Load default template
    if (!LoadDefaultTemplate())
    {
        UE_LOG(LogTemp, Warning, TEXT("P_MEIS: Failed to load default profile template"));
    }

    UE_LOG(LogTemp, Log, TEXT("P_MEIS: Input Binding Manager Initialized (Per-Player Profile + Integration)"));
}

void UCPP_InputBindingManager::Deinitialize()
{
    // Clean up all player data
    for (auto &Pair : PlayerDataMap)
    {
        if (Pair.Value.Integration && Pair.Value.Integration->IsValidLowLevel())
        {
            Pair.Value.Integration->RemoveFromRoot();
            Pair.Value.Integration->ConditionalBeginDestroy();
        }
    }
    PlayerDataMap.Empty();
    ProfileTemplates.Empty();

    Super::Deinitialize();
    UE_LOG(LogTemp, Log, TEXT("P_MEIS: Input Binding Manager Deinitialized"));
}

// ==================== Player Management (Per-Player Data) ====================

UCPP_EnhancedInputIntegration *UCPP_InputBindingManager::RegisterPlayer(APlayerController *PlayerController)
{
    if (!PlayerController)
    {
        UE_LOG(LogTemp, Warning, TEXT("P_MEIS: Cannot register null PlayerController"));
        return nullptr;
    }

    // Clean up any invalid entries first
    CleanupInvalidPlayers();

    // Check if already registered
    if (FS_PlayerInputData *ExistingData = PlayerDataMap.Find(PlayerController))
    {
        if (ExistingData->IsValid())
        {
            UE_LOG(LogTemp, Log, TEXT("P_MEIS: Player already registered, returning existing Integration"));
            return ExistingData->Integration;
        }
    }

    // Create new Integration for this player
    UCPP_EnhancedInputIntegration *NewIntegration = NewObject<UCPP_EnhancedInputIntegration>();
    if (!NewIntegration)
    {
        UE_LOG(LogTemp, Error, TEXT("P_MEIS: Failed to create EnhancedInputIntegration"));
        return nullptr;
    }

    // Prevent garbage collection
    NewIntegration->AddToRoot();

    // Set the player controller
    NewIntegration->SetPlayerController(PlayerController);

    // Create player data with empty profile
    FS_PlayerInputData PlayerData;
    PlayerData.Integration = NewIntegration;
    PlayerData.ActiveProfile = FS_InputProfile();
    PlayerData.ActiveProfile.ProfileName = FName(*FString::Printf(TEXT("Player_%s"), *PlayerController->GetName()));
    PlayerData.LoadedTemplateName = NAME_None;

    // Store in map
    PlayerDataMap.Add(PlayerController, PlayerData);

    UE_LOG(LogTemp, Log, TEXT("P_MEIS: Registered player %s with dedicated profile"), *PlayerController->GetName());

    return NewIntegration;
}

void UCPP_InputBindingManager::UnregisterPlayer(APlayerController *PlayerController)
{
    if (!PlayerController)
    {
        return;
    }

    if (FS_PlayerInputData *PlayerData = PlayerDataMap.Find(PlayerController))
    {
        if (PlayerData->Integration && PlayerData->Integration->IsValidLowLevel())
        {
            PlayerData->Integration->ClearAllMappings();
            PlayerData->Integration->RemoveFromRoot();
            PlayerData->Integration->ConditionalBeginDestroy();
        }
        PlayerDataMap.Remove(PlayerController);
        UE_LOG(LogTemp, Log, TEXT("P_MEIS: Unregistered player %s"), *PlayerController->GetName());
    }
}

UCPP_EnhancedInputIntegration *UCPP_InputBindingManager::GetIntegrationForPlayer(APlayerController *PlayerController)
{
    if (!PlayerController)
    {
        UE_LOG(LogTemp, Warning, TEXT("P_MEIS: GetIntegrationForPlayer called with null PlayerController"));
        return nullptr;
    }

    // Clean up invalid entries
    CleanupInvalidPlayers();

    // Find existing
    if (FS_PlayerInputData *PlayerData = PlayerDataMap.Find(PlayerController))
    {
        if (PlayerData->IsValid())
        {
            return PlayerData->Integration;
        }
    }

    // Lazy init - register the player
    return RegisterPlayer(PlayerController);
}

FS_InputProfile UCPP_InputBindingManager::GetProfileForPlayer(APlayerController *PlayerController)
{
    if (!PlayerController)
    {
        UE_LOG(LogTemp, Warning, TEXT("P_MEIS: GetProfileForPlayer called with null PlayerController"));
        return FS_InputProfile();
    }

    if (FS_PlayerInputData *PlayerData = PlayerDataMap.Find(PlayerController))
    {
        return PlayerData->ActiveProfile;
    }

    // Player not registered - return empty profile
    return FS_InputProfile();
}

FS_InputProfile *UCPP_InputBindingManager::GetProfileRefForPlayer(APlayerController *PlayerController)
{
    if (!PlayerController)
    {
        return nullptr;
    }

    if (FS_PlayerInputData *PlayerData = PlayerDataMap.Find(PlayerController))
    {
        return &PlayerData->ActiveProfile;
    }

    return nullptr;
}

bool UCPP_InputBindingManager::HasPlayerRegistered(APlayerController *PlayerController) const
{
    if (!PlayerController)
    {
        return false;
    }

    if (const FS_PlayerInputData *PlayerData = PlayerDataMap.Find(PlayerController))
    {
        return PlayerData->IsValid();
    }
    return false;
}

void UCPP_InputBindingManager::GetRegisteredPlayers(TArray<APlayerController *> &OutPlayers) const
{
    OutPlayers.Empty();
    for (const auto &Pair : PlayerDataMap)
    {
        if (Pair.Key && Pair.Key->IsValidLowLevel() && Pair.Value.IsValid())
        {
            OutPlayers.Add(Pair.Key);
        }
    }
}

int32 UCPP_InputBindingManager::GetRegisteredPlayerCount() const
{
    int32 Count = 0;
    for (const auto &Pair : PlayerDataMap)
    {
        if (Pair.Key && Pair.Key->IsValidLowLevel() && Pair.Value.IsValid())
        {
            Count++;
        }
    }
    return Count;
}

FName UCPP_InputBindingManager::GetPlayerLoadedTemplateName(APlayerController *PlayerController) const
{
    if (const FS_PlayerInputData *PlayerData = GetPlayerData(PlayerController))
    {
        return PlayerData->LoadedTemplateName;
    }
    return NAME_None;
}

bool UCPP_InputBindingManager::HasPlayerModifiedProfile(APlayerController *PlayerController) const
{
    if (const FS_PlayerInputData *PlayerData = GetPlayerData(PlayerController))
    {
        // If no template was loaded, consider it modified
        if (PlayerData->LoadedTemplateName.IsNone())
        {
            return true;
        }

        // Compare current profile with original template
        FS_InputProfile OriginalTemplate;
        if (GetTemplate(PlayerData->LoadedTemplateName, OriginalTemplate))
        {
            // Simple check: compare action and axis binding counts
            // A more thorough check would compare all bindings
            if (PlayerData->ActiveProfile.ActionBindings.Num() != OriginalTemplate.ActionBindings.Num() ||
                PlayerData->ActiveProfile.AxisBindings.Num() != OriginalTemplate.AxisBindings.Num())
            {
                return true;
            }
        }
        return false;
    }
    return false;
}

void UCPP_InputBindingManager::CleanupInvalidPlayers()
{
    TArray<APlayerController *> InvalidPlayers;
    for (const auto &Pair : PlayerDataMap)
    {
        if (!Pair.Key || !Pair.Key->IsValidLowLevel() || !Pair.Value.IsValid())
        {
            InvalidPlayers.Add(Pair.Key);
        }
    }

    for (APlayerController *PC : InvalidPlayers)
    {
        if (FS_PlayerInputData *PlayerData = PlayerDataMap.Find(PC))
        {
            if (PlayerData->Integration && PlayerData->Integration->IsValidLowLevel())
            {
                PlayerData->Integration->RemoveFromRoot();
            }
        }
        PlayerDataMap.Remove(PC);
    }
}

FS_PlayerInputData *UCPP_InputBindingManager::GetPlayerData(APlayerController *PlayerController)
{
    if (!PlayerController)
    {
        return nullptr;
    }
    return PlayerDataMap.Find(PlayerController);
}

const FS_PlayerInputData *UCPP_InputBindingManager::GetPlayerData(APlayerController *PlayerController) const
{
    if (!PlayerController)
    {
        return nullptr;
    }
    return PlayerDataMap.Find(PlayerController);
}

// ==================== Profile Template Management (Global Library) ====================

bool UCPP_InputBindingManager::LoadProfileTemplate(const FName &TemplateName)
{
    FS_InputProfile Profile;
    if (UCPP_InputProfileStorage::LoadProfile(TemplateName, Profile))
    {
        ProfileTemplates.Add(TemplateName, Profile);
        UE_LOG(LogTemp, Log, TEXT("P_MEIS: Loaded template '%s'"), *TemplateName.ToString());
        return true;
    }
    return false;
}

bool UCPP_InputBindingManager::SaveProfileTemplate(const FName &TemplateName, const FS_InputProfile &Profile)
{
    FS_InputProfile TemplateProfile = Profile;
    TemplateProfile.ProfileName = TemplateName;
    TemplateProfile.Timestamp = FDateTime::Now();

    if (UCPP_InputProfileStorage::SaveProfile(TemplateProfile))
    {
        ProfileTemplates.Add(TemplateName, TemplateProfile);
        UE_LOG(LogTemp, Log, TEXT("P_MEIS: Saved template '%s'"), *TemplateName.ToString());
        return true;
    }
    return false;
}

bool UCPP_InputBindingManager::CreateProfileTemplate(const FName &TemplateName, const FText &Description)
{
    FS_InputProfile NewTemplate;
    NewTemplate.ProfileName = TemplateName;
    NewTemplate.ProfileDescription = Description;
    NewTemplate.Timestamp = FDateTime::Now();
    NewTemplate.CreatedBy = FString(TEXT("User"));

    return SaveProfileTemplate(TemplateName, NewTemplate);
}

bool UCPP_InputBindingManager::DeleteProfileTemplate(const FName &TemplateName)
{
    ProfileTemplates.Remove(TemplateName);
    return UCPP_InputProfileStorage::DeleteProfile(TemplateName);
}

void UCPP_InputBindingManager::GetAvailableTemplates(TArray<FName> &OutTemplates)
{
    UCPP_InputProfileStorage::GetAvailableProfiles(OutTemplates);
}

bool UCPP_InputBindingManager::HasTemplate(const FName &TemplateName) const
{
    return ProfileTemplates.Contains(TemplateName);
}

bool UCPP_InputBindingManager::DoesTemplateExist(const FName &TemplateName) const
{
    // Check in-memory first, then disk
    if (ProfileTemplates.Contains(TemplateName))
    {
        return true;
    }
    return UCPP_InputProfileStorage::ProfileExists(TemplateName);
}

int32 UCPP_InputBindingManager::GetTemplateCount() const
{
    return ProfileTemplates.Num();
}

bool UCPP_InputBindingManager::GetTemplate(const FName &TemplateName, FS_InputProfile &OutProfile) const
{
    if (const FS_InputProfile *Template = ProfileTemplates.Find(TemplateName))
    {
        OutProfile = *Template;
        return true;
    }

    // Try loading from disk
    return UCPP_InputProfileStorage::LoadProfile(TemplateName, OutProfile);
}

// ==================== Per-Player Profile Operations ====================

bool UCPP_InputBindingManager::ApplyTemplateToPlayer(APlayerController *PlayerController, const FName &TemplateName)
{
    if (!PlayerController)
    {
        UE_LOG(LogTemp, Warning, TEXT("P_MEIS: ApplyTemplateToPlayer - null PlayerController"));
        return false;
    }

    // Ensure player is registered
    GetIntegrationForPlayer(PlayerController);

    FS_PlayerInputData *PlayerData = GetPlayerData(PlayerController);
    if (!PlayerData)
    {
        UE_LOG(LogTemp, Warning, TEXT("P_MEIS: ApplyTemplateToPlayer - player not registered"));
        return false;
    }

    // Get the template
    FS_InputProfile Template;
    if (!GetTemplate(TemplateName, Template))
    {
        // Try loading from disk
        if (!LoadProfileTemplate(TemplateName))
        {
            UE_LOG(LogTemp, Warning, TEXT("P_MEIS: ApplyTemplateToPlayer - template '%s' not found"), *TemplateName.ToString());
            return false;
        }
        Template = ProfileTemplates[TemplateName];
    }

    // Copy template to player's active profile
    PlayerData->ActiveProfile = Template;
    PlayerData->ActiveProfile.ProfileName = FName(*FString::Printf(TEXT("Player_%s"), *PlayerController->GetName()));
    PlayerData->LoadedTemplateName = TemplateName;

    // Apply to Enhanced Input
    return ApplyPlayerProfileToEnhancedInput(PlayerController);
}

bool UCPP_InputBindingManager::SavePlayerProfileAsTemplate(APlayerController *PlayerController, const FName &TemplateName)
{
    if (!PlayerController)
    {
        return false;
    }

    const FS_PlayerInputData *PlayerData = GetPlayerData(PlayerController);
    if (!PlayerData)
    {
        UE_LOG(LogTemp, Warning, TEXT("P_MEIS: SavePlayerProfileAsTemplate - player not registered"));
        return false;
    }

    return SaveProfileTemplate(TemplateName, PlayerData->ActiveProfile);
}

bool UCPP_InputBindingManager::ApplyPlayerProfileToEnhancedInput(APlayerController *PlayerController)
{
    FS_PlayerInputData *PlayerData = GetPlayerData(PlayerController);
    if (!PlayerData || !PlayerData->IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("P_MEIS: ApplyPlayerProfileToEnhancedInput - player not valid"));
        return false;
    }

    // Debug: Log all axis bindings and their value types
    for (const FS_InputAxisBinding &AxisBinding : PlayerData->ActiveProfile.AxisBindings)
    {
        UE_LOG(LogTemp, Log, TEXT("P_MEIS: ApplyPlayerProfileToEnhancedInput - AxisBinding '%s' has ValueType: %d"),
               *AxisBinding.InputAxisName.ToString(), static_cast<int32>(AxisBinding.ValueType));
    }

    return PlayerData->Integration->ApplyProfile(PlayerData->ActiveProfile);
}

// ==================== Per-Player Action Binding Operations ====================

bool UCPP_InputBindingManager::SetPlayerActionBinding(APlayerController *PlayerController, const FName &ActionName, const FS_InputActionBinding &Binding)
{
    FS_InputProfile *Profile = GetProfileRefForPlayer(PlayerController);
    if (!Profile)
    {
        return false;
    }

    // Find or create action binding
    FS_InputActionBinding *ActionBindingPtr = Profile->ActionBindings.FindByPredicate(
        [ActionName](const FS_InputActionBinding &B)
        { return B.InputActionName == ActionName; });

    if (!ActionBindingPtr)
    {
        ActionBindingPtr = &Profile->ActionBindings.Add_GetRef(Binding);
    }
    else
    {
        *ActionBindingPtr = Binding;
    }

    return true;
}

bool UCPP_InputBindingManager::GetPlayerActionBinding(APlayerController *PlayerController, const FName &ActionName, FS_InputActionBinding &OutBinding)
{
    const FS_PlayerInputData *PlayerData = GetPlayerData(PlayerController);
    if (!PlayerData)
    {
        return false;
    }

    const FS_InputActionBinding *ActionBinding = PlayerData->ActiveProfile.ActionBindings.FindByPredicate(
        [ActionName](const FS_InputActionBinding &Binding)
        { return Binding.InputActionName == ActionName; });

    if (ActionBinding)
    {
        OutBinding = *ActionBinding;
        return true;
    }
    return false;
}

bool UCPP_InputBindingManager::RemovePlayerActionBinding(APlayerController *PlayerController, const FName &ActionName)
{
    FS_InputProfile *Profile = GetProfileRefForPlayer(PlayerController);
    if (!Profile)
    {
        return false;
    }

    int32 RemovedCount = Profile->ActionBindings.RemoveAll(
        [ActionName](const FS_InputActionBinding &Binding)
        { return Binding.InputActionName == ActionName; });
    return RemovedCount > 0;
}

void UCPP_InputBindingManager::GetPlayerActionBindings(APlayerController *PlayerController, TArray<FS_InputActionBinding> &OutBindings)
{
    OutBindings.Empty();
    const FS_PlayerInputData *PlayerData = GetPlayerData(PlayerController);
    if (PlayerData)
    {
        OutBindings = PlayerData->ActiveProfile.ActionBindings;
    }
}

// ==================== Per-Player Axis Binding Operations ====================

bool UCPP_InputBindingManager::SetPlayerAxisBinding(APlayerController *PlayerController, const FName &AxisName, const FS_InputAxisBinding &Binding)
{
    FS_InputProfile *Profile = GetProfileRefForPlayer(PlayerController);
    if (!Profile)
    {
        return false;
    }

    UE_LOG(LogTemp, Log, TEXT("P_MEIS: SetPlayerAxisBinding '%s' with ValueType: %d"),
           *AxisName.ToString(), static_cast<int32>(Binding.ValueType));

    FS_InputAxisBinding *AxisBindingPtr = Profile->AxisBindings.FindByPredicate(
        [AxisName](const FS_InputAxisBinding &B)
        { return B.InputAxisName == AxisName; });

    if (!AxisBindingPtr)
    {
        AxisBindingPtr = &Profile->AxisBindings.Add_GetRef(Binding);
    }
    else
    {
        *AxisBindingPtr = Binding;
    }

    UE_LOG(LogTemp, Log, TEXT("P_MEIS: After SetPlayerAxisBinding '%s', stored ValueType: %d"),
           *AxisName.ToString(), static_cast<int32>(AxisBindingPtr->ValueType));

    return true;
}

bool UCPP_InputBindingManager::GetPlayerAxisBinding(APlayerController *PlayerController, const FName &AxisName, FS_InputAxisBinding &OutBinding)
{
    const FS_PlayerInputData *PlayerData = GetPlayerData(PlayerController);
    if (!PlayerData)
    {
        return false;
    }

    const FS_InputAxisBinding *AxisBinding = PlayerData->ActiveProfile.AxisBindings.FindByPredicate(
        [AxisName](const FS_InputAxisBinding &Binding)
        { return Binding.InputAxisName == AxisName; });

    if (AxisBinding)
    {
        OutBinding = *AxisBinding;
        return true;
    }
    return false;
}

bool UCPP_InputBindingManager::RemovePlayerAxisBinding(APlayerController *PlayerController, const FName &AxisName)
{
    FS_InputProfile *Profile = GetProfileRefForPlayer(PlayerController);
    if (!Profile)
    {
        return false;
    }

    int32 Index = Profile->AxisBindings.FindLastByPredicate(
        [AxisName](const FS_InputAxisBinding &Binding)
        { return Binding.InputAxisName == AxisName; });

    if (Index != INDEX_NONE)
    {
        Profile->AxisBindings.RemoveAt(Index);
        return true;
    }
    return false;
}

void UCPP_InputBindingManager::GetPlayerAxisBindings(APlayerController *PlayerController, TArray<FS_InputAxisBinding> &OutBindings)
{
    OutBindings.Empty();
    const FS_PlayerInputData *PlayerData = GetPlayerData(PlayerController);
    if (PlayerData)
    {
        OutBindings = PlayerData->ActiveProfile.AxisBindings;
    }
}

// ==================== Action Binding Helper Functions ====================

bool UCPP_InputBindingManager::DoesActionExist(APlayerController *PlayerController, const FName &ActionName) const
{
    const FS_PlayerInputData *PlayerData = GetPlayerData(PlayerController);
    if (!PlayerData)
    {
        return false;
    }

    return PlayerData->ActiveProfile.ActionBindings.ContainsByPredicate(
        [ActionName](const FS_InputActionBinding &Binding)
        { return Binding.InputActionName == ActionName; });
}

void UCPP_InputBindingManager::GetKeysForAction(APlayerController *PlayerController, const FName &ActionName, TArray<FKey> &OutKeys)
{
    OutKeys.Empty();
    const FS_PlayerInputData *PlayerData = GetPlayerData(PlayerController);
    if (!PlayerData)
    {
        return;
    }

    const FS_InputActionBinding *ActionBinding = PlayerData->ActiveProfile.ActionBindings.FindByPredicate(
        [ActionName](const FS_InputActionBinding &Binding)
        { return Binding.InputActionName == ActionName; });

    if (ActionBinding)
    {
        for (const FS_KeyBinding &KeyBinding : ActionBinding->KeyBindings)
        {
            OutKeys.Add(KeyBinding.Key);
        }
    }
}

FKey UCPP_InputBindingManager::GetPrimaryKeyForAction(APlayerController *PlayerController, const FName &ActionName)
{
    const FS_PlayerInputData *PlayerData = GetPlayerData(PlayerController);
    if (!PlayerData)
    {
        return EKeys::Invalid;
    }

    const FS_InputActionBinding *ActionBinding = PlayerData->ActiveProfile.ActionBindings.FindByPredicate(
        [ActionName](const FS_InputActionBinding &Binding)
        { return Binding.InputActionName == ActionName; });

    if (ActionBinding && ActionBinding->KeyBindings.Num() > 0)
    {
        return ActionBinding->KeyBindings[0].Key;
    }
    return EKeys::Invalid;
}

void UCPP_InputBindingManager::GetActionsForKey(APlayerController *PlayerController, const FKey &Key, TArray<FName> &OutActions)
{
    OutActions.Empty();
    const FS_PlayerInputData *PlayerData = GetPlayerData(PlayerController);
    if (!PlayerData)
    {
        return;
    }

    for (const FS_InputActionBinding &ActionBinding : PlayerData->ActiveProfile.ActionBindings)
    {
        for (const FS_KeyBinding &KeyBinding : ActionBinding.KeyBindings)
        {
            if (KeyBinding.Key == Key)
            {
                OutActions.AddUnique(ActionBinding.InputActionName);
                break;
            }
        }
    }
}

int32 UCPP_InputBindingManager::GetKeyCountForAction(APlayerController *PlayerController, const FName &ActionName) const
{
    const FS_PlayerInputData *PlayerData = GetPlayerData(PlayerController);
    if (!PlayerData)
    {
        return 0;
    }

    const FS_InputActionBinding *ActionBinding = PlayerData->ActiveProfile.ActionBindings.FindByPredicate(
        [ActionName](const FS_InputActionBinding &Binding)
        { return Binding.InputActionName == ActionName; });

    if (ActionBinding)
    {
        return ActionBinding->KeyBindings.Num();
    }
    return 0;
}

bool UCPP_InputBindingManager::SetPrimaryKeyForAction(APlayerController *PlayerController, const FName &ActionName, const FKey &Key)
{
    FS_InputProfile *Profile = GetProfileRefForPlayer(PlayerController);
    if (!Profile)
    {
        return false;
    }

    FS_InputActionBinding *ActionBinding = Profile->ActionBindings.FindByPredicate(
        [ActionName](const FS_InputActionBinding &Binding)
        { return Binding.InputActionName == ActionName; });

    if (!ActionBinding)
    {
        // Create new action binding
        FS_InputActionBinding NewBinding;
        NewBinding.InputActionName = ActionName;
        FS_KeyBinding KeyBinding;
        KeyBinding.Key = Key;
        NewBinding.KeyBindings.Add(KeyBinding);
        Profile->ActionBindings.Add(NewBinding);
        return true;
    }

    // Set primary key (index 0)
    if (ActionBinding->KeyBindings.Num() > 0)
    {
        ActionBinding->KeyBindings[0].Key = Key;
    }
    else
    {
        FS_KeyBinding KeyBinding;
        KeyBinding.Key = Key;
        ActionBinding->KeyBindings.Add(KeyBinding);
    }
    return true;
}

bool UCPP_InputBindingManager::AddKeyToAction(APlayerController *PlayerController, const FName &ActionName, const FKey &Key)
{
    FS_InputProfile *Profile = GetProfileRefForPlayer(PlayerController);
    if (!Profile)
    {
        return false;
    }

    FS_InputActionBinding *ActionBinding = Profile->ActionBindings.FindByPredicate(
        [ActionName](const FS_InputActionBinding &Binding)
        { return Binding.InputActionName == ActionName; });

    if (!ActionBinding)
    {
        // Create new action binding
        FS_InputActionBinding NewBinding;
        NewBinding.InputActionName = ActionName;
        FS_KeyBinding KeyBinding;
        KeyBinding.Key = Key;
        NewBinding.KeyBindings.Add(KeyBinding);
        Profile->ActionBindings.Add(NewBinding);
        return true;
    }

    // Add key to existing bindings
    FS_KeyBinding KeyBinding;
    KeyBinding.Key = Key;
    ActionBinding->KeyBindings.Add(KeyBinding);
    return true;
}

bool UCPP_InputBindingManager::RemoveKeyFromAction(APlayerController *PlayerController, const FName &ActionName, const FKey &Key)
{
    FS_InputProfile *Profile = GetProfileRefForPlayer(PlayerController);
    if (!Profile)
    {
        return false;
    }

    FS_InputActionBinding *ActionBinding = Profile->ActionBindings.FindByPredicate(
        [ActionName](const FS_InputActionBinding &Binding)
        { return Binding.InputActionName == ActionName; });

    if (!ActionBinding)
    {
        return false;
    }

    int32 RemovedCount = ActionBinding->KeyBindings.RemoveAll(
        [Key](const FS_KeyBinding &Binding)
        { return Binding.Key == Key; });
    return RemovedCount > 0;
}

bool UCPP_InputBindingManager::ClearActionKeys(APlayerController *PlayerController, const FName &ActionName)
{
    FS_InputProfile *Profile = GetProfileRefForPlayer(PlayerController);
    if (!Profile)
    {
        return false;
    }

    FS_InputActionBinding *ActionBinding = Profile->ActionBindings.FindByPredicate(
        [ActionName](const FS_InputActionBinding &Binding)
        { return Binding.InputActionName == ActionName; });

    if (!ActionBinding)
    {
        return false;
    }

    ActionBinding->KeyBindings.Empty();
    return true;
}

bool UCPP_InputBindingManager::SwapActionBindings(APlayerController *PlayerController, const FName &ActionA, const FName &ActionB)
{
    FS_InputProfile *Profile = GetProfileRefForPlayer(PlayerController);
    if (!Profile)
    {
        return false;
    }

    FS_InputActionBinding *BindingA = Profile->ActionBindings.FindByPredicate(
        [ActionA](const FS_InputActionBinding &Binding)
        { return Binding.InputActionName == ActionA; });

    FS_InputActionBinding *BindingB = Profile->ActionBindings.FindByPredicate(
        [ActionB](const FS_InputActionBinding &Binding)
        { return Binding.InputActionName == ActionB; });

    if (!BindingA || !BindingB)
    {
        return false;
    }

    // Swap the key bindings arrays
    TArray<FS_KeyBinding> TempKeys = BindingA->KeyBindings;
    BindingA->KeyBindings = BindingB->KeyBindings;
    BindingB->KeyBindings = TempKeys;
    return true;
}

// ==================== Axis Binding Helper Functions ====================

float UCPP_InputBindingManager::GetAxisSensitivity(APlayerController *PlayerController, const FName &AxisName) const
{
    const FS_PlayerInputData *PlayerData = GetPlayerData(PlayerController);
    if (!PlayerData)
    {
        return 1.0f;
    }

    const FS_InputAxisBinding *AxisBinding = PlayerData->ActiveProfile.AxisBindings.FindByPredicate(
        [AxisName](const FS_InputAxisBinding &Binding)
        { return Binding.InputAxisName == AxisName; });

    if (AxisBinding)
    {
        return AxisBinding->Sensitivity;
    }
    return 1.0f;
}

bool UCPP_InputBindingManager::SetAxisSensitivity(APlayerController *PlayerController, const FName &AxisName, float Sensitivity)
{
    FS_InputProfile *Profile = GetProfileRefForPlayer(PlayerController);
    if (!Profile)
    {
        return false;
    }

    FS_InputAxisBinding *AxisBinding = Profile->AxisBindings.FindByPredicate(
        [AxisName](const FS_InputAxisBinding &Binding)
        { return Binding.InputAxisName == AxisName; });

    if (!AxisBinding)
    {
        // Create new axis binding
        FS_InputAxisBinding NewBinding;
        NewBinding.InputAxisName = AxisName;
        NewBinding.Sensitivity = Sensitivity;
        Profile->AxisBindings.Add(NewBinding);
        return true;
    }

    AxisBinding->Sensitivity = Sensitivity;
    return true;
}

float UCPP_InputBindingManager::GetAxisDeadZone(APlayerController *PlayerController, const FName &AxisName) const
{
    const FS_PlayerInputData *PlayerData = GetPlayerData(PlayerController);
    if (!PlayerData)
    {
        return 0.2f; // Default dead zone
    }

    const FS_InputAxisBinding *AxisBinding = PlayerData->ActiveProfile.AxisBindings.FindByPredicate(
        [AxisName](const FS_InputAxisBinding &Binding)
        { return Binding.InputAxisName == AxisName; });

    if (AxisBinding)
    {
        return AxisBinding->DeadZone;
    }
    return 0.2f;
}

bool UCPP_InputBindingManager::SetAxisDeadZone(APlayerController *PlayerController, const FName &AxisName, float DeadZone)
{
    FS_InputProfile *Profile = GetProfileRefForPlayer(PlayerController);
    if (!Profile)
    {
        return false;
    }

    FS_InputAxisBinding *AxisBinding = Profile->AxisBindings.FindByPredicate(
        [AxisName](const FS_InputAxisBinding &Binding)
        { return Binding.InputAxisName == AxisName; });

    if (!AxisBinding)
    {
        // Create new axis binding
        FS_InputAxisBinding NewBinding;
        NewBinding.InputAxisName = AxisName;
        NewBinding.DeadZone = DeadZone;
        Profile->AxisBindings.Add(NewBinding);
        return true;
    }

    AxisBinding->DeadZone = DeadZone;
    return true;
}

// ==================== Validation & Conflict Detection ====================

bool UCPP_InputBindingManager::ValidateBinding(const FS_InputActionBinding &Binding, FString &OutErrorMessage)
{
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

bool UCPP_InputBindingManager::IsKeyBoundForPlayer(APlayerController *PlayerController, const FKey &Key)
{
    const FS_PlayerInputData *PlayerData = GetPlayerData(PlayerController);
    if (!PlayerData)
    {
        return false;
    }

    for (const FS_InputActionBinding &Binding : PlayerData->ActiveProfile.ActionBindings)
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

void UCPP_InputBindingManager::GetConflictingBindings(APlayerController *PlayerController, TArray<FInputBindingConflict> &OutConflicts)
{
    OutConflicts.Empty();
    if (!PlayerController)
    {
        UE_LOG(LogTemp, Warning, TEXT("P_MEIS: GetConflictingBindings called with null PlayerController"));
        return;
    }
    FS_PlayerInputData *PlayerData = GetPlayerData(PlayerController);
    if (!PlayerData)
    {
        UE_LOG(LogTemp, Warning, TEXT("P_MEIS: No PlayerData found for PlayerController in GetConflictingBindings"));
        return;
    }
    const TArray<FS_InputActionBinding> &ActionBindings = PlayerData->ActiveProfile.ActionBindings;
    TArray<TPair<FName, FName>> RawConflicts;
    UCPP_InputValidator::DetectConflicts(ActionBindings, RawConflicts);
    for (const TPair<FName, FName> &Pair : RawConflicts)
    {
        OutConflicts.Add(FInputBindingConflict(Pair.Key, Pair.Value));
    }
}

// ==================== Import/Export ====================

bool UCPP_InputBindingManager::ExportTemplate(const FName &TemplateName, const FString &FilePath)
{
    FS_InputProfile Profile;
    if (GetTemplate(TemplateName, Profile))
    {
        return UCPP_InputProfileStorage::ExportProfile(Profile, FilePath);
    }
    return false;
}

bool UCPP_InputBindingManager::ImportTemplate(const FString &FilePath, FName &OutTemplateName)
{
    FS_InputProfile ImportedProfile;
    if (UCPP_InputProfileStorage::ImportProfile(FilePath, ImportedProfile))
    {
        OutTemplateName = ImportedProfile.ProfileName;
        return SaveProfileTemplate(ImportedProfile.ProfileName, ImportedProfile);
    }
    return false;
}

// ==================== Legacy Compatibility ====================

bool UCPP_InputBindingManager::LoadProfile(const FName &ProfileName)
{
    // Legacy: Just load as template
    return LoadProfileTemplate(ProfileName);
}

bool UCPP_InputBindingManager::SaveProfile(const FName &ProfileName, const FText &Description)
{
    // Legacy: Create empty template with description
    FS_InputProfile Profile;
    Profile.ProfileName = ProfileName;
    Profile.ProfileDescription = Description;
    Profile.Timestamp = FDateTime::Now();
    return SaveProfileTemplate(ProfileName, Profile);
}

FS_InputProfile UCPP_InputBindingManager::GetCurrentProfile() const
{
    // Legacy: Return first template or empty
    if (ProfileTemplates.Num() > 0)
    {
        for (const auto &Pair : ProfileTemplates)
        {
            return Pair.Value;
        }
    }
    return FS_InputProfile();
}

void UCPP_InputBindingManager::GetAvailableProfiles(TArray<FName> &OutProfiles)
{
    // Legacy: Same as GetAvailableTemplates
    GetAvailableTemplates(OutProfiles);
}

bool UCPP_InputBindingManager::LoadDefaultTemplate()
{
    return LoadProfileTemplate(FName(TEXT("Default")));
}

void UCPP_InputBindingManager::BroadcastBindingChanges(APlayerController *PlayerController)
{
    // Apply the player's profile to their Enhanced Input
    ApplyPlayerProfileToEnhancedInput(PlayerController);
}
