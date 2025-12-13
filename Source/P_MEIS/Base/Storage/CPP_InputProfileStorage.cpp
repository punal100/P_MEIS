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

        ActionBindingsArray.Add(MakeShareable(new FJsonValueObject(ActionObj)));
    }
    JsonObject->SetArrayField(TEXT("ActionBindings"), ActionBindingsArray);

    // Serialize axis bindings
    TArray<TSharedPtr<FJsonValue>> AxisBindingsArray;
    for (const FS_InputAxisBinding &AxisBinding : Profile.AxisBindings)
    {
        TSharedPtr<FJsonObject> AxisObj = MakeShareable(new FJsonObject());
        AxisObj->SetStringField(TEXT("InputAxisName"), AxisBinding.InputAxisName.ToString());
        AxisObj->SetStringField(TEXT("Category"), AxisBinding.Category.ToString());
        AxisObj->SetNumberField(TEXT("DeadZone"), AxisBinding.DeadZone);
        AxisObj->SetNumberField(TEXT("Sensitivity"), AxisBinding.Sensitivity);
        AxisObj->SetBoolField(TEXT("bInvert"), AxisBinding.bInvert);
        AxisObj->SetBoolField(TEXT("bEnabled"), AxisBinding.bEnabled);

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
    OutProfile.Version = JsonObject->GetIntegerField(TEXT("Version"));
    OutProfile.bIsDefault = JsonObject->GetBoolField(TEXT("bIsDefault"));
    OutProfile.bIsCompetitive = JsonObject->GetBoolField(TEXT("bIsCompetitive"));

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
                AxisBinding.DeadZone = AxisObj->GetNumberField(TEXT("DeadZone"));
                AxisBinding.Sensitivity = AxisObj->GetNumberField(TEXT("Sensitivity"));
                AxisBinding.bInvert = AxisObj->GetBoolField(TEXT("bInvert"));
                AxisBinding.bEnabled = AxisObj->GetBoolField(TEXT("bEnabled"));

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