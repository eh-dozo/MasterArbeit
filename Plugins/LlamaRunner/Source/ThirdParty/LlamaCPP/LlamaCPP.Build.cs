using System.IO;
using UnrealBuildTool;

public class LlamaCPP : ModuleRules
{
    public LlamaCPP(ReadOnlyTargetRules Target) : base(Target)
    {
        Type = ModuleType.External;

        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Include"));

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "Lib", "llama.lib"));
            PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "Lib", "ggml.lib"));
            PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "Lib", "ggml-base.lib"));
            PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "Lib", "ggml-cpu.lib"));
            PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "Lib", "ggml-cuda.lib"));
            PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "Lib", "llava_shared.lib"));
        }
    }
}