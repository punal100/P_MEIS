/*
 * @Author: Punal Manalan
 * @Description: Input Profile Structure - Complete input configuration snapshot
 * @Date: 06/12/2025
 */

#pragma once

#include "CoreMinimal.h"
#include "FS_InputActionBinding.h"
#include "FS_InputAxisBinding.h"
#include "FS_InputModifier.h"
#include "FS_InputProfile.generated.h"

/**
 * Complete input profile containing all bindings
 */
USTRUCT(BlueprintType)
struct FS_InputProfile
{
GENERATED_BODY()

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Profile")
FName ProfileName;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Profile")
FText ProfileDescription;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Profile")
TArray<FS_InputActionBinding> ActionBindings;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Profile")
TArray<FS_InputAxisBinding> AxisBindings;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Profile")
TArray<FS_InputModifier> Modifiers;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Profile")
FDateTime Timestamp = FDateTime::Now();

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Profile")
int32 Version = 1;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Profile")
FString CreatedBy;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Profile")
bool bIsDefault = false;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Profile")
bool bIsCompetitive = false;
};
