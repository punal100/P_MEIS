/*
 * @Author: Punal Manalan
 * @Description: Input Profile Storage Implementation
 * @Date: 06/12/2025
 */

#include "Storage/CPP_InputProfileStorage.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Json.h"
#include "JsonUtilities.h"

bool UCPP_InputProfileStorage::SaveProfile(const FS_InputProfile &Profile)
{
    FString JsonString = SerializeProfileToJson(Profile);
    FString FilePath = GetProfileFilePath(Profile.ProfileName);

    if (FFileHelper::SaveStringToFile(JsonString, *FilePath))
    {
        UE_LOG(LogTemp, Log, TEXT("P_MEIS: Profile saved to %s"), *FilePath);
        return true;
    }

    UE_LOG(LogTemp, Warning, TEXT("P_MEIS: Failed to save profile to %s"), *FilePath);
    return false;
}

bool UCPP_InputProfileStorage::LoadProfile(const FName &ProfileName, FS_InputProfile &OutProfile)
{
    FString FilePath = GetProfileFilePath(ProfileName);

    if (!FPaths::FileExists(*FilePath))
    {
        if (IsRunningCommandlet())
        {
            UE_LOG(LogTemp, Log, TEXT("P_MEIS: Profile file not found: %s"), *FilePath);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("P_MEIS: Profile file not found: %s"), *FilePath);
        }
        return false;
    }

    FString JsonString;
    if (!FFileHelper::LoadFileToString(JsonString, *FilePath))
    {
        UE_LOG(LogTemp, Warning, TEXT("P_MEIS: Failed to load profile file: %s"), *FilePath);
        return false;
    }

    return DeserializeProfileFromJson(JsonString, OutProfile);
}

bool UCPP_InputProfileStorage::DeleteProfile(const FName &ProfileName)
{
    FString FilePath = GetProfileFilePath(ProfileName);

    if (IFileManager::Get().Delete(*FilePath))
    {
        UE_LOG(LogTemp, Log, TEXT("P_MEIS: Profile deleted: %s"), *FilePath);
        return true;
    }

    UE_LOG(LogTemp, Warning, TEXT("P_MEIS: Failed to delete profile: %s"), *FilePath);
    return false;
}

bool UCPP_InputProfileStorage::ProfileExists(const FName &ProfileName)
{
    return FPaths::FileExists(*GetProfileFilePath(ProfileName));
}

void UCPP_InputProfileStorage::GetAvailableProfiles(TArray<FName> &OutProfiles)
{
    FString ProfileDir = GetProfileDirectory();
    IFileManager &FileMgr = IFileManager::Get();

    // Find all .json files in the profiles directory
    TArray<FString> FoundFiles;
    FileMgr.FindFiles(FoundFiles, *ProfileDir, TEXT("json"));

    for (const FString &FilePath : FoundFiles)
    {
        FString FileName = FPaths::GetBaseFilename(FilePath);
        OutProfiles.Add(FName(*FileName));
    }
}

bool UCPP_InputProfileStorage::ExportProfile(const FS_InputProfile &Profile, const FString &FilePath)
{
    FString JsonString = SerializeProfileToJson(Profile);

    if (FFileHelper::SaveStringToFile(JsonString, *FilePath))
    {
        UE_LOG(LogTemp, Log, TEXT("P_MEIS: Profile exported to %s"), *FilePath);
        return true;
    }

    return false;
}

bool UCPP_InputProfileStorage::ImportProfile(const FString &FilePath, FS_InputProfile &OutProfile)
{
    if (!FPaths::FileExists(*FilePath))
    {
        UE_LOG(LogTemp, Warning, TEXT("P_MEIS: Import file not found: %s"), *FilePath);
        return false;
    }

    FString JsonString;
    if (!FFileHelper::LoadFileToString(JsonString, *FilePath))
    {
        UE_LOG(LogTemp, Warning, TEXT("P_MEIS: Failed to load import file: %s"), *FilePath);
        return false;
    }

    return DeserializeProfileFromJson(JsonString, OutProfile);
}

FString UCPP_InputProfileStorage::GetProfileDirectory()
{
    return FPaths::ProjectSavedDir() + TEXT("InputProfiles/");
}

FString UCPP_InputProfileStorage::GetProfileFilePath(const FName &ProfileName)
{
    return GetProfileDirectory() + ProfileName.ToString() + TEXT(".json");
}

FString UCPP_InputProfileStorage::SerializeProfileToJson(const FS_InputProfile &Profile)
{
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

    JsonObject->SetStringField(TEXT("ProfileName"), Profile.ProfileName.ToString());
    JsonObject->SetStringField(TEXT("ProfileDescription"), Profile.ProfileDescription.ToString());
    JsonObject->SetStringField(TEXT("CreatedBy"), Profile.CreatedBy);
    JsonObject->SetNumberField(TEXT("Version"), Profile.Version);
    JsonObject->SetBoolField(TEXT("bIsDefault"), Profile.bIsDefault);
    JsonObject->SetBoolField(TEXT("bIsCompetitive"), Profile.bIsCompetitive);

    // Optional gameplay preferences (modular; action names provided by game/module)
    {
        TArray<TSharedPtr<FJsonValue>> ToggleModeActionsArray;
        for (const FName &ActionName : Profile.ToggleModeActions)
        {
            ToggleModeActionsArray.Add(MakeShareable(new FJsonValueString(ActionName.ToString())));
        }
        JsonObject->SetArrayField(TEXT("ToggleModeActions"), ToggleModeActionsArray);

        TArray<TSharedPtr<FJsonValue>> ActiveActionTogglesArray;
        for (const FName &ActionName : Profile.bActiveActionToggles)
        {
            ActiveActionTogglesArray.Add(MakeShareable(new FJsonValueString(ActionName.ToString())));
        }
        JsonObject->SetArrayField(TEXT("bActiveActionToggles"), ActiveActionTogglesArray);
    }

    // Serialize action bindings
    TArray<TSharedPtr<FJsonValue>> ActionBindingsArray;
    for (const FS_InputActionBinding &ActionBinding : Profile.ActionBindings)
    {
        TSharedPtr<FJsonObject> ActionObj = MakeShareable(new FJsonObject());
        ActionObj->SetStringField(TEXT("InputActionName"), ActionBinding.InputActionName.ToString());
        ActionObj->SetStringField(TEXT("DisplayName"), ActionBinding.DisplayName.ToString());
        ActionObj->SetStringField(TEXT("Category"), ActionBinding.Category.ToString());
        ActionObj->SetStringField(TEXT("Description"), ActionBinding.Description.ToString());
        ActionObj->SetNumberField(TEXT("Priority"), ActionBinding.Priority);
        ActionObj->SetBoolField(TEXT("bEnabled"), ActionBinding.bEnabled);

        // Key bindings
        TArray<TSharedPtr<FJsonValue>> KeyBindingsArray;
        for (const FS_KeyBinding &KeyBinding : ActionBinding.KeyBindings)
        {
            TSharedPtr<FJsonObject> KeyObj = MakeShareable(new FJsonObject());
            KeyObj->SetStringField(TEXT("Key"), KeyBinding.Key.GetFName().ToString());
            KeyObj->SetNumberField(TEXT("Value"), KeyBinding.Value);
            KeyObj->SetBoolField(TEXT("bShift"), KeyBinding.bShift);
            KeyObj->SetBoolField(TEXT("bCtrl"), KeyBinding.bCtrl);
            KeyObj->SetBoolField(TEXT("bAlt"), KeyBinding.bAlt);
            KeyObj->SetBoolField(TEXT("bCmd"), KeyBinding.bCmd);
            KeyBindingsArray.Add(MakeShareable(new FJsonValueObject(KeyObj)));
        }
        ActionObj->SetArrayField(TEXT("KeyBindings"), KeyBindingsArray);

        ActionBindingsArray.Add(MakeShareable(new FJsonValueObject(ActionObj)));
    }
    JsonObject->SetArrayField(TEXT("ActionBindings"), ActionBindingsArray);

    // Serialize axis bindings
    TArray<TSharedPtr<FJsonValue>> AxisBindingsArray;
    for (const FS_InputAxisBinding &AxisBinding : Profile.AxisBindings)
    {
        TSharedPtr<FJsonObject> AxisObj = MakeShareable(new FJsonObject());
        AxisObj->SetStringField(TEXT("InputAxisName"), AxisBinding.InputAxisName.ToString());
        AxisObj->SetStringField(TEXT("DisplayName"), AxisBinding.DisplayName.ToString());
        AxisObj->SetStringField(TEXT("Category"), AxisBinding.Category.ToString());
        AxisObj->SetStringField(TEXT("Description"), AxisBinding.Description.ToString());
        AxisObj->SetNumberField(TEXT("ValueType"), static_cast<int32>(AxisBinding.ValueType));
        AxisObj->SetNumberField(TEXT("DeadZone"), AxisBinding.DeadZone);
        AxisObj->SetNumberField(TEXT("Sensitivity"), AxisBinding.Sensitivity);
        AxisObj->SetNumberField(TEXT("Priority"), AxisBinding.Priority);
        AxisObj->SetBoolField(TEXT("bInvert"), AxisBinding.bInvert);
        AxisObj->SetBoolField(TEXT("bEnabled"), AxisBinding.bEnabled);

        // Axis key bindings
        TArray<TSharedPtr<FJsonValue>> AxisKeysArray;
        for (const FS_AxisKeyBinding &AxisKey : AxisBinding.AxisBindings)
        {
            TSharedPtr<FJsonObject> AxisKeyObj = MakeShareable(new FJsonObject());
            AxisKeyObj->SetStringField(TEXT("Key"), AxisKey.Key.GetFName().ToString());
            AxisKeyObj->SetNumberField(TEXT("Scale"), AxisKey.Scale);
            AxisKeyObj->SetBoolField(TEXT("bSwizzleYXZ"), AxisKey.bSwizzleYXZ);
            AxisKeysArray.Add(MakeShareable(new FJsonValueObject(AxisKeyObj)));
        }
        AxisObj->SetArrayField(TEXT("AxisBindings"), AxisKeysArray);

        AxisBindingsArray.Add(MakeShareable(new FJsonValueObject(AxisObj)));
    }
    JsonObject->SetArrayField(TEXT("AxisBindings"), AxisBindingsArray);

    // Serialize modifiers
    TArray<TSharedPtr<FJsonValue>> ModifiersArray;
    for (const FS_InputModifier &Modifier : Profile.Modifiers)
    {
        TSharedPtr<FJsonObject> ModObj = MakeShareable(new FJsonObject());
        ModObj->SetNumberField(TEXT("ModifierType"), static_cast<int32>(Modifier.ModifierType));
        ModObj->SetNumberField(TEXT("DeadZoneValue"), Modifier.DeadZoneValue);
        ModObj->SetNumberField(TEXT("ScaleValue"), Modifier.ScaleValue);
        ModObj->SetBoolField(TEXT("bEnabled"), Modifier.bEnabled);

        ModifiersArray.Add(MakeShareable(new FJsonValueObject(ModObj)));
    }
    JsonObject->SetArrayField(TEXT("Modifiers"), ModifiersArray);

    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

    return OutputString;
}
bool UCPP_InputProfileStorage::DeserializeProfileFromJson(const FString &JsonString, FS_InputProfile &OutProfile)
{
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

    if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("P_MEIS: Failed to parse JSON"));
        return false;
    }

    OutProfile.ProfileName = FName(*JsonObject->GetStringField(TEXT("ProfileName")));
    OutProfile.ProfileDescription = FText::FromString(JsonObject->GetStringField(TEXT("ProfileDescription")));
    OutProfile.CreatedBy = JsonObject->GetStringField(TEXT("CreatedBy"));

    int32 Version = 1;
    if (JsonObject->TryGetNumberField(TEXT("Version"), Version))
    {
        OutProfile.Version = Version;
    }
    else
    {
        OutProfile.Version = 1;
    }

    bool bIsDefault = false;
    if (JsonObject->TryGetBoolField(TEXT("bIsDefault"), bIsDefault))
    {
        OutProfile.bIsDefault = bIsDefault;
    }

    bool bIsCompetitive = false;
    if (JsonObject->TryGetBoolField(TEXT("bIsCompetitive"), bIsCompetitive))
    {
        OutProfile.bIsCompetitive = bIsCompetitive;
    }

    // Optional gameplay preferences (safe defaults if missing)
    OutProfile.ToggleModeActions.Empty();
    const TArray<TSharedPtr<FJsonValue>> *ToggleModeActionsArray = nullptr;
    if (JsonObject->TryGetArrayField(TEXT("ToggleModeActions"), ToggleModeActionsArray))
    {
        for (const TSharedPtr<FJsonValue> &ActionValue : *ToggleModeActionsArray)
        {
            FString ActionNameStr;
            if (ActionValue->TryGetString(ActionNameStr))
            {
                OutProfile.ToggleModeActions.Add(FName(*ActionNameStr));
            }
        }
    }

    OutProfile.bActiveActionToggles.Empty();
    const TArray<TSharedPtr<FJsonValue>> *ActiveActionTogglesArray = nullptr;
    if (JsonObject->TryGetArrayField(TEXT("bActiveActionToggles"), ActiveActionTogglesArray))
    {
        for (const TSharedPtr<FJsonValue> &ActionValue : *ActiveActionTogglesArray)
        {
            FString ActionNameStr;
            if (ActionValue->TryGetString(ActionNameStr))
            {
                OutProfile.bActiveActionToggles.Add(FName(*ActionNameStr));
            }
        }
    }

    // Deserialize action bindings
    OutProfile.ActionBindings.Empty();
    const TArray<TSharedPtr<FJsonValue>> *ActionBindingsArray = nullptr;
    if (JsonObject->TryGetArrayField(TEXT("ActionBindings"), ActionBindingsArray))
    {
        for (const TSharedPtr<FJsonValue> &Value : *ActionBindingsArray)
        {
            TSharedPtr<FJsonObject> ActionObj = Value->AsObject();
            if (ActionObj.IsValid())
            {
                FS_InputActionBinding ActionBinding;
                ActionBinding.InputActionName = FName(*ActionObj->GetStringField(TEXT("InputActionName")));
                ActionBinding.DisplayName = FText::FromString(ActionObj->GetStringField(TEXT("DisplayName")));
                ActionBinding.Category = FName(*ActionObj->GetStringField(TEXT("Category")));
                ActionBinding.Description = FText::FromString(ActionObj->GetStringField(TEXT("Description")));
                ActionBinding.Priority = ActionObj->GetNumberField(TEXT("Priority"));
                ActionBinding.bEnabled = ActionObj->GetBoolField(TEXT("bEnabled"));

                // Key bindings (optional for older profiles)
                ActionBinding.KeyBindings.Empty();
                const TArray<TSharedPtr<FJsonValue>> *KeyBindingsArray = nullptr;
                if (ActionObj->TryGetArrayField(TEXT("KeyBindings"), KeyBindingsArray))
                {
                    for (const TSharedPtr<FJsonValue> &KeyValue : *KeyBindingsArray)
                    {
                        const TSharedPtr<FJsonObject> KeyObj = KeyValue->AsObject();
                        if (!KeyObj.IsValid())
                        {
                            continue;
                        }

                        FS_KeyBinding KeyBinding;
                        KeyBinding.Key = FKey(*KeyObj->GetStringField(TEXT("Key")));

                        double ValueNum = 1.0;
                        if (KeyObj->TryGetNumberField(TEXT("Value"), ValueNum))
                        {
                            KeyBinding.Value = static_cast<float>(ValueNum);
                        }

                        KeyObj->TryGetBoolField(TEXT("bShift"), KeyBinding.bShift);
                        KeyObj->TryGetBoolField(TEXT("bCtrl"), KeyBinding.bCtrl);
                        KeyObj->TryGetBoolField(TEXT("bAlt"), KeyBinding.bAlt);
                        KeyObj->TryGetBoolField(TEXT("bCmd"), KeyBinding.bCmd);

                        ActionBinding.KeyBindings.Add(KeyBinding);
                    }
                }

                OutProfile.ActionBindings.Add(ActionBinding);
            }
        }
    }

    // Deserialize axis bindings
    OutProfile.AxisBindings.Empty();
    const TArray<TSharedPtr<FJsonValue>> *AxisBindingsArray = nullptr;
    if (JsonObject->TryGetArrayField(TEXT("AxisBindings"), AxisBindingsArray))
    {
        for (const TSharedPtr<FJsonValue> &Value : *AxisBindingsArray)
        {
            TSharedPtr<FJsonObject> AxisObj = Value->AsObject();
            if (AxisObj.IsValid())
            {
                FS_InputAxisBinding AxisBinding;
                AxisBinding.InputAxisName = FName(*AxisObj->GetStringField(TEXT("InputAxisName")));
                AxisBinding.Category = FName(*AxisObj->GetStringField(TEXT("Category")));

                FString DisplayNameStr;
                if (AxisObj->TryGetStringField(TEXT("DisplayName"), DisplayNameStr))
                {
                    AxisBinding.DisplayName = FText::FromString(DisplayNameStr);
                }

                FString DescriptionStr;
                if (AxisObj->TryGetStringField(TEXT("Description"), DescriptionStr))
                {
                    AxisBinding.Description = FText::FromString(DescriptionStr);
                }

                int32 ValueTypeInt = static_cast<int32>(AxisBinding.ValueType);
                if (AxisObj->TryGetNumberField(TEXT("ValueType"), ValueTypeInt))
                {
                    AxisBinding.ValueType = static_cast<EInputActionValueType>(ValueTypeInt);
                }

                AxisBinding.DeadZone = AxisObj->GetNumberField(TEXT("DeadZone"));
                AxisBinding.Sensitivity = AxisObj->GetNumberField(TEXT("Sensitivity"));

                double PriorityNum = AxisBinding.Priority;
                if (AxisObj->TryGetNumberField(TEXT("Priority"), PriorityNum))
                {
                    AxisBinding.Priority = static_cast<float>(PriorityNum);
                }

                AxisBinding.bInvert = AxisObj->GetBoolField(TEXT("bInvert"));
                AxisBinding.bEnabled = AxisObj->GetBoolField(TEXT("bEnabled"));

                // Axis key bindings (optional for older profiles)
                AxisBinding.AxisBindings.Empty();
                const TArray<TSharedPtr<FJsonValue>> *AxisKeysArray = nullptr;
                if (AxisObj->TryGetArrayField(TEXT("AxisBindings"), AxisKeysArray))
                {
                    for (const TSharedPtr<FJsonValue> &AxisKeyValue : *AxisKeysArray)
                    {
                        const TSharedPtr<FJsonObject> AxisKeyObj = AxisKeyValue->AsObject();
                        if (!AxisKeyObj.IsValid())
                        {
                            continue;
                        }

                        FS_AxisKeyBinding AxisKey;
                        AxisKey.Key = FKey(*AxisKeyObj->GetStringField(TEXT("Key")));

                        double ScaleNum = 1.0;
                        if (AxisKeyObj->TryGetNumberField(TEXT("Scale"), ScaleNum))
                        {
                            AxisKey.Scale = static_cast<float>(ScaleNum);
                        }
                        AxisKeyObj->TryGetBoolField(TEXT("bSwizzleYXZ"), AxisKey.bSwizzleYXZ);

                        AxisBinding.AxisBindings.Add(AxisKey);
                    }
                }

                OutProfile.AxisBindings.Add(AxisBinding);
            }
        }
    }

    // Deserialize modifiers
    OutProfile.Modifiers.Empty();
    const TArray<TSharedPtr<FJsonValue>> *ModifiersArray = nullptr;
    if (JsonObject->TryGetArrayField(TEXT("Modifiers"), ModifiersArray))
    {
        for (const TSharedPtr<FJsonValue> &Value : *ModifiersArray)
        {
            TSharedPtr<FJsonObject> ModObj = Value->AsObject();
            if (ModObj.IsValid())
            {
                FS_InputModifier Modifier;
                Modifier.ModifierType = static_cast<EInputModifierType>(ModObj->GetIntegerField(TEXT("ModifierType")));
                Modifier.DeadZoneValue = ModObj->GetNumberField(TEXT("DeadZoneValue"));
                Modifier.ScaleValue = ModObj->GetNumberField(TEXT("ScaleValue"));
                Modifier.bEnabled = ModObj->GetBoolField(TEXT("bEnabled"));

                OutProfile.Modifiers.Add(Modifier);
            }
        }
    }

    return true;
}