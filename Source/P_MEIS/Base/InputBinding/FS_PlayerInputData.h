/*
 * @Author: Punal Manalan
 * @Description: Player Input Data Structure - Per-player profile and integration data
 * @Date: 06/12/2025
 */

#pragma once

#include "CoreMinimal.h"
#include "FS_InputProfile.h"
#include "FS_PlayerInputData.generated.h"

class UCPP_EnhancedInputIntegration;

/**
 * Per-player input data containing their profile and Enhanced Input integration
 * Each player has their OWN profile (key bindings) and integration (runtime IA/IMC)
 */
USTRUCT(BlueprintType)
struct FS_PlayerInputData
{
    GENERATED_BODY()

    /** This player's active input configuration (their OWN key bindings) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Input")
    FS_InputProfile ActiveProfile;

    /** This player's Enhanced Input bridge (creates runtime IA/IMC) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Input")
    UCPP_EnhancedInputIntegration* Integration = nullptr;

    /** Name of the template this profile was loaded from (for save/reload tracking) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Input")
    FName LoadedTemplateName = NAME_None;

    /** Default constructor */
    FS_PlayerInputData()
        : Integration(nullptr)
        , LoadedTemplateName(NAME_None)
    {
    }

    /** Check if this player data is valid (has an integration) */
    bool IsValid() const
    {
        return Integration != nullptr;
    }
};
