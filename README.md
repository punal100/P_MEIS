# P_MEIS - Modular Enhanced Input System

A modular plugin for Unreal Engine 5 that extends the Enhanced Input System with additional functionality.

## Author

**Punal Manalan**

## Requirements

- Unreal Engine 5.5+
- Enhanced Input Plugin (enabled by default in UE5)

## Installation

1. Clone or copy this plugin to your project's `Plugins/` folder
2. Regenerate project files
3. Enable the plugin in your project settings

## Folder Structure

```
P_MEIS/
├── P_MEIS.uplugin           # Plugin descriptor
├── Resources/
│   └── Icon128.png          # Plugin icon
└── Source/
    └── P_MEIS/
        ├── P_MEIS.Build.cs  # Build configuration
        ├── Base/            # Core implementation code
        ├── Private/         # Private module files
        │   └── P_MEIS.cpp
        └── Public/          # Public headers
            └── P_MEIS.h
```

## Usage

Add `"P_MEIS"` to your module's dependencies in your `.Build.cs` file:

```csharp
PublicDependencyModuleNames.AddRange(new string[] { "P_MEIS" });
```

## License

See LICENSE file for details.
