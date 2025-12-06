/*
 * @Author: Punal Manalan
 * @Description: Context-Aware Input Binding System
 * @Date: 06/12/2025
 */

#pragma once

#include "CoreMinimal.h"
#include "InputBinding/FS_InputProfile.h"
#include "CPP_InputContextManager.generated.h"

/**
 * Enum for different input contexts
 */
UENUM(BlueprintType)
enum class EInputContext : uint8
{
    Menu = 0 UMETA(DisplayName = "Menu"),
    Gameplay = 1 UMETA(DisplayName = "Gameplay"),
    Cutscene = 2 UMETA(DisplayName = "Cutscene"),
    Vehicle = 3 UMETA(DisplayName = "Vehicle"),
    Custom1 = 4 UMETA(DisplayName = "Custom 1"),
    Custom2 = 5 UMETA(DisplayName = "Custom 2"),
    Custom3 = 6 UMETA(DisplayName = "Custom 3"),
    Custom4 = 7 UMETA(DisplayName = "Custom 4")
};

/**
 * Structure for context-specific bindings
 */
USTRUCT(BlueprintType)
struct FS_ContextBinding
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Binding")
    EInputContext Context = EInputContext::Gameplay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Binding")
    FS_InputProfile ContextProfile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Binding")
    float Priority = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Binding")
    bool bEnabled = true;
};

/**
 * Manager for context-aware input bindings
 */
UCLASS()
class P_MEIS_API UCPP_InputContextManager : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Input Binding|Context")
    bool SetInputContext(EInputContext NewContext);

    UFUNCTION(BlueprintCallable, Category = "Input Binding|Context")
    EInputContext GetCurrentContext() const { return CurrentContext; }

    UFUNCTION(BlueprintCallable, Category = "Input Binding|Context")
    bool RegisterContextProfile(EInputContext Context, const FS_InputProfile &Profile, float Priority = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "Input Binding|Context")
    bool GetContextProfile(EInputContext Context, FS_InputProfile &OutProfile);

    UFUNCTION(BlueprintCallable, Category = "Input Binding|Context")
    void ListContexts(TArray<uint8> &OutContexts);

private:
    UPROPERTY()
    EInputContext CurrentContext = EInputContext::Gameplay;

    UPROPERTY()
    TArray<FS_ContextBinding> ContextBindings;

    UPROPERTY()
    class UCPP_InputBindingManager *BindingManager;
};
