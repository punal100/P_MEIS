/*
 * @Author: Punal Manalan
 * @Description: Enhanced Input Integration - Bridges P_MEIS with UE5 Enhanced Input System
 * @Date: 06/12/2025
 */

#pragma once

#include "CoreMinimal.h"
#include "InputBinding/FS_InputProfile.h"
#include "CPP_EnhancedInputIntegration.generated.h"

class APlayerController;
class UEnhancedInputComponent;
class UInputMappingContext;

/**
 * Integration layer with UE5 Enhanced Input System
 */
UCLASS()
class P_MEIS_API UCPP_EnhancedInputIntegration : public UObject
{
GENERATED_BODY()

public:
UFUNCTION(BlueprintCallable, Category = "Input Binding|Integration")
bool ApplyProfile(const FS_InputProfile& Profile);

UFUNCTION(BlueprintCallable, Category = "Input Binding|Integration")
bool ApplyActionBinding(const FS_InputActionBinding& ActionBinding);

UFUNCTION(BlueprintCallable, Category = "Input Binding|Integration")
bool ApplyAxisBinding(const FS_InputAxisBinding& AxisBinding);

UFUNCTION(BlueprintCallable, Category = "Input Binding|Integration")
void SetPlayerController(APlayerController* InPlayerController);

private:
UPROPERTY()
APlayerController* PlayerController;

UPROPERTY()
UInputMappingContext* MappingContext;

bool CreateOrUpdateMappingContext();
};
