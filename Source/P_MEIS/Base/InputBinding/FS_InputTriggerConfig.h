/*
 * @Author: Punal Manalan
 * @Description: Input Trigger Configuration for controlling WHEN input actions fire
 *               Comprehensive struct that supports all UE5 Enhanced Input triggers
 *               Used with Key Mapping-level trigger configuration
 * @Date: 07/12/2025
 * @Version: 1.0 - Section 0.9 Initial implementation
 */

#pragma once

#include "CoreMinimal.h"
#include "InputTriggers.h" // For ETriggerEvent, ETriggerState
#include "FS_InputTriggerConfig.generated.h"

// Forward declarations
class UInputTrigger;
class UInputAction;

// ==================== Input Trigger Types ====================
// Maps to UE5 Enhanced Input trigger classes

/**
 * Input Trigger Types - Maps to UE5 Enhanced Input trigger classes
 *
 * Basic Triggers: Down, Pressed, Released
 * Time-Based Triggers: Hold, HoldAndRelease, Tap, Pulse
 * Combination Triggers: ChordAction, Combo
 * Custom: User-defined trigger class
 */
UENUM(BlueprintType)
enum class EInputTriggerType : uint8
{
    Down = 0 UMETA(DisplayName = "Down", ToolTip = "Fire while input exceeds actuation threshold (default behavior)"),
    Pressed = 1 UMETA(DisplayName = "Pressed", ToolTip = "Fire once when input first exceeds threshold"),
    Released = 2 UMETA(DisplayName = "Released", ToolTip = "Fire once when input drops below threshold"),
    Hold = 3 UMETA(DisplayName = "Hold", ToolTip = "Fire after input held for specified duration"),
    HoldAndRelease = 4 UMETA(DisplayName = "Hold And Release", ToolTip = "Fire on release after holding for duration"),
    Tap = 5 UMETA(DisplayName = "Tap", ToolTip = "Fire if input pressed and released quickly"),
    Pulse = 6 UMETA(DisplayName = "Pulse", ToolTip = "Fire repeatedly at interval while held"),
    ChordAction = 7 UMETA(DisplayName = "Chord Action", ToolTip = "Require another action to be active"),
    Combo = 8 UMETA(DisplayName = "Combo", ToolTip = "Require sequence of actions (advanced)"),
    Custom = 255 UMETA(DisplayName = "Custom", ToolTip = "User-defined custom trigger class")
};

// ==================== Input Trigger Configuration ====================

/**
 * FS_InputTriggerConfig - Comprehensive input trigger configuration
 *
 * Supports ALL UE5 Enhanced Input trigger types with full parameter control.
 * Triggers determine WHEN an action fires based on input state transitions.
 *
 * Trigger Types:
 * - Down: Fire continuously while input is held (default)
 * - Pressed: Fire once on initial press
 * - Released: Fire once on release
 * - Hold: Fire after holding for HoldTimeThreshold seconds
 * - HoldAndRelease: Fire on release if held for HoldTimeThreshold
 * - Tap: Fire if pressed and released within TapReleaseTimeThreshold
 * - Pulse: Fire repeatedly at PulseInterval while held
 * - ChordAction: Require another action to be triggering
 *
 * Example Usage:
 * - Quick Fire: Use Pressed trigger
 * - Charge Attack: Use Hold with bIsOneShot = false
 * - Toggle-style action: Use Tap trigger
 * - Modifier Keys: Use ChordAction with ChordActionName
 */
USTRUCT(BlueprintType)
struct P_MEIS_API FS_InputTriggerConfig
{
    GENERATED_BODY()

    // ==================== Core Settings ====================

    /** The type of trigger to create */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
    EInputTriggerType TriggerType = EInputTriggerType::Down;

    /** Whether this trigger is active */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
    bool bEnabled = true;

    // ==================== Actuation Threshold ====================
    // Common to most triggers

    /** Input magnitude required to consider input "actuated" (0.0 - 1.0) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actuation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ActuationThreshold = 0.5f;

    // ==================== Hold Parameters ====================
    // Used when TriggerType == Hold or HoldAndRelease

    /** How long input must be held before triggering (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hold", meta = (EditCondition = "TriggerType == EInputTriggerType::Hold || TriggerType == EInputTriggerType::HoldAndRelease", ClampMin = "0.0"))
    float HoldTimeThreshold = 0.5f;

    /** If true, trigger fires only once when hold threshold is met. If false, fires every frame after. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hold", meta = (EditCondition = "TriggerType == EInputTriggerType::Hold"))
    bool bIsOneShot = false;

    /** Whether global time dilation affects hold duration */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hold", meta = (EditCondition = "TriggerType == EInputTriggerType::Hold || TriggerType == EInputTriggerType::HoldAndRelease"))
    bool bAffectedByTimeDilation = false;

    // ==================== Tap Parameters ====================
    // Used when TriggerType == Tap

    /** Maximum time between press and release to count as a "tap" (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tap", meta = (EditCondition = "TriggerType == EInputTriggerType::Tap", ClampMin = "0.0"))
    float TapReleaseTimeThreshold = 0.2f;

    // ==================== Pulse Parameters ====================
    // Used when TriggerType == Pulse

    /** Whether to trigger immediately when input first exceeds threshold */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pulse", meta = (EditCondition = "TriggerType == EInputTriggerType::Pulse"))
    bool bTriggerOnStart = true;

    /** Time between each pulse trigger (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pulse", meta = (EditCondition = "TriggerType == EInputTriggerType::Pulse", ClampMin = "0.01"))
    float PulseInterval = 0.1f;

    /** Maximum number of times to trigger (0 = unlimited) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pulse", meta = (EditCondition = "TriggerType == EInputTriggerType::Pulse", ClampMin = "0"))
    int32 PulseTriggerLimit = 0;

    // ==================== Chord Parameters ====================
    // Used when TriggerType == ChordAction

    /** Name of the action that must be triggering for this action to trigger.
     *  The referenced action must exist in the same Integration instance.
     *  Example: "IA_Modifier" must be active before other actions fire */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chord", meta = (EditCondition = "TriggerType == EInputTriggerType::ChordAction"))
    FName ChordActionName;

    // ==================== Custom Trigger Parameters ====================
    // Used when TriggerType == Custom

    /** Class of custom trigger to instantiate (must derive from UInputTrigger) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom", meta = (EditCondition = "TriggerType == EInputTriggerType::Custom"))
    TSubclassOf<UInputTrigger> CustomTriggerClass;

    // ==================== Constructors ====================

    FS_InputTriggerConfig() = default;

    /** Construct a Down trigger (default behavior) */
    static FS_InputTriggerConfig MakeDown(float Actuation = 0.5f)
    {
        FS_InputTriggerConfig Config;
        Config.TriggerType = EInputTriggerType::Down;
        Config.ActuationThreshold = Actuation;
        return Config;
    }

    /** Construct a Pressed trigger (fire once on press) */
    static FS_InputTriggerConfig MakePressed(float Actuation = 0.5f)
    {
        FS_InputTriggerConfig Config;
        Config.TriggerType = EInputTriggerType::Pressed;
        Config.ActuationThreshold = Actuation;
        return Config;
    }

    /** Construct a Released trigger (fire once on release) */
    static FS_InputTriggerConfig MakeReleased(float Actuation = 0.5f)
    {
        FS_InputTriggerConfig Config;
        Config.TriggerType = EInputTriggerType::Released;
        Config.ActuationThreshold = Actuation;
        return Config;
    }

    /** Construct a Hold trigger */
    static FS_InputTriggerConfig MakeHold(float HoldTime = 0.5f, bool bOneShot = false)
    {
        FS_InputTriggerConfig Config;
        Config.TriggerType = EInputTriggerType::Hold;
        Config.HoldTimeThreshold = HoldTime;
        Config.bIsOneShot = bOneShot;
        return Config;
    }

    /** Construct a HoldAndRelease trigger */
    static FS_InputTriggerConfig MakeHoldAndRelease(float HoldTime = 0.5f)
    {
        FS_InputTriggerConfig Config;
        Config.TriggerType = EInputTriggerType::HoldAndRelease;
        Config.HoldTimeThreshold = HoldTime;
        return Config;
    }

    /** Construct a Tap trigger */
    static FS_InputTriggerConfig MakeTap(float MaxTapTime = 0.2f)
    {
        FS_InputTriggerConfig Config;
        Config.TriggerType = EInputTriggerType::Tap;
        Config.TapReleaseTimeThreshold = MaxTapTime;
        return Config;
    }

    /** Construct a Pulse trigger */
    static FS_InputTriggerConfig MakePulse(float Interval = 0.1f, int32 Limit = 0, bool bStartImmediately = true)
    {
        FS_InputTriggerConfig Config;
        Config.TriggerType = EInputTriggerType::Pulse;
        Config.PulseInterval = Interval;
        Config.PulseTriggerLimit = Limit;
        Config.bTriggerOnStart = bStartImmediately;
        return Config;
    }

    /** Construct a ChordAction trigger */
    static FS_InputTriggerConfig MakeChord(FName RequiredActionName)
    {
        FS_InputTriggerConfig Config;
        Config.TriggerType = EInputTriggerType::ChordAction;
        Config.ChordActionName = RequiredActionName;
        return Config;
    }
};