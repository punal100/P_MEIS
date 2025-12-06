/*
 * @Author: Punal Manalan
 * @Description: Input Analytics & Monitoring System
 * @Date: 06/12/2025
 */

#pragma once

#include "CoreMinimal.h"
#include "CPP_InputAnalytics.generated.h"

/**
 * Structure for tracking individual key usage
 */
USTRUCT(BlueprintType)
struct FS_KeyUsageData
{
GENERATED_BODY()

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Analytics")
FKey Key;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Analytics")
int32 PressCount = 0;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Analytics")
float TotalHoldTime = 0.0f;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Analytics")
float LastUsedTime = 0.0f;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Analytics")
bool bIsHeld = false;
};

/**
 * Input Analytics System
 */
UCLASS()
class P_MEIS_API UCPP_InputAnalytics : public UObject
{
GENERATED_BODY()

public:
UFUNCTION(BlueprintCallable, Category = "Input Binding|Analytics")
void RecordKeyPress(const FKey& Key);

UFUNCTION(BlueprintCallable, Category = "Input Binding|Analytics")
void RecordKeyRelease(const FKey& Key);

UFUNCTION(BlueprintCallable, Category = "Input Binding|Analytics")
void RecordAxisInput(const FName& AxisName, float Value);

UFUNCTION(BlueprintCallable, Category = "Input Binding|Analytics")
bool GetKeyUsageData(const FKey& Key, FS_KeyUsageData& OutData);

UFUNCTION(BlueprintCallable, Category = "Input Binding|Analytics")
void GetMostUsedKeys(int32 Count, TArray<FKey>& OutKeys);

UFUNCTION(BlueprintCallable, Category = "Input Binding|Analytics")
void GetLeastUsedKeys(int32 Count, TArray<FKey>& OutKeys);

UFUNCTION(BlueprintCallable, Category = "Input Binding|Analytics")
void ResetAnalytics();

UFUNCTION(BlueprintCallable, Category = "Input Binding|Analytics")
float GetAverageLatency() const;

private:
UPROPERTY()
TMap<FKey, FS_KeyUsageData> KeyUsageMap;

UPROPERTY()
TArray<float> LatencyHistory;

float TotalFrameLatency = 0.0f;
int32 FrameCount = 0;
};
