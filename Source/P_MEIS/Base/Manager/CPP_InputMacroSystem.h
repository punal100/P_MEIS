/*
 * @Author: Punal Manalan
 * @Description: Input Macro/Combo System
 * @Date: 06/12/2025
 */

#pragma once

#include "CoreMinimal.h"
#include "CPP_InputMacroSystem.generated.h"

/**
 * Single macro step/command
 */
USTRUCT(BlueprintType)
struct FS_MacroStep
{
GENERATED_BODY()

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Macro")
FKey Key;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Macro")
float Duration = 0.1f;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Macro")
float DelayAfter = 0.0f;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Macro")
bool bIsHold = false;
};

/**
 * Complete input macro
 */
USTRUCT(BlueprintType)
struct FS_InputMacro
{
GENERATED_BODY()

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Macro")
FName MacroName;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Macro")
FText DisplayName;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Macro")
FText Description;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Macro")
TArray<FS_MacroStep> Steps;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Macro")
float Cooldown = 0.0f;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Macro")
bool bEnabled = true;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Macro")
bool bLoop = false;
};

/**
 * Input Macro System Manager
 */
UCLASS()
class P_MEIS_API UCPP_InputMacroSystem : public UObject
{
GENERATED_BODY()

public:
UFUNCTION(BlueprintCallable, Category = "Input Binding|Macro")
bool RegisterMacro(const FS_InputMacro& Macro);

UFUNCTION(BlueprintCallable, Category = "Input Binding|Macro")
bool PlayMacro(const FName& MacroName);

UFUNCTION(BlueprintCallable, Category = "Input Binding|Macro")
bool StopMacro(const FName& MacroName);

UFUNCTION(BlueprintCallable, Category = "Input Binding|Macro")
bool DeleteMacro(const FName& MacroName);

UFUNCTION(BlueprintCallable, Category = "Input Binding|Macro")
bool GetMacro(const FName& MacroName, FS_InputMacro& OutMacro);

UFUNCTION(BlueprintCallable, Category = "Input Binding|Macro")
void GetAllMacros(TArray<FS_InputMacro>& OutMacros);

UFUNCTION(BlueprintPure, Category = "Input Binding|Macro")
bool IsMacroPlaying(const FName& MacroName) const;

private:
UPROPERTY()
TArray<FS_InputMacro> RegisteredMacros;

UPROPERTY()
TArray<FName> PlayingMacros;

UPROPERTY()
TMap<FName, float> MacroCooldowns;
};
