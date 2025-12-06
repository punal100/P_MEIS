/*
 * @Author: Punal Manalan
 * @Description: Input Accessibility Implementation
 * @Date: 06/12/2025
 */

#include "Manager/CPP_InputAccessibility.h"

void UCPP_InputAccessibility::SetAccessibilitySettings(const FS_AccessibilitySettings& Settings)
{
AccessibilitySettings = Settings;
UE_LOG(LogTemp, Log, TEXT("P_MEIS: Accessibility settings updated"));
}

void UCPP_InputAccessibility::EnableLargeText(bool bEnable)
{
AccessibilitySettings.bEnableLargeText = bEnable;
if (bEnable)
{
AccessibilitySettings.TextScale = 1.5f;
}
else
{
AccessibilitySettings.TextScale = 1.0f;
}

UE_LOG(LogTemp, Log, TEXT("P_MEIS: Large text %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void UCPP_InputAccessibility::EnableHighContrast(bool bEnable)
{
AccessibilitySettings.bHighContrast = bEnable;
UE_LOG(LogTemp, Log, TEXT("P_MEIS: High contrast %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void UCPP_InputAccessibility::EnableScreenReader(bool bEnable)
{
AccessibilitySettings.bEnableScreenReader = bEnable;
UE_LOG(LogTemp, Log, TEXT("P_MEIS: Screen reader %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void UCPP_InputAccessibility::SetTextScale(float Scale)
{
AccessibilitySettings.TextScale = FMath::Clamp(Scale, 0.5f, 3.0f);
UE_LOG(LogTemp, Log, TEXT("P_MEIS: Text scale set to %f"), AccessibilitySettings.TextScale);
}

void UCPP_InputAccessibility::EnableAnalogToDigitalConversion(bool bEnable)
{
AccessibilitySettings.bAnalogToDigitalConversion = bEnable;
UE_LOG(LogTemp, Log, TEXT("P_MEIS: Analog to digital conversion %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void UCPP_InputAccessibility::SetAnalogThreshold(float Threshold)
{
AccessibilitySettings.AnalogThreshold = FMath::Clamp(Threshold, 0.0f, 1.0f);
UE_LOG(LogTemp, Log, TEXT("P_MEIS: Analog threshold set to %f"), AccessibilitySettings.AnalogThreshold);
}

void UCPP_InputAccessibility::SetKeyRepeatRate(float Rate)
{
AccessibilitySettings.RepeatRate = FMath::Clamp(Rate, 0.01f, 1.0f);
UE_LOG(LogTemp, Log, TEXT("P_MEIS: Key repeat rate set to %f"), AccessibilitySettings.RepeatRate);
}

bool UCPP_InputAccessibility::IsAccessibilityFeatureEnabled(const FString& FeatureName)
{
if (FeatureName == TEXT("LargeText"))
{
return AccessibilitySettings.bEnableLargeText;
}
else if (FeatureName == TEXT("HighContrast"))
{
return AccessibilitySettings.bHighContrast;
}
else if (FeatureName == TEXT("ScreenReader"))
{
return AccessibilitySettings.bEnableScreenReader;
}
else if (FeatureName == TEXT("AnalogToDigital"))
{
return AccessibilitySettings.bAnalogToDigitalConversion;
}

return false;
}
