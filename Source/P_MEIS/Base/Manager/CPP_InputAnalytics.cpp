/*
 * @Author: Punal Manalan
 * @Description: Input Analytics System Implementation
 * @Date: 06/12/2025
 */

#include "Manager/CPP_InputAnalytics.h"

void UCPP_InputAnalytics::RecordKeyPress(const FKey& Key)
{
if (!KeyUsageMap.Contains(Key))
{
KeyUsageMap.Add(Key, FS_KeyUsageData());
KeyUsageMap[Key].Key = Key;
}

FS_KeyUsageData& Data = KeyUsageMap[Key];
Data.PressCount++;
Data.LastUsedTime = FPlatformTime::Seconds();
Data.bIsHeld = true;
}

void UCPP_InputAnalytics::RecordKeyRelease(const FKey& Key)
{
if (KeyUsageMap.Contains(Key))
{
FS_KeyUsageData& Data = KeyUsageMap[Key];
Data.bIsHeld = false;
}
}

void UCPP_InputAnalytics::RecordAxisInput(const FName& AxisName, float Value)
{
// Advanced axis analytics could be implemented here
UE_LOG(LogTemp, Verbose, TEXT("P_MEIS: Axis input recorded - %s: %f"), *AxisName.ToString(), Value);
}

bool UCPP_InputAnalytics::GetKeyUsageData(const FKey& Key, FS_KeyUsageData& OutData)
{
if (KeyUsageMap.Contains(Key))
{
OutData = KeyUsageMap[Key];
return true;
}

return false;
}

void UCPP_InputAnalytics::GetMostUsedKeys(int32 Count, TArray<FKey>& OutKeys)
{
OutKeys.Empty();

// Sort by press count
TArray<TPair<FKey, int32>> SortedKeys;
for (const auto& Pair : KeyUsageMap)
{
SortedKeys.Add(TPair<FKey, int32>(Pair.Key, Pair.Value.PressCount));
}

SortedKeys.Sort([](const TPair<FKey, int32>& A, const TPair<FKey, int32>& B)
{
return A.Value > B.Value;
});

for (int32 i = 0; i < FMath::Min(Count, SortedKeys.Num()); ++i)
{
OutKeys.Add(SortedKeys[i].Key);
}
}

void UCPP_InputAnalytics::GetLeastUsedKeys(int32 Count, TArray<FKey>& OutKeys)
{
OutKeys.Empty();

TArray<TPair<FKey, int32>> SortedKeys;
for (const auto& Pair : KeyUsageMap)
{
SortedKeys.Add(TPair<FKey, int32>(Pair.Key, Pair.Value.PressCount));
}

SortedKeys.Sort([](const TPair<FKey, int32>& A, const TPair<FKey, int32>& B)
{
return A.Value < B.Value;
});

for (int32 i = 0; i < FMath::Min(Count, SortedKeys.Num()); ++i)
{
OutKeys.Add(SortedKeys[i].Key);
}
}

void UCPP_InputAnalytics::ResetAnalytics()
{
KeyUsageMap.Empty();
LatencyHistory.Empty();
TotalFrameLatency = 0.0f;
FrameCount = 0;
UE_LOG(LogTemp, Log, TEXT("P_MEIS: Analytics reset"));
}

float UCPP_InputAnalytics::GetAverageLatency() const
{
if (FrameCount == 0)
{
return 0.0f;
}

return TotalFrameLatency / static_cast<float>(FrameCount);
}
