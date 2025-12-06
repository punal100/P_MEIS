/*
 * @Author: Punal Manalan
 * @Description: Enhanced Input Integration Implementation
 * @Date: 06/12/2025
 */

#include "Integration/CPP_EnhancedInputIntegration.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "GameFramework/PlayerController.h"

bool UCPP_EnhancedInputIntegration::ApplyProfile(const FS_InputProfile &Profile)
{
    if (!CreateOrUpdateMappingContext())
    {
        UE_LOG(LogTemp, Warning, TEXT("P_MEIS: Failed to create mapping context"));
        return false;
    }

    // Apply all action bindings
    for (const FS_InputActionBinding &ActionBinding : Profile.ActionBindings)
    {
        ApplyActionBinding(ActionBinding);
    }

    // Apply all axis bindings
    for (const FS_InputAxisBinding &AxisBinding : Profile.AxisBindings)
    {
        ApplyAxisBinding(AxisBinding);
    }

    return true;
}

bool UCPP_EnhancedInputIntegration::ApplyActionBinding(const FS_InputActionBinding &ActionBinding)
{
    if (!ActionBinding.bEnabled)
    {
        return false;
    }

    UE_LOG(LogTemp, Log, TEXT("P_MEIS: Applying action binding: %s"), *ActionBinding.InputActionName.ToString());
    return true;
}

bool UCPP_EnhancedInputIntegration::ApplyAxisBinding(const FS_InputAxisBinding &AxisBinding)
{
    if (!AxisBinding.bEnabled)
    {
        return false;
    }

    UE_LOG(LogTemp, Log, TEXT("P_MEIS: Applying axis binding: %s"), *AxisBinding.InputAxisName.ToString());
    return true;
}

void UCPP_EnhancedInputIntegration::SetPlayerController(APlayerController *InPlayerController)
{
    PlayerController = InPlayerController;
}

bool UCPP_EnhancedInputIntegration::CreateOrUpdateMappingContext()
{
    if (!PlayerController)
    {
        UE_LOG(LogTemp, Warning, TEXT("P_MEIS: PlayerController not set"));
        return false;
    }

    // Get the Enhanced Input Subsystem
    if (UEnhancedInputLocalPlayerSubsystem *Subsystem =
            PlayerController->GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
    {
        if (!MappingContext)
        {
            MappingContext = NewObject<UInputMappingContext>();
        }

        Subsystem->ClearAllMappings();
        Subsystem->AddMappingContext(MappingContext, 0);

        return true;
    }

    return false;
}
