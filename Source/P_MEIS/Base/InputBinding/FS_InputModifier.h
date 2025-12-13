/*
 * @Author: Punal Manalan
 * @Description: Input Modifier Configuration for adjusting input values
 *               Comprehensive struct that supports all UE5 Enhanced Input modifiers
 *               Can be used with both Action-level and Key Mapping-level modifiers
 * @Date: 06/12/2025
 * @Version: 2.0 - Section 0.9 Expanded with full UE5 modifier support
 */

#pragma once

#include "CoreMinimal.h"
#include "InputModifiers.h" // For EDeadZoneType, EInputAxisSwizzle
#include "FS_InputModifier.generated.h"

// Forward declarations
class UInputModifier;

// ==================== Input Modifier Types ====================
// Maps to UE5 Enhanced Input modifier classes

/**
 * Input Modifier Types - Maps to UE5 Enhanced Input modifier classes
 *
 * Core Modifiers: DeadZone, Scale, Negate
 * Axis Manipulation: Swizzle
 * Response Curves: ResponseCurveExponential, ResponseCurveUser
 * Smoothing: Smooth, SmoothDelta
 * Scaling: FOVScaling, ScaleByDeltaTime
 * Space Conversion: ToWorldSpace
 * Legacy/Utility: Clamp, ResponseCurve
 * Custom: User-defined modifier class
 */
UENUM(BlueprintType)
enum class EInputModifierType : uint8
{
    DeadZone = 0 UMETA(DisplayName = "Dead Zone", ToolTip = "Ignore input values below threshold, remap above to 0-1"),
    Scale = 1 UMETA(DisplayName = "Scale / Sensitivity", ToolTip = "Multiply input by a factor per axis"),
    Negate = 2 UMETA(DisplayName = "Negate / Invert", ToolTip = "Invert input values per axis (e.g., invert Y look)"),
    Swizzle = 3 UMETA(DisplayName = "Swizzle Axis", ToolTip = "Swap or reorder axis components (e.g., X<->Y)"),
    ResponseCurveExponential = 4 UMETA(DisplayName = "Response Curve (Exponential)", ToolTip = "Apply exponential curve per axis"),
    ResponseCurveUser = 5 UMETA(DisplayName = "Response Curve (Custom)", ToolTip = "Apply custom curve asset"),
    Smooth = 6 UMETA(DisplayName = "Smooth", ToolTip = "Smooth input over multiple frames"),
    SmoothDelta = 7 UMETA(DisplayName = "Smooth Delta", ToolTip = "Smooth normalized delta between frames"),
    FOVScaling = 8 UMETA(DisplayName = "FOV Scaling", ToolTip = "Scale input based on field of view"),
    ScaleByDeltaTime = 9 UMETA(DisplayName = "Scale By Delta Time", ToolTip = "Multiply input by frame delta time"),
    ToWorldSpace = 10 UMETA(DisplayName = "To World Space", ToolTip = "Convert input vector to world space"),
    Clamp = 11 UMETA(DisplayName = "Clamp", ToolTip = "Limit input to specified range (custom implementation)"),
    ResponseCurve = 12 UMETA(DisplayName = "Response Curve (Legacy)", ToolTip = "Legacy response curve - use ResponseCurveExponential"),
    Custom = 255 UMETA(DisplayName = "Custom", ToolTip = "User-defined custom modifier class")
};

// ==================== Smooth Delta Method Types ====================
// Maps to ENormalizeInputSmoothingType from UE5

UENUM(BlueprintType)
enum class EP_MEIS_SmoothingMethod : uint8
{
    Lerp = 0 UMETA(DisplayName = "Lerp"),
    InterpTo = 1 UMETA(DisplayName = "Interp To"),
    InterpConstantTo = 2 UMETA(DisplayName = "Interp Constant To"),
    InterpCircularIn = 3 UMETA(DisplayName = "Interp Circular In"),
    InterpCircularOut = 4 UMETA(DisplayName = "Interp Circular Out"),
    InterpCircularInOut = 5 UMETA(DisplayName = "Interp Circular In/Out"),
    InterpEaseIn = 6 UMETA(DisplayName = "Interp Ease In"),
    InterpEaseOut = 7 UMETA(DisplayName = "Interp Ease Out"),
    InterpEaseInOut = 8 UMETA(DisplayName = "Interp Ease In/Out"),
    InterpExpoIn = 9 UMETA(DisplayName = "Interp Expo In"),
    InterpExpoOut = 10 UMETA(DisplayName = "Interp Expo Out"),
    InterpExpoInOut = 11 UMETA(DisplayName = "Interp Expo In/Out"),
    InterpSinIn = 12 UMETA(DisplayName = "Interp Sin In"),
    InterpSinOut = 13 UMETA(DisplayName = "Interp Sin Out"),
    InterpSinInOut = 14 UMETA(DisplayName = "Interp Sin In/Out")
};

// ==================== Dead Zone Type Wrapper ====================
// Blueprint-friendly wrapper for EDeadZoneType

UENUM(BlueprintType)
enum class EP_MEIS_DeadZoneType : uint8
{
    Axial = 0 UMETA(DisplayName = "Axial", ToolTip = "Apply dead zone to each axis independently"),
    Radial = 1 UMETA(DisplayName = "Radial (Smoothed)", ToolTip = "Apply dead zone to magnitude, smooth transition"),
    UnscaledRadial = 2 UMETA(DisplayName = "Radial (Unscaled)", ToolTip = "Apply dead zone to magnitude, no smoothing")
};

// ==================== Swizzle Axis Order Wrapper ====================
// Blueprint-friendly wrapper for EInputAxisSwizzle

UENUM(BlueprintType)
enum class EP_MEIS_SwizzleOrder : uint8
{
    YXZ = 0 UMETA(DisplayName = "YXZ (Swap X<->Y)", ToolTip = "Swap X and Y axis - common for 1D to Y axis"),
    ZYX = 1 UMETA(DisplayName = "ZYX (Swap X<->Z)", ToolTip = "Swap X and Z axis"),
    XZY = 2 UMETA(DisplayName = "XZY (Swap Y<->Z)", ToolTip = "Swap Y and Z axis"),
    YZX = 3 UMETA(DisplayName = "YZX (Reorder Y first)", ToolTip = "Reorder: Y->X, Z->Y, X->Z"),
    ZXY = 4 UMETA(DisplayName = "ZXY (Reorder Z first)", ToolTip = "Reorder: Z->X, X->Y, Y->Z")
};

// ==================== Legacy Struct (Backward Compatibility) ====================

/**
 * Legacy FS_InputModifier - kept for backward compatibility
 * @deprecated Use FS_InputModifierConfig for new code
 */
USTRUCT(BlueprintType)
struct FS_InputModifier
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Modifier")
    EInputModifierType ModifierType = EInputModifierType::DeadZone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Modifier")
    float DeadZoneValue = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Modifier")
    float ScaleValue = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Modifier")
    FVector2D ClampRange = FVector2D(-1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Modifier")
    bool bInvert = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Modifier")
    bool bEnabled = true;
};

// ==================== Comprehensive Modifier Configuration ====================

/**
 * FS_InputModifierConfig - Comprehensive input modifier configuration
 *
 * Supports ALL UE5 Enhanced Input modifier types with full parameter control.
 * Use this struct when creating dynamic Input Actions or adding modifiers at runtime.
 *
 * Example Usage:
 * - Create DeadZone: Set ModifierType to DeadZone, configure DeadZoneLower/Upper/Type
 * - Create Sensitivity: Set ModifierType to Scale, configure ScaleVector
 * - Create Invert Y: Set ModifierType to Negate, set bNegateY = true
 */
USTRUCT(BlueprintType)
struct P_MEIS_API FS_InputModifierConfig
{
    GENERATED_BODY()

    // ==================== Core Settings ====================

    /** The type of modifier to create */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modifier")
    EInputModifierType ModifierType = EInputModifierType::DeadZone;

    /** Whether this modifier is active */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modifier")
    bool bEnabled = true;

    // ==================== Dead Zone Parameters ====================
    // Used when ModifierType == DeadZone

    /** Threshold below which input is ignored (0.0 - 1.0) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dead Zone", meta = (EditCondition = "ModifierType == EInputModifierType::DeadZone", ClampMin = "0.0", ClampMax = "1.0"))
    float DeadZoneLower = 0.2f;

    /** Threshold above which input is clamped to 1.0 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dead Zone", meta = (EditCondition = "ModifierType == EInputModifierType::DeadZone", ClampMin = "0.0", ClampMax = "1.0"))
    float DeadZoneUpper = 1.0f;

    /** How the dead zone is calculated */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dead Zone", meta = (EditCondition = "ModifierType == EInputModifierType::DeadZone"))
    EP_MEIS_DeadZoneType DeadZoneType = EP_MEIS_DeadZoneType::Radial;

    // ==================== Scale Parameters ====================
    // Used when ModifierType == Scale

    /** Scale factor per axis (sensitivity) - e.g., (2.0, 2.0, 1.0) doubles X/Y sensitivity */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scale", meta = (EditCondition = "ModifierType == EInputModifierType::Scale"))
    FVector ScaleVector = FVector(1.0f, 1.0f, 1.0f);

    // ==================== Negate Parameters ====================
    // Used when ModifierType == Negate

    /** Negate (invert) X axis */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Negate", meta = (EditCondition = "ModifierType == EInputModifierType::Negate"))
    bool bNegateX = false;

    /** Negate (invert) Y axis - common for "Invert Look" option */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Negate", meta = (EditCondition = "ModifierType == EInputModifierType::Negate"))
    bool bNegateY = false;

    /** Negate (invert) Z axis */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Negate", meta = (EditCondition = "ModifierType == EInputModifierType::Negate"))
    bool bNegateZ = false;

    // ==================== Swizzle Parameters ====================
    // Used when ModifierType == Swizzle

    /** How to reorder axis components */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swizzle", meta = (EditCondition = "ModifierType == EInputModifierType::Swizzle"))
    EP_MEIS_SwizzleOrder SwizzleOrder = EP_MEIS_SwizzleOrder::YXZ;

    // ==================== Response Curve Parameters ====================
    // Used when ModifierType == ResponseCurveExponential

    /** Exponential curve factor per axis - values > 1 make input less sensitive near center */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Response Curve", meta = (EditCondition = "ModifierType == EInputModifierType::ResponseCurveExponential"))
    FVector CurveExponent = FVector(1.0f, 1.0f, 1.0f);

    // ==================== Smooth Delta Parameters ====================
    // Used when ModifierType == SmoothDelta

    /** Smoothing interpolation method */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Smooth Delta", meta = (EditCondition = "ModifierType == EInputModifierType::SmoothDelta"))
    EP_MEIS_SmoothingMethod SmoothingMethod = EP_MEIS_SmoothingMethod::Lerp;

    /** Smoothing speed/alpha (0 = instant jump to target) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Smooth Delta", meta = (EditCondition = "ModifierType == EInputModifierType::SmoothDelta", ClampMin = "0.0"))
    float SmoothingSpeed = 0.5f;

    /** Easing exponent for Ease smoothing methods */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Smooth Delta", meta = (EditCondition = "ModifierType == EInputModifierType::SmoothDelta"))
    float EasingExponent = 2.0f;

    // ==================== FOV Scaling Parameters ====================
    // Used when ModifierType == FOVScaling
    // Note: UInputModifierFOVScaling uses engine FOV settings automatically

    /** FOV scale multiplier (for custom implementations) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FOV Scaling", meta = (EditCondition = "ModifierType == EInputModifierType::FOVScaling"))
    float FOVScale = 1.0f;

    // ==================== Clamp Parameters ====================
    // Used when ModifierType == Clamp (custom implementation)

    /** Minimum clamp value per axis */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clamp", meta = (EditCondition = "ModifierType == EInputModifierType::Clamp"))
    FVector ClampMin = FVector(0.0f, 0.0f, 0.0f);

    /** Maximum clamp value per axis */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clamp", meta = (EditCondition = "ModifierType == EInputModifierType::Clamp"))
    FVector ClampMax = FVector(1.0f, 1.0f, 1.0f);

    // ==================== Custom Modifier Parameters ====================
    // Used when ModifierType == Custom

    /** Class of custom modifier to instantiate (must derive from UInputModifier) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom", meta = (EditCondition = "ModifierType == EInputModifierType::Custom"))
    TSubclassOf<UInputModifier> CustomModifierClass;

    // ==================== Constructors ====================

    FS_InputModifierConfig() = default;

    /** Construct a DeadZone modifier */
    static FS_InputModifierConfig MakeDeadZone(float Lower = 0.2f, float Upper = 1.0f, EP_MEIS_DeadZoneType Type = EP_MEIS_DeadZoneType::Radial)
    {
        FS_InputModifierConfig Config;
        Config.ModifierType = EInputModifierType::DeadZone;
        Config.DeadZoneLower = Lower;
        Config.DeadZoneUpper = Upper;
        Config.DeadZoneType = Type;
        return Config;
    }

    /** Construct a Scale/Sensitivity modifier */
    static FS_InputModifierConfig MakeScale(FVector Scale)
    {
        FS_InputModifierConfig Config;
        Config.ModifierType = EInputModifierType::Scale;
        Config.ScaleVector = Scale;
        return Config;
    }

    /** Construct a uniform Scale modifier */
    static FS_InputModifierConfig MakeUniformScale(float Scale)
    {
        return MakeScale(FVector(Scale, Scale, Scale));
    }

    /** Construct a Negate modifier for inverting axes */
    static FS_InputModifierConfig MakeNegate(bool bX = false, bool bY = false, bool bZ = false)
    {
        FS_InputModifierConfig Config;
        Config.ModifierType = EInputModifierType::Negate;
        Config.bNegateX = bX;
        Config.bNegateY = bY;
        Config.bNegateZ = bZ;
        return Config;
    }

    /** Construct Invert Y modifier (common for look controls) */
    static FS_InputModifierConfig MakeInvertY()
    {
        return MakeNegate(false, true, false);
    }

    /** Construct a Swizzle modifier */
    static FS_InputModifierConfig MakeSwizzle(EP_MEIS_SwizzleOrder Order = EP_MEIS_SwizzleOrder::YXZ)
    {
        FS_InputModifierConfig Config;
        Config.ModifierType = EInputModifierType::Swizzle;
        Config.SwizzleOrder = Order;
        return Config;
    }

    /** Construct an Exponential Response Curve modifier */
    static FS_InputModifierConfig MakeResponseCurve(FVector Exponent)
    {
        FS_InputModifierConfig Config;
        Config.ModifierType = EInputModifierType::ResponseCurveExponential;
        Config.CurveExponent = Exponent;
        return Config;
    }
};
