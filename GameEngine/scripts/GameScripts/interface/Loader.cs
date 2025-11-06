using System.Reflection;
using System.Runtime.Loader;
using System.Runtime.InteropServices;

public class ScriptLoader
{
    private static AssemblyLoadContext? context = null;
    
    // Keepts reference to loaded script assembly (in this case GameScripts.dll)
    private static Assembly? scriptAssembly = null;

    public static void LoadScripts(string path)
    {
        // If there is already a context,
        // unload the context before loading new scripts
        if (context != null)
            UnloadScripts();

        // Create AssemblyLoadContext by loading assembly
        context = new AssemblyLoadContext("HotReloadContext", true);
        scriptAssembly = context.LoadFromAssemblyPath(path);

        Console.WriteLine($"[ScriptLoader] Loaded scripts from: {path}");
    }

    public static void UnloadScripts()
    {
        if (context != null)
        {
            Console.WriteLine("[ScriptLoader] Unloading scripts...");

            // Unload current context
            context.Unload();

            // Null out to enable GC to clean up
            context = null;
            scriptAssembly = null;

            // Enable GC cleanup
            GC.Collect();
            GC.WaitForPendingFinalizers();

            Console.WriteLine("[ScriptLoader] Scripts unloaded successfully.");
        }
    }

    public static void ReloadScripts(string path)
    {
        UnloadScripts();

        LoadScripts(path);

        Console.WriteLine($"[ScriptLoader] Reloaded scripts from: {path}");
    }

    // Script instance object instantiation
    public static object CreateScriptInstance(string typeName)
    {
        if (scriptAssembly == null)
            throw new InvalidOperationException("No script assembly loaded!");

        // Look up type definition from the loaded assembly
        Type? type = scriptAssembly.GetType(typeName);

        // Ensure that type does exist in assembly
        if (type == null)
            throw new TypeLoadException($"Type '{typeName}' not found in the loaded assembly!");
        
        object? instance = Activator.CreateInstance(type);
        if (instance == null)
            throw new InvalidOperationException($"Failed to create instance of {typeName}!");

        Console.WriteLine($"[ScriptLoader] Created instance of {typeName}");
        return instance;
    }
}