/*
 * @Author: Punal Manalan
 * @Description: Modular Enhanced Input System - Public Module Header
 * @Date: 06/12/2025
 */

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/**
 * P_MEIS Module Interface
 * Modular Enhanced Input System plugin module
 */
class FP_MEISModule : public IModuleInterface
{
public:

/** IModuleInterface implementation */
virtual void StartupModule() override;
virtual void ShutdownModule() override;

/**
 * Singleton-like access to this module's interface.
 * @return Returns singleton instance, loading the module on demand if needed.
 */
static inline FP_MEISModule& Get()
{
return FModuleManager::LoadModuleChecked<FP_MEISModule>("P_MEIS");
}

/**
 * Checks to see if this module is loaded and ready.
 * @return True if the module is loaded and ready to use.
 */
static inline bool IsAvailable()
{
return FModuleManager::Get().IsModuleLoaded("P_MEIS");
}
};
