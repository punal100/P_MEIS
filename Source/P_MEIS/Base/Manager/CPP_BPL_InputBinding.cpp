/*
 * @Author: Punal Manalan
 * @Description: Blueprint Function Library Implementation - Multi-Player Support
 *               All Dynamic functions route through Manager
 * @Date: 06/12/2025
 */

#include "Manager/CPP_BPL_InputBinding.h"
#include "Manager/CPP_InputBindingManager.h"
#include "Storage/CPP_InputProfileStorage.h"
#include "Integration/CPP_EnhancedInputIntegration.h"
#include "Integration/CPP_AsyncAction_WaitForInputAction.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

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
        return Manager->CreateProfileTemplate(FName(*ProfileName));
    }
    return false;
}

bool UCPP_BPL_InputBinding::DeleteProfile(const FString &ProfileName)
{
    if (UCPP_InputBindingManager *Manager = GetManager())
    {
        return Manager->DeleteProfileTemplate(FName(*ProfileName));
    }
    return false;
}

bool UCPP_BPL_InputBinding::RenameProfile(const FString &OldName, const FString &NewName)
{
    UE_LOG(LogTemp, Warning, TEXT("P_MEIS: RenameProfile is deprecated. Delete old template and create new one instead."));
    return false;
}

bool UCPP_BPL_InputBinding::DuplicateProfile(const FString &SourceProfile, const FString &NewProfile)
{
    if (UCPP_InputBindingManager *Manager = GetManager())
    {
        // Load source template, save as new template
        FS_InputProfile Profile;
        if (Manager->GetTemplate(FName(*SourceProfile), Profile))
        {
            return Manager->SaveProfileTemplate(FName(*NewProfile), Profile);
        }
    }
    return false;
}

bool UCPP_BPL_InputBinding::ResetToDefaults()
{
    UE_LOG(LogTemp, Warning, TEXT("P_MEIS: ResetToDefaults is deprecated. Use ApplyTemplateToPlayer with 'Default' template instead."));
    return false;
}

void UCPP_BPL_InputBinding::GetAvailableProfiles(TArray<FString> &OutProfiles)
{
    // Forwards to GetAvailableProfileTemplates
    GetAvailableProfileTemplates(OutProfiles);
}

bool UCPP_BPL_InputBinding::SetActionBinding(const FName &ActionName, const FS_InputActionBinding &Binding)
{
    UE_LOG(LogTemp, Warning, TEXT("P_MEIS: SetActionBinding is deprecated. Global profiles removed - use per-player profile operations instead."));
    return false;
}

bool UCPP_BPL_InputBinding::RemoveActionBinding(const FName &ActionName)
{
    UE_LOG(LogTemp, Warning, TEXT("P_MEIS: RemoveActionBinding is deprecated. Global profiles removed - use per-player profile operations instead."));
    return false;
}

bool UCPP_BPL_InputBinding::GetActionBinding(const FName &ActionName, FS_InputActionBinding &OutBinding)
{
    UE_LOG(LogTemp, Warning, TEXT("P_MEIS: GetActionBinding is deprecated. Global profiles removed - use per-player profile operations instead."));
    return false;
}

void UCPP_BPL_InputBinding::GetActionBindings(TArray<FS_InputActionBinding> &OutBindings)
{
    UE_LOG(LogTemp, Warning, TEXT("P_MEIS: GetActionBindings is deprecated. Global profiles removed - use per-player profile operations instead."));
    OutBindings.Empty();
}

bool UCPP_BPL_InputBinding::ClearActionBindings()
{
    UE_LOG(LogTemp, Warning, TEXT("P_MEIS: ClearActionBindings is deprecated. Global profiles removed - use per-player profile operations instead."));
    return false;
}

bool UCPP_BPL_InputBinding::SetAxisBinding(const FName &AxisName, const FS_InputAxisBinding &Binding)
{
    UE_LOG(LogTemp, Warning, TEXT("P_MEIS: SetAxisBinding is deprecated. Global profiles removed - use per-player profile operations instead."));
    return false;
}

bool UCPP_BPL_InputBinding::RemoveAxisBinding(const FName &AxisName)
{
    UE_LOG(LogTemp, Warning, TEXT("P_MEIS: RemoveAxisBinding is deprecated. Global profiles removed - use per-player profile operations instead."));
    return false;
}

bool UCPP_BPL_InputBinding::GetAxisBinding(const FName &AxisName, FS_InputAxisBinding &OutBinding)
{
    UE_LOG(LogTemp, Warning, TEXT("P_MEIS: GetAxisBinding is deprecated. Global profiles removed - use per-player profile operations instead."));
    return false;
}

void UCPP_BPL_InputBinding::GetAxisBindings(TArray<FS_InputAxisBinding> &OutBindings)
{
    UE_LOG(LogTemp, Warning, TEXT("P_MEIS: GetAxisBindings is deprecated. Global profiles removed - use per-player profile operations instead."));
    OutBindings.Empty();
}

bool UCPP_BPL_InputBinding::ClearAxisBindings()
{
    UE_LOG(LogTemp, Warning, TEXT("P_MEIS: ClearAxisBindings is deprecated. Global profiles removed - use per-player profile operations instead."));
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

void UCPP_BPL_InputBinding::GetConflictingBindings(APlayerController *PlayerController, TArray<FInputBindingConflict> &OutConflicts)
{
    if (UCPP_InputBindingManager *Manager = GetManager())
    {
        Manager->GetConflictingBindings(PlayerController, OutConflicts);
    }
}

bool UCPP_BPL_InputBinding::IsKeyBound(const FKey &Key)
{
    UE_LOG(LogTemp, Warning, TEXT("P_MEIS: IsKeyBound is deprecated. Use IsKeyBoundForPlayer(PC, Key) instead - key binding is now per-player."));
    return false;
}

bool UCPP_BPL_InputBinding::ExportProfile(const FString &ProfileName, const FString &ExportPath)
{
    if (UCPP_InputBindingManager *Manager = GetManager())
    {
        return Manager->ExportTemplate(FName(*ProfileName), ExportPath);
    }
    return false;
}

bool UCPP_BPL_InputBinding::ImportProfile(const FString &ImportPath, FString &OutProfileName)
{
    if (UCPP_InputBindingManager *Manager = GetManager())
    {
        FName TemplateName;
        bool bSuccess = Manager->ImportTemplate(ImportPath, TemplateName);
        if (bSuccess)
        {
            OutProfileName = TemplateName.ToString();
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

// ==================== Key Utility Functions ====================

FKey UCPP_BPL_InputBinding::StringToKey(const FString &KeyString)
{
    return UCPP_EnhancedInputIntegration::StringToKey(KeyString);
}

FString UCPP_BPL_InputBinding::KeyToString(const FKey &Key)
{
    return UCPP_EnhancedInputIntegration::KeyToString(Key);
}

bool UCPP_BPL_InputBinding::IsValidKeyString(const FString &KeyString)
{
    return UCPP_EnhancedInputIntegration::IsValidKeyString(KeyString);
}

void UCPP_BPL_InputBinding::GetAllKeyNames(TArray<FString> &OutKeyNames)
{
    UCPP_EnhancedInputIntegration::GetAllKeyNames(OutKeyNames);
}

FS_KeyBinding UCPP_BPL_InputBinding::MakeKeyBinding(const FString &KeyString, bool bShift, bool bCtrl, bool bAlt, bool bCmd)
{
    FS_KeyBinding Binding;
    Binding.Key = StringToKey(KeyString);
    Binding.bShift = bShift;
    Binding.bCtrl = bCtrl;
    Binding.bAlt = bAlt;
    Binding.bCmd = bCmd;
    Binding.Value = 1.0f;
    return Binding;
}

FS_AxisKeyBinding UCPP_BPL_InputBinding::MakeAxisKeyBinding(const FString &KeyString, float Scale)
{
    FS_AxisKeyBinding Binding;
    Binding.Key = StringToKey(KeyString);
    Binding.Scale = Scale;
    return Binding;
}

// ==================== Player Management (Multi-Player Support) ====================

UCPP_EnhancedInputIntegration *UCPP_BPL_InputBinding::InitializeEnhancedInputIntegration(UObject *WorldContextObject, APlayerController *PlayerController)
{
    if (!PlayerController)
    {
        UE_LOG(LogTemp, Warning, TEXT("P_MEIS: PlayerController is null"));
        return nullptr;
    }

    UCPP_InputBindingManager *Manager = GetManager();
    if (!Manager)
    {
        UE_LOG(LogTemp, Error, TEXT("P_MEIS: Input Binding Manager not available"));
        return nullptr;
    }

    // Register player through Manager (this creates and returns Integration)
    UCPP_EnhancedInputIntegration *Integration = Manager->RegisterPlayer(PlayerController);

    if (Integration)
    {
        UE_LOG(LogTemp, Log, TEXT("P_MEIS: Initialized Enhanced Input Integration for player %s"), *PlayerController->GetName());
    }

    return Integration;
}

UCPP_EnhancedInputIntegration *UCPP_BPL_InputBinding::GetIntegrationForPlayer(APlayerController *PlayerController)
{
    if (!PlayerController)
    {
        return nullptr;
    }

    UCPP_InputBindingManager *Manager = GetManager();
    if (!Manager)
    {
        return nullptr;
    }

    return Manager->GetIntegrationForPlayer(PlayerController);
}

void UCPP_BPL_InputBinding::UnregisterPlayer(APlayerController *PlayerController)
{
    if (!PlayerController)
    {
        return;
    }

    UCPP_InputBindingManager *Manager = GetManager();
    if (Manager)
    {
        Manager->UnregisterPlayer(PlayerController);
    }
}

bool UCPP_BPL_InputBinding::IsPlayerRegistered(APlayerController *PlayerController)
{
    if (!PlayerController)
    {
        return false;
    }

    UCPP_InputBindingManager *Manager = GetManager();
    if (!Manager)
    {
        return false;
    }

    return Manager->HasPlayerRegistered(PlayerController);
}

// ==================== UI / Virtual Device Injection ====================

void UCPP_BPL_InputBinding::InjectActionStarted(APlayerController *PlayerController, const FName &ActionName)
{
    if (!PlayerController || !PlayerController->IsLocalController())
    {
        return;
    }

    UCPP_EnhancedInputIntegration *Integration = GetIntegrationForPlayer(PlayerController);
    if (Integration)
    {
        Integration->InjectActionStarted(ActionName);
    }
}

void UCPP_BPL_InputBinding::InjectActionTriggered(APlayerController *PlayerController, const FName &ActionName)
{
    if (!PlayerController || !PlayerController->IsLocalController())
    {
        return;
    }

    UCPP_EnhancedInputIntegration *Integration = GetIntegrationForPlayer(PlayerController);
    if (Integration)
    {
        Integration->InjectActionTriggered(ActionName);
    }
}

void UCPP_BPL_InputBinding::InjectActionCompleted(APlayerController *PlayerController, const FName &ActionName)
{
    if (!PlayerController || !PlayerController->IsLocalController())
    {
        return;
    }

    UCPP_EnhancedInputIntegration *Integration = GetIntegrationForPlayer(PlayerController);
    if (Integration)
    {
        Integration->InjectActionCompleted(ActionName);
    }
}

void UCPP_BPL_InputBinding::InjectAxis2D(APlayerController *PlayerController, const FName &AxisName, const FVector2D &Value)
{
    if (!PlayerController || !PlayerController->IsLocalController())
    {
        return;
    }

    UCPP_EnhancedInputIntegration *Integration = GetIntegrationForPlayer(PlayerController);
    if (Integration)
    {
        Integration->InjectAxis2D(AxisName, Value);
    }
}

// ==================== Dynamic Input Action Creation (Multi-Player) ====================

UInputAction *UCPP_BPL_InputBinding::CreateDynamicInputAction(APlayerController *PlayerController, const FName &ActionName, bool bIsAxis)
{
    UCPP_EnhancedInputIntegration *Integration = GetIntegrationForPlayer(PlayerController);
    if (!Integration)
    {
        UE_LOG(LogTemp, Warning, TEXT("P_MEIS: No Integration for player. Call InitializeEnhancedInputIntegration first."));
        return nullptr;
    }

    EInputActionValueType ValueType = bIsAxis ? EInputActionValueType::Axis1D : EInputActionValueType::Boolean;
    return Integration->CreateInputAction(ActionName, ValueType);
}

UInputAction *UCPP_BPL_InputBinding::GetDynamicInputAction(APlayerController *PlayerController, const FName &ActionName)
{
    UCPP_EnhancedInputIntegration *Integration = GetIntegrationForPlayer(PlayerController);
    if (!Integration)
    {
        return nullptr;
    }

    return Integration->GetInputAction(ActionName);
}

bool UCPP_BPL_InputBinding::MapKeyToDynamicAction(APlayerController *PlayerController, const FName &ActionName, const FKey &Key)
{
    UCPP_EnhancedInputIntegration *Integration = GetIntegrationForPlayer(PlayerController);
    if (!Integration)
    {
        UE_LOG(LogTemp, Warning, TEXT("P_MEIS: No Integration for player. Call InitializeEnhancedInputIntegration first."));
        return false;
    }

    return Integration->MapKeyToAction(ActionName, Key);
}

bool UCPP_BPL_InputBinding::MapKeyBindingToDynamicAction(APlayerController *PlayerController, const FName &ActionName, const FS_KeyBinding &KeyBinding)
{
    UCPP_EnhancedInputIntegration *Integration = GetIntegrationForPlayer(PlayerController);
    if (!Integration)
    {
        UE_LOG(LogTemp, Warning, TEXT("P_MEIS: No Integration for player. Call InitializeEnhancedInputIntegration first."));
        return false;
    }

    return Integration->MapKeyBindingToAction(ActionName, KeyBinding);
}

bool UCPP_BPL_InputBinding::MapKeyStringWithModifiers(APlayerController *PlayerController, const FName &ActionName,
                                                      const FString &KeyString, bool bShift, bool bCtrl,
                                                      bool bAlt, bool bCmd)
{
    UCPP_EnhancedInputIntegration *Integration = GetIntegrationForPlayer(PlayerController);
    if (!Integration)
    {
        UE_LOG(LogTemp, Warning, TEXT("P_MEIS: No Integration for player. Call InitializeEnhancedInputIntegration first."));
        return false;
    }

    return Integration->MapKeyStringWithModifiers(ActionName, KeyString, bShift, bCtrl, bAlt, bCmd);
}

bool UCPP_BPL_InputBinding::MapKeyStringToDynamicAction(APlayerController *PlayerController, const FName &ActionName, const FString &KeyString)
{
    FKey Key = StringToKey(KeyString);
    return MapKeyToDynamicAction(PlayerController, ActionName, Key);
}

bool UCPP_BPL_InputBinding::ApplyProfileToPlayer(APlayerController *PlayerController)
{
    UCPP_InputBindingManager *Manager = GetManager();
    if (!Manager)
    {
        UE_LOG(LogTemp, Warning, TEXT("P_MEIS: Input Binding Manager not available"));
        return false;
    }

    return Manager->ApplyPlayerProfileToEnhancedInput(PlayerController);
}

bool UCPP_BPL_InputBinding::ApplyProfileToAllPlayers()
{
    UCPP_InputBindingManager *Manager = GetManager();
    if (!Manager)
    {
        UE_LOG(LogTemp, Warning, TEXT("P_MEIS: Input Binding Manager not available"));
        return false;
    }

    // Apply to all registered players
    TArray<APlayerController *> Players;
    Manager->GetRegisteredPlayers(Players);

    bool bAllSuccess = true;
    for (APlayerController *PC : Players)
    {
        if (!Manager->ApplyPlayerProfileToEnhancedInput(PC))
        {
            bAllSuccess = false;
        }
    }
    return bAllSuccess;
}

// ==================== Per-Player Profile Operations ====================

bool UCPP_BPL_InputBinding::LoadProfileForPlayer(APlayerController *PlayerController, const FString &TemplateName)
{
    UCPP_InputBindingManager *Manager = GetManager();
    if (!Manager)
    {
        UE_LOG(LogTemp, Warning, TEXT("P_MEIS: Input Binding Manager not available"));
        return false;
    }

    return Manager->ApplyTemplateToPlayer(PlayerController, FName(*TemplateName));
}

bool UCPP_BPL_InputBinding::SaveProfileForPlayer(APlayerController *PlayerController, const FString &TemplateName)
{
    UCPP_InputBindingManager *Manager = GetManager();
    if (!Manager)
    {
        UE_LOG(LogTemp, Warning, TEXT("P_MEIS: Input Binding Manager not available"));
        return false;
    }

    return Manager->SavePlayerProfileAsTemplate(PlayerController, FName(*TemplateName));
}

FS_InputProfile UCPP_BPL_InputBinding::GetProfileForPlayer(APlayerController *PlayerController)
{
    UCPP_InputBindingManager *Manager = GetManager();
    if (!Manager)
    {
        return FS_InputProfile();
    }

    return Manager->GetProfileForPlayer(PlayerController);
}

bool UCPP_BPL_InputBinding::ApplyTemplateToPlayer(APlayerController *PlayerController, const FString &TemplateName)
{
    UCPP_InputBindingManager *Manager = GetManager();
    if (!Manager)
    {
        UE_LOG(LogTemp, Warning, TEXT("P_MEIS: Input Binding Manager not available"));
        return false;
    }

    return Manager->ApplyTemplateToPlayer(PlayerController, FName(*TemplateName));
}

// ==================== Profile Template Management (Global Library) ====================

void UCPP_BPL_InputBinding::GetAvailableProfileTemplates(TArray<FString> &OutTemplates)
{
    OutTemplates.Empty();
    TArray<FName> TemplateNames;

    UCPP_InputBindingManager *Manager = GetManager();
    if (Manager)
    {
        Manager->GetAvailableTemplates(TemplateNames);
        for (const FName &Name : TemplateNames)
        {
            OutTemplates.Add(Name.ToString());
        }
    }
}

bool UCPP_BPL_InputBinding::CreateProfileTemplate(const FString &TemplateName)
{
    UCPP_InputBindingManager *Manager = GetManager();
    if (!Manager)
    {
        UE_LOG(LogTemp, Warning, TEXT("P_MEIS: Input Binding Manager not available"));
        return false;
    }

    return Manager->CreateProfileTemplate(FName(*TemplateName));
}

bool UCPP_BPL_InputBinding::DeleteProfileTemplate(const FString &TemplateName)
{
    UCPP_InputBindingManager *Manager = GetManager();
    if (!Manager)
    {
        UE_LOG(LogTemp, Warning, TEXT("P_MEIS: Input Binding Manager not available"));
        return false;
    }

    return Manager->DeleteProfileTemplate(FName(*TemplateName));
}

// ==================== Template Helpers ====================

bool UCPP_BPL_InputBinding::DoesTemplateExist(const FString &TemplateName)
{
    UCPP_InputBindingManager *Manager = GetManager();
    if (!Manager)
    {
        return false;
    }
    return Manager->DoesTemplateExist(FName(*TemplateName));
}

int32 UCPP_BPL_InputBinding::GetTemplateCount()
{
    UCPP_InputBindingManager *Manager = GetManager();
    if (!Manager)
    {
        return 0;
    }
    return Manager->GetTemplateCount();
}

// ==================== Per-Player Action Binding Helpers ====================

bool UCPP_BPL_InputBinding::DoesActionExist(APlayerController *PlayerController, const FName &ActionName)
{
    UCPP_InputBindingManager *Manager = GetManager();
    if (!Manager)
    {
        return false;
    }
    return Manager->DoesActionExist(PlayerController, ActionName);
}

TArray<FKey> UCPP_BPL_InputBinding::GetKeysForAction(APlayerController *PlayerController, const FName &ActionName)
{
    TArray<FKey> Keys;
    UCPP_InputBindingManager *Manager = GetManager();
    if (Manager)
    {
        Manager->GetKeysForAction(PlayerController, ActionName, Keys);
    }
    return Keys;
}

FKey UCPP_BPL_InputBinding::GetPrimaryKeyForAction(APlayerController *PlayerController, const FName &ActionName)
{
    UCPP_InputBindingManager *Manager = GetManager();
    if (!Manager)
    {
        return EKeys::Invalid;
    }
    return Manager->GetPrimaryKeyForAction(PlayerController, ActionName);
}

TArray<FName> UCPP_BPL_InputBinding::GetActionsForKey(APlayerController *PlayerController, const FKey &Key)
{
    TArray<FName> Actions;
    UCPP_InputBindingManager *Manager = GetManager();
    if (Manager)
    {
        Manager->GetActionsForKey(PlayerController, Key, Actions);
    }
    return Actions;
}

bool UCPP_BPL_InputBinding::SetPrimaryKeyForAction(APlayerController *PlayerController, const FName &ActionName, const FKey &Key)
{
    UCPP_InputBindingManager *Manager = GetManager();
    if (!Manager)
    {
        return false;
    }
    return Manager->SetPrimaryKeyForAction(PlayerController, ActionName, Key);
}

bool UCPP_BPL_InputBinding::AddKeyToAction(APlayerController *PlayerController, const FName &ActionName, const FKey &Key)
{
    UCPP_InputBindingManager *Manager = GetManager();
    if (!Manager)
    {
        return false;
    }
    return Manager->AddKeyToAction(PlayerController, ActionName, Key);
}

bool UCPP_BPL_InputBinding::RemoveKeyFromAction(APlayerController *PlayerController, const FName &ActionName, const FKey &Key)
{
    UCPP_InputBindingManager *Manager = GetManager();
    if (!Manager)
    {
        return false;
    }
    return Manager->RemoveKeyFromAction(PlayerController, ActionName, Key);
}

bool UCPP_BPL_InputBinding::ClearActionKeys(APlayerController *PlayerController, const FName &ActionName)
{
    UCPP_InputBindingManager *Manager = GetManager();
    if (!Manager)
    {
        return false;
    }
    return Manager->ClearActionKeys(PlayerController, ActionName);
}

bool UCPP_BPL_InputBinding::IsKeyBoundForPlayer(APlayerController *PlayerController, const FKey &Key)
{
    UCPP_InputBindingManager *Manager = GetManager();
    if (!Manager)
    {
        return false;
    }
    return Manager->IsKeyBoundForPlayer(PlayerController, Key);
}

// ==================== Per-Player Axis Binding Helpers ====================

float UCPP_BPL_InputBinding::GetAxisSensitivity(APlayerController *PlayerController, const FName &AxisName)
{
    UCPP_InputBindingManager *Manager = GetManager();
    if (!Manager)
    {
        return 1.0f;
    }
    return Manager->GetAxisSensitivity(PlayerController, AxisName);
}

bool UCPP_BPL_InputBinding::SetAxisSensitivity(APlayerController *PlayerController, const FName &AxisName, float Sensitivity)
{
    UCPP_InputBindingManager *Manager = GetManager();
    if (!Manager)
    {
        return false;
    }
    return Manager->SetAxisSensitivity(PlayerController, AxisName, Sensitivity);
}

float UCPP_BPL_InputBinding::GetAxisDeadZone(APlayerController *PlayerController, const FName &AxisName)
{
    UCPP_InputBindingManager *Manager = GetManager();
    if (!Manager)
    {
        return 0.2f;
    }
    return Manager->GetAxisDeadZone(PlayerController, AxisName);
}

bool UCPP_BPL_InputBinding::SetAxisDeadZone(APlayerController *PlayerController, const FName &AxisName, float DeadZone)
{
    UCPP_InputBindingManager *Manager = GetManager();
    if (!Manager)
    {
        return false;
    }
    return Manager->SetAxisDeadZone(PlayerController, AxisName, DeadZone);
}

// ==================== Dynamic Input Action Helpers ====================

bool UCPP_BPL_InputBinding::DoesInputActionExist(APlayerController *PlayerController, const FName &ActionName)
{
    UCPP_EnhancedInputIntegration *Integration = GetIntegrationForPlayer(PlayerController);
    if (!Integration)
    {
        return false;
    }
    return Integration->HasInputAction(ActionName);
}

UInputMappingContext *UCPP_BPL_InputBinding::GetInputMappingContext(APlayerController *PlayerController)
{
    UCPP_EnhancedInputIntegration *Integration = GetIntegrationForPlayer(PlayerController);
    if (!Integration)
    {
        return nullptr;
    }
    return Integration->GetMappingContext();
}

// ==================== Player Info Helpers ====================

int32 UCPP_BPL_InputBinding::GetRegisteredPlayerCount()
{
    UCPP_InputBindingManager *Manager = GetManager();
    if (!Manager)
    {
        return 0;
    }
    return Manager->GetRegisteredPlayerCount();
}

FString UCPP_BPL_InputBinding::GetPlayerLoadedTemplateName(APlayerController *PlayerController)
{
    UCPP_InputBindingManager *Manager = GetManager();
    if (!Manager)
    {
        return FString();
    }
    FName TemplateName = Manager->GetPlayerLoadedTemplateName(PlayerController);
    return TemplateName.IsNone() ? FString() : TemplateName.ToString();
}

bool UCPP_BPL_InputBinding::HasPlayerModifiedProfile(APlayerController *PlayerController)
{
    UCPP_InputBindingManager *Manager = GetManager();
    if (!Manager)
    {
        return false;
    }
    return Manager->HasPlayerModifiedProfile(PlayerController);
}

// ==================== Key Utility Functions (Extended) ====================

TArray<FKey> UCPP_BPL_InputBinding::GetAllKeyboardKeys()
{
    TArray<FKey> Keys;

    // Add all standard keyboard keys
    Keys.Add(EKeys::A);
    Keys.Add(EKeys::B);
    Keys.Add(EKeys::C);
    Keys.Add(EKeys::D);
    Keys.Add(EKeys::E);
    Keys.Add(EKeys::F);
    Keys.Add(EKeys::G);
    Keys.Add(EKeys::H);
    Keys.Add(EKeys::I);
    Keys.Add(EKeys::J);
    Keys.Add(EKeys::K);
    Keys.Add(EKeys::L);
    Keys.Add(EKeys::M);
    Keys.Add(EKeys::N);
    Keys.Add(EKeys::O);
    Keys.Add(EKeys::P);
    Keys.Add(EKeys::Q);
    Keys.Add(EKeys::R);
    Keys.Add(EKeys::S);
    Keys.Add(EKeys::T);
    Keys.Add(EKeys::U);
    Keys.Add(EKeys::V);
    Keys.Add(EKeys::W);
    Keys.Add(EKeys::X);
    Keys.Add(EKeys::Y);
    Keys.Add(EKeys::Z);

    // Numbers
    Keys.Add(EKeys::Zero);
    Keys.Add(EKeys::One);
    Keys.Add(EKeys::Two);
    Keys.Add(EKeys::Three);
    Keys.Add(EKeys::Four);
    Keys.Add(EKeys::Five);
    Keys.Add(EKeys::Six);
    Keys.Add(EKeys::Seven);
    Keys.Add(EKeys::Eight);
    Keys.Add(EKeys::Nine);

    // Function keys
    Keys.Add(EKeys::F1);
    Keys.Add(EKeys::F2);
    Keys.Add(EKeys::F3);
    Keys.Add(EKeys::F4);
    Keys.Add(EKeys::F5);
    Keys.Add(EKeys::F6);
    Keys.Add(EKeys::F7);
    Keys.Add(EKeys::F8);
    Keys.Add(EKeys::F9);
    Keys.Add(EKeys::F10);
    Keys.Add(EKeys::F11);
    Keys.Add(EKeys::F12);

    // Numpad
    Keys.Add(EKeys::NumPadZero);
    Keys.Add(EKeys::NumPadOne);
    Keys.Add(EKeys::NumPadTwo);
    Keys.Add(EKeys::NumPadThree);
    Keys.Add(EKeys::NumPadFour);
    Keys.Add(EKeys::NumPadFive);
    Keys.Add(EKeys::NumPadSix);
    Keys.Add(EKeys::NumPadSeven);
    Keys.Add(EKeys::NumPadEight);
    Keys.Add(EKeys::NumPadNine);

    // Special keys
    Keys.Add(EKeys::SpaceBar);
    Keys.Add(EKeys::Enter);
    Keys.Add(EKeys::BackSpace);
    Keys.Add(EKeys::Tab);
    Keys.Add(EKeys::Escape);
    Keys.Add(EKeys::CapsLock);
    Keys.Add(EKeys::LeftShift);
    Keys.Add(EKeys::RightShift);
    Keys.Add(EKeys::LeftControl);
    Keys.Add(EKeys::RightControl);
    Keys.Add(EKeys::LeftAlt);
    Keys.Add(EKeys::RightAlt);
    Keys.Add(EKeys::LeftCommand);
    Keys.Add(EKeys::RightCommand);

    // Arrow keys
    Keys.Add(EKeys::Up);
    Keys.Add(EKeys::Down);
    Keys.Add(EKeys::Left);
    Keys.Add(EKeys::Right);

    // Navigation keys
    Keys.Add(EKeys::Insert);
    Keys.Add(EKeys::Delete);
    Keys.Add(EKeys::Home);
    Keys.Add(EKeys::End);
    Keys.Add(EKeys::PageUp);
    Keys.Add(EKeys::PageDown);

    // Punctuation
    Keys.Add(EKeys::Semicolon);
    Keys.Add(EKeys::Comma);
    Keys.Add(EKeys::Period);
    Keys.Add(EKeys::Slash);
    Keys.Add(EKeys::Tilde);
    Keys.Add(EKeys::LeftBracket);
    Keys.Add(EKeys::RightBracket);
    Keys.Add(EKeys::Backslash);
    Keys.Add(EKeys::Apostrophe);
    Keys.Add(EKeys::Hyphen);
    Keys.Add(EKeys::Equals);

    return Keys;
}

TArray<FKey> UCPP_BPL_InputBinding::GetAllGamepadKeys()
{
    TArray<FKey> Keys;

    // Face buttons
    Keys.Add(EKeys::Gamepad_FaceButton_Bottom);
    Keys.Add(EKeys::Gamepad_FaceButton_Right);
    Keys.Add(EKeys::Gamepad_FaceButton_Left);
    Keys.Add(EKeys::Gamepad_FaceButton_Top);

    // Shoulder buttons
    Keys.Add(EKeys::Gamepad_LeftShoulder);
    Keys.Add(EKeys::Gamepad_RightShoulder);

    // Triggers
    Keys.Add(EKeys::Gamepad_LeftTrigger);
    Keys.Add(EKeys::Gamepad_RightTrigger);
    Keys.Add(EKeys::Gamepad_LeftTriggerAxis);
    Keys.Add(EKeys::Gamepad_RightTriggerAxis);

    // D-Pad
    Keys.Add(EKeys::Gamepad_DPad_Up);
    Keys.Add(EKeys::Gamepad_DPad_Down);
    Keys.Add(EKeys::Gamepad_DPad_Left);
    Keys.Add(EKeys::Gamepad_DPad_Right);

    // Thumbsticks
    Keys.Add(EKeys::Gamepad_LeftThumbstick);
    Keys.Add(EKeys::Gamepad_RightThumbstick);
    Keys.Add(EKeys::Gamepad_LeftStick_Up);
    Keys.Add(EKeys::Gamepad_LeftStick_Down);
    Keys.Add(EKeys::Gamepad_LeftStick_Left);
    Keys.Add(EKeys::Gamepad_LeftStick_Right);
    Keys.Add(EKeys::Gamepad_RightStick_Up);
    Keys.Add(EKeys::Gamepad_RightStick_Down);
    Keys.Add(EKeys::Gamepad_RightStick_Left);
    Keys.Add(EKeys::Gamepad_RightStick_Right);

    // Axes
    Keys.Add(EKeys::Gamepad_LeftX);
    Keys.Add(EKeys::Gamepad_LeftY);
    Keys.Add(EKeys::Gamepad_RightX);
    Keys.Add(EKeys::Gamepad_RightY);

    // Special
    Keys.Add(EKeys::Gamepad_Special_Left);
    Keys.Add(EKeys::Gamepad_Special_Right);

    return Keys;
}

TArray<FKey> UCPP_BPL_InputBinding::GetAllMouseKeys()
{
    TArray<FKey> Keys;

    // Mouse buttons
    Keys.Add(EKeys::LeftMouseButton);
    Keys.Add(EKeys::RightMouseButton);
    Keys.Add(EKeys::MiddleMouseButton);
    Keys.Add(EKeys::ThumbMouseButton);
    Keys.Add(EKeys::ThumbMouseButton2);

    // Mouse axes
    Keys.Add(EKeys::MouseX);
    Keys.Add(EKeys::MouseY);
    Keys.Add(EKeys::MouseScrollUp);
    Keys.Add(EKeys::MouseScrollDown);
    Keys.Add(EKeys::MouseWheelAxis);

    return Keys;
}

FString UCPP_BPL_InputBinding::KeyToDisplayString(const FKey &Key)
{
    // Get the display name from the key
    FText DisplayText = Key.GetDisplayName();
    return DisplayText.ToString();
}

void UCPP_BPL_InputBinding::StopWaitingForInputAction(UAsyncAction_WaitForInputAction *AsyncAction)
{
    if (AsyncAction && AsyncAction->IsActive())
    {
        AsyncAction->Cancel();
    }
}

// ==================== Section 0.9: Dynamic Modifiers & Triggers ====================

bool UCPP_BPL_InputBinding::SetActionDeadZone(APlayerController *PlayerController, const FName &ActionName, float LowerThreshold, float UpperThreshold)
{
    UCPP_EnhancedInputIntegration *Integration = GetIntegrationForPlayer(PlayerController);
    if (!Integration)
    {
        UE_LOG(LogTemp, Warning, TEXT("P_MEIS BPL: SetActionDeadZone - No integration found for player"));
        return false;
    }
    return Integration->SetActionDeadZone(ActionName, LowerThreshold, UpperThreshold);
}

bool UCPP_BPL_InputBinding::SetActionSensitivity(APlayerController *PlayerController, const FName &ActionName, float Sensitivity)
{
    UCPP_EnhancedInputIntegration *Integration = GetIntegrationForPlayer(PlayerController);
    if (!Integration)
    {
        UE_LOG(LogTemp, Warning, TEXT("P_MEIS BPL: SetActionSensitivity - No integration found for player"));
        return false;
    }
    return Integration->SetActionSensitivity(ActionName, Sensitivity);
}

bool UCPP_BPL_InputBinding::SetActionSensitivityPerAxis(APlayerController *PlayerController, const FName &ActionName, FVector Sensitivity)
{
    UCPP_EnhancedInputIntegration *Integration = GetIntegrationForPlayer(PlayerController);
    if (!Integration)
    {
        UE_LOG(LogTemp, Warning, TEXT("P_MEIS BPL: SetActionSensitivityPerAxis - No integration found for player"));
        return false;
    }
    return Integration->SetActionSensitivityPerAxis(ActionName, Sensitivity);
}

bool UCPP_BPL_InputBinding::SetActionInvertY(APlayerController *PlayerController, const FName &ActionName, bool bInvert)
{
    UCPP_EnhancedInputIntegration *Integration = GetIntegrationForPlayer(PlayerController);
    if (!Integration)
    {
        UE_LOG(LogTemp, Warning, TEXT("P_MEIS BPL: SetActionInvertY - No integration found for player"));
        return false;
    }
    return Integration->SetActionInvertY(ActionName, bInvert);
}

bool UCPP_BPL_InputBinding::SetKeyHoldTrigger(APlayerController *PlayerController, const FName &ActionName, const FKey &Key, float HoldTime)
{
    UCPP_EnhancedInputIntegration *Integration = GetIntegrationForPlayer(PlayerController);
    if (!Integration)
    {
        UE_LOG(LogTemp, Warning, TEXT("P_MEIS BPL: SetKeyHoldTrigger - No integration found for player"));
        return false;
    }
    return Integration->SetKeyHoldTrigger(ActionName, Key, HoldTime);
}

bool UCPP_BPL_InputBinding::SetKeyTapTrigger(APlayerController *PlayerController, const FName &ActionName, const FKey &Key, float MaxTapTime)
{
    UCPP_EnhancedInputIntegration *Integration = GetIntegrationForPlayer(PlayerController);
    if (!Integration)
    {
        UE_LOG(LogTemp, Warning, TEXT("P_MEIS BPL: SetKeyTapTrigger - No integration found for player"));
        return false;
    }
    return Integration->SetKeyTapTrigger(ActionName, Key, MaxTapTime);
}

bool UCPP_BPL_InputBinding::ClearKeyTriggers(APlayerController *PlayerController, const FName &ActionName, const FKey &Key)
{
    UCPP_EnhancedInputIntegration *Integration = GetIntegrationForPlayer(PlayerController);
    if (!Integration)
    {
        UE_LOG(LogTemp, Warning, TEXT("P_MEIS BPL: ClearKeyTriggers - No integration found for player"));
        return false;
    }
    return Integration->ClearKeyMappingTriggers(ActionName, Key);
}

bool UCPP_BPL_InputBinding::ClearActionModifiers(APlayerController *PlayerController, const FName &ActionName)
{
    UCPP_EnhancedInputIntegration *Integration = GetIntegrationForPlayer(PlayerController);
    if (!Integration)
    {
        UE_LOG(LogTemp, Warning, TEXT("P_MEIS BPL: ClearActionModifiers - No integration found for player"));
        return false;
    }
    return Integration->ClearActionModifiers(ActionName);
}
