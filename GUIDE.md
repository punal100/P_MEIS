# P_MEIS Guide

This is a quick, project-oriented checklist for using **P_MEIS** (Modular Enhanced Input System).

For the full feature/reference doc, see [README.md](./README.md).

## 1) Initialize per player

P_MEIS is **per-PlayerController**. For every local player/controller:

- Initialize the Enhanced Input integration
- Load (or create) a profile

Blueprint flow (typical):

- `Initialize Enhanced Input Integration (PC)`
- `Load Profile For Player (PC, "Default")`
  - If it fails (first run), create mappings then `Save Profile For Player`

Project note (A_MiniFootball):

- The gameplay controller provides a convenience wrapper `EnsureInputProfileReady("Default")` that registers the player, creates a `Default` template if missing, and applies it.

Binding location (recommended):

- Prefer binding gameplay actions on the **PlayerController's InputComponent** (stable across pawn possession).
- Pawn InputComponents can be re-created/changed when possession changes; if you bind on the pawn, ensure you re-bind when the input component changes.

Mapping contexts:

- Enhanced Input mapping contexts are only applied for **local players** (via `UEnhancedInputLocalPlayerSubsystem`).
- For AI controllers, you can still use P_MEIS as an action/event + injection layer, but there is no local-player mapping context.

## 2) Keep P_MEIS generic (important boundary)

- P_MEIS should **not** hardcode action names like "Sprint".
- Game/UI code defines action names and uses P_MEIS purely as a binding + injection layer.

This supports reuse across projects without coupling gameplay semantics into the plugin.

## 3) Profiles + persistence

- Profiles are persisted as JSON under: `Saved/InputProfiles/`
- Profile data can include toggle semantics via:
  - `ToggleModeActions`: list of action names that behave as toggle instead of hold
  - `ToggleActionStates`: persisted per-action ON/OFF state (map from action name → bool)

## 4) UI input injection (mobile / UMG)

If you have UI controls (virtual joystick, buttons), inject into the local player’s P_MEIS integration so gameplay listens to one unified pipeline:

- `InjectAxis2D(PC, ActionName, FVector2D)` for movement-style axes
- `InjectActionStarted/Triggered/Completed(PC, ActionName)` for button-style actions

## 5) Troubleshooting

- Nothing fires
  - Ensure you initialized integration for that PlayerController
  - Ensure the profile loaded and contains mappings for the action names you’re using
- Split-screen players share bindings
  - You’re likely applying a template globally instead of copying per-player, or using the wrong PC reference
