/*
 * @Author: Punal Manalan
 * @Description: Modular Enhanced Input System - Module Implementation
 * @Date: 06/12/2025
 */

#include "P_MEIS.h"

#define LOCTEXT_NAMESPACE "FP_MEISModule"

void FP_MEISModule::StartupModule()
{
// This code will execute after your module is loaded into memory
// The exact timing is specified in the .uplugin file per-module
UE_LOG(LogTemp, Log, TEXT("P_MEIS: Module Started"));
}

void FP_MEISModule::ShutdownModule()
{
// This function may be called during shutdown to clean up your module.
// For modules that support dynamic reloading, we call this function before unloading the module.
UE_LOG(LogTemp, Log, TEXT("P_MEIS: Module Shutdown"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FP_MEISModule, P_MEIS)
