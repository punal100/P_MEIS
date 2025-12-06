/*
 * @Author: Punal Manalan
 * @Description: Accessibility Features for Input Binding System
 * @Date: 06/12/2025
 */

#pragma once

#include "CoreMinimal.h"
#include "CPP_InputAccessibility.generated.h"

/**
 * Accessibility settings structure
 */
USTRUCT(BlueprintType)
struct FS_AccessibilitySettings
{
GENERATED_BODY()

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility")
bool bEnableLargeText = false;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility")
bool bHighContrast = false;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility")
bool bInvertColors = false;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility")
float TextScale = 1.0f;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility")
bool bEnableKeyHold = false;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility")
bool bAnalogToDigitalConversion = false;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility")
float AnalogThreshold = 0.5f;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility")
float RepeatRate = 0.1f;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility")
bool bEnableScreenReader = false;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility")
bool bEnableControllerVibration = true;
};

/**
 * Input Accessibility Manager
 */
UCLASS()
class P_MEIS_API UCPP_InputAccessibility : public UObject
{
GENERATED_BODY()

public:
UFUNCTION(BlueprintCallable, Category = "Input Binding|Accessibility")
void SetAccessibilitySettings(const FS_AccessibilitySettings& Settings);

UFUNCTION(BlueprintCallable, Category = "Input Binding|Accessibility")
FS_AccessibilitySettings GetAccessibilitySettings() const { return AccessibilitySettings; }

UFUNCTION(BlueprintCallable, Category = "Input Binding|Accessibility")
void EnableLargeText(bool bEnable);

UFUNCTION(BlueprintCallable, Category = "Input Binding|Accessibility")
void EnableHighContrast(bool bEnable);

UFUNCTION(BlueprintCallable, Category = "Input Binding|Accessibility")
void EnableScreenReader(bool bEnable);

UFUNCTION(BlueprintCallable, Category = "Input Binding|Accessibility")
void SetTextScale(float Scale);

UFUNCTION(BlueprintCallable, Category = "Input Binding|Accessibility")
void EnableAnalogToDigitalConversion(bool bEnable);

UFUNCTION(BlueprintCallable, Category = "Input Binding|Accessibility")
void SetAnalogThreshold(float Threshold);

UFUNCTION(BlueprintCallable, Category = "Input Binding|Accessibility")
void SetKeyRepeatRate(float Rate);

UFUNCTION(BlueprintCallable, Category = "Input Binding|Accessibility")
bool IsAccessibilityFeatureEnabled(const FString& FeatureName);

private:
UPROPERTY()
FS_AccessibilitySettings AccessibilitySettings;
};
