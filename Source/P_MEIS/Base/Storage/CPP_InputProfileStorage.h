/*
 * @Author: Punal Manalan
 * @Description: Input Profile Storage - Handles loading/saving profiles to disk
 * @Date: 06/12/2025
 */

#pragma once

#include "CoreMinimal.h"
#include "InputBinding/FS_InputProfile.h"
#include "CPP_InputProfileStorage.generated.h"

/**
 * Static utility class for profile persistence
 */
UCLASS()
class P_MEIS_API UCPP_InputProfileStorage : public UObject
{
    GENERATED_BODY()

public:
    // Profile IO
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Storage")
    static bool SaveProfile(const FS_InputProfile &Profile);

    UFUNCTION(BlueprintCallable, Category = "Input Binding|Storage")
    static bool LoadProfile(const FName &ProfileName, FS_InputProfile &OutProfile);

    UFUNCTION(BlueprintCallable, Category = "Input Binding|Storage")
    static bool DeleteProfile(const FName &ProfileName);

    UFUNCTION(BlueprintCallable, Category = "Input Binding|Storage")
    static bool ProfileExists(const FName &ProfileName);

    UFUNCTION(BlueprintCallable, Category = "Input Binding|Storage")
    static void GetAvailableProfiles(TArray<FName> &OutProfiles);

    // Import/Export
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Storage")
    static bool ExportProfile(const FS_InputProfile &Profile, const FString &FilePath);

    UFUNCTION(BlueprintCallable, Category = "Input Binding|Storage")
    static bool ImportProfile(const FString &FilePath, FS_InputProfile &OutProfile);

    static FString GetProfileDirectory();
    static FString GetProfileFilePath(const FName &ProfileName);
    static FString SerializeProfileToJson(const FS_InputProfile &Profile);
    static bool DeserializeProfileFromJson(const FString &JsonString, FS_InputProfile &OutProfile);
};
