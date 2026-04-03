using System;
using System.IO;
using System.Linq;
using System.Reflection;
using Microsoft.CodeAnalysis;
using Microsoft.CodeAnalysis.CSharp;

// Compilation with Roslyn!
// Source(s): 
// https://blog.jetbrains.com/dotnet/2021/12/02/how-rider-hot-reload-works-under-the-hood/
// https://learn.microsoft.com/en-us/dotnet/csharp/roslyn-sdk/get-started/syntax-analysis
// https://learn.microsoft.com/en-us/dotnet/api/microsoft.codeanalysis.csharp.csharpcompilation.create?view=roslyn-dotnet-4.14.0

public static class HotReloadCompiler
{
    // TO create a new compilation (using CSharpCompilation.Create) we need
    //      - assembly name
    //      - syntax trees (trees with the source code for the new compilation)
    //      - references (for new compilation)
    //      - options (compiler options)
    public static bool Compile(string userScriptsFolder, string dllPath, string? refDir = null)
    {
        Console.WriteLine($"DLLPATH: {dllPath}");
        // Retrieve names/paths of .cs files from the specified folder
        var scripts = Directory.GetFiles(userScriptsFolder, "*.cs", SearchOption.AllDirectories).ToArray();

        // Retrieve parsed representation of a .cs file,
        // in the form of a Roslyn syntax tree
        var newST = scripts.Select(file =>
        {
            using (var fs = new FileStream(file, FileMode.Open, FileAccess.Read, FileShare.ReadWrite))
            using (var sr = new StreamReader(fs))
            {
                return CSharpSyntaxTree.ParseText(sr.ReadToEnd());
            }
        }).ToList();

        // Get all assembles loaded in app domain
        // Filter out assemblies that are dynamic or have no physical file
        // Convert assembly into Roslyn MetaDataReferences
        //
        // These references are used by the C# compiler to resolve types
        // and namespaces when compiling user scripts, like a linker
        var assemblies = AppDomain.CurrentDomain
            .GetAssemblies()
            .Where(a => !a.IsDynamic && !string.IsNullOrEmpty(a.Location))
            .Select(a => MetadataReference.CreateFromFile(a.Location))
            .ToList();

        // Reference DLLs live next to the original DLL, not the temp output
        string searchDir = refDir ?? Path.GetDirectoryName(dllPath)!;
        var mainDllPath = Path.Combine(searchDir, "GameScripts.dll");
        if (!File.Exists(mainDllPath))
            throw new FileNotFoundException("Main DLL not found", mainDllPath);
        var coreDllPath = Path.Combine(searchDir, "Core.dll");
        if (!File.Exists(coreDllPath))
            throw new FileNotFoundException("Core DLL not found", coreDllPath);

        assemblies.Add(MetadataReference.CreateFromFile(mainDllPath));

        var loadedCore = AppDomain.CurrentDomain.GetAssemblies().
            FirstOrDefault(a => a.GetName().Name == "Core");
            
        if (loadedCore != null)
            assemblies.Add(MetadataReference.CreateFromFile(loadedCore.Location));
        else
            assemblies.Add(MetadataReference.CreateFromFile(coreDllPath));

        // Creating the new Roslyn compilation with all of our
        // previously created components!
        //
        // We want a new DLL to load during runtime
        // so options will utilize OutputKind.DynamicallyLinkedLibrary
        var compilation = CSharpCompilation.Create(
            assemblyName: "GameScripts_temp",
            syntaxTrees: newST,
            references: assemblies,
            options: new CSharpCompilationOptions(OutputKind.DynamicallyLinkedLibrary)
        );

        // Emit the new compilation
        // or to generate the new DLL with the predefined information!
        // Subsequently, indicate success with green or failure with red
        var result = compilation.Emit(dllPath);
        if (!result.Success)
        {
            Console.ForegroundColor = ConsoleColor.Red;
            foreach(var diag in result.Diagnostics)
                Console.WriteLine(diag.ToString());
            Console.ResetColor();
            return false;
        }

        Console.ForegroundColor = ConsoleColor.Green;
        Console.WriteLine("Scripts compiled successfully!");
        Console.ResetColor();
        return true;
    }
}