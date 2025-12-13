/*
 * @Author: Punal Manalan
 * @Description: Modular Enhanced Input System - Build.cs.
 * @Date: 06/12/2025
 */

using UnrealBuildTool;
using System.IO;

// Build configuration for the P_MEIS runtime module
public class P_MEIS : ModuleRules
{
	public P_MEIS(ReadOnlyTargetRules Target) : base(Target)
	{
		// Use explicit precompiled headers for better performance
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		// Get the module directory path
		string ModulePath = ModuleDirectory;

		// Public include paths - these are exposed to other modules
		PublicIncludePaths.AddRange(
			new string[] {
				Path.Combine(ModulePath, "Public"),
				Path.Combine(ModulePath, "Base"),
			}
			);

		// Private include paths - internal to this module only
		PrivateIncludePaths.AddRange(
			new string[] {
				Path.Combine(ModulePath, "Private"),
				Path.Combine(ModulePath, "Base"),
			}
			);

		// Public dependencies - modules that are required by public headers
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"InputCore",
				"EnhancedInput",
				"Projects",
				"Json",
				"JsonUtilities",
			}
			);

		// Private dependencies - modules only needed for implementation
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Slate",
				"SlateCore",
			}
			);

		// Dynamically loaded modules - loaded at runtime when needed
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
			}
			);
	}
}
