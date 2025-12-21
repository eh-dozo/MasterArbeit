using System.IO;
using UnrealBuildTool;

public class LlamaCPP : ModuleRules
{
    public LlamaCPP(ReadOnlyTargetRules Target) : base(Target)
    {
        Type = ModuleType.External;
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        bUseRTTI = true;
        bEnableExceptions = true;
        
        var IncludePath = Path.Combine(ModuleDirectory, "Include");
        var LibraryPath = Path.Combine(ModuleDirectory, "Lib");
        var BinaryPath = Path.Combine(ModuleDirectory, "Bin");
        
        PublicSystemIncludePaths.AddRange([IncludePath]);
        
        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            PublicDelayLoadDLLs.AddRange([
                Path.Combine(BinaryPath, "llama.dll"),
                Path.Combine(BinaryPath, "ggml.dll"),
                Path.Combine(BinaryPath, "ggml-base.dll"),
                Path.Combine(BinaryPath, "ggml-cpu.dll"),
                Path.Combine(BinaryPath, "ggml-cuda.dll"),
                Path.Combine(BinaryPath, "cudart64_12.dll"),
                Path.Combine(BinaryPath, "cublas64_12.dll"),
                Path.Combine(BinaryPath, "cublasLt64_12.dll")
            ]);
            
            PublicAdditionalLibraries.AddRange([
                Path.Combine(LibraryPath, "llama.lib"),
                Path.Combine(LibraryPath, "common.lib"),
                Path.Combine(LibraryPath, "ggml.lib"),
                Path.Combine(LibraryPath, "ggml-base.lib"),
                Path.Combine(LibraryPath, "ggml-cpu.lib"),
                Path.Combine(LibraryPath, "ggml-cuda.lib")
            ]);
            
            if (Target.bBuildEditor)
            {
                void AddRuntimeDependencyEditorBuild(string dllName)
                {
                    RuntimeDependencies.Add(
                        "$(BinaryOutputDir)/" + dllName + ".dll",
                        Path.Combine(BinaryPath, dllName + ".dll"),
                        StagedFileType.SystemNonUFS);
                }
                
                AddRuntimeDependencyEditorBuild("llama");
                AddRuntimeDependencyEditorBuild("ggml");
                AddRuntimeDependencyEditorBuild("ggml-base");
                AddRuntimeDependencyEditorBuild("ggml-cpu");
                AddRuntimeDependencyEditorBuild("ggml-cuda");
                AddRuntimeDependencyEditorBuild("cudart64_12");
                AddRuntimeDependencyEditorBuild("cublas64_12");
                AddRuntimeDependencyEditorBuild("cublasLt64_12");
            }
            else
            {
                void AddRuntimeDependencyPackage(string dllName)
                {
                    RuntimeDependencies.Add(
                        "$(TargetOutputDir)/" + dllName + ".dll",
                        Path.Combine(BinaryPath, dllName + ".dll"),
                        StagedFileType.SystemNonUFS);
                }
                
                AddRuntimeDependencyPackage("llama");
                AddRuntimeDependencyPackage("ggml");
                AddRuntimeDependencyPackage("ggml-base");
                AddRuntimeDependencyPackage("ggml-cpu");
                AddRuntimeDependencyPackage("ggml-cuda");
                AddRuntimeDependencyPackage("cudart64_12");
                AddRuntimeDependencyPackage("cublas64_12");
                AddRuntimeDependencyPackage("cublasLt64_12");
            }
        }
        
        PublicDefinitions.AddRange([
            "GGML_USE_CUDA=ON"
        ]);
        
        PublicDependencyModuleNames.AddRange([
            "Core",
            "CoreUObject",
            "Engine"
        ]);
        
        PrivateDependencyModuleNames.AddRange([
            "Projects"
        ]);
    }
}