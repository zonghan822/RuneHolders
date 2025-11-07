using System;
using System.IO;
using System.Reflection;
using System.Runtime.Loader;
using System.Threading;

namespace Eno
{
    public class HotReloadManager
    {
        private AssemblyLoadContext? loadContext;
        private Assembly? loadedAssembly;
        private WeakReference? loadContextWeak;

        public ScriptProxy? Proxy { get; private set; }

        // Path to the temp DLL
        private string? mainDll;
        private string? tempDllPath;
        private string? tempDll;

        public void Init(string dll)
        {
            loadContext = new AssemblyLoadContext("HotReloadContext", true);
            loadContextWeak = new WeakReference(loadContext);
            
            mainDll = dll;
            Console.WriteLine($"Main DLL: { mainDll }");
        }

        public void LoadTemp()
        {
            // Load temporary DLL
            Load(tempDllPath!);

            // Once that DLL has been loaded, assign assembly
            if (loadedAssembly == null)
                throw new InvalidOperationException("Failed to load assembly!");
            
            Proxy.SetAssembly(loadedAssembly?? throw new InvalidOperationException("Failed to load assembly!"));
            ProxyEntry.SetProxy(Proxy);
        }

        public void Load(string path)
        {
            Console.WriteLine($"Loading : { path }");

            // Read all bytes from the original dll
            // and these bytes will be streamed into the new temp dll
            // The same is done for pdb
            byte[] bytes = File.ReadAllBytes(path);
            string pdbPath = Path.ChangeExtension(path, ".pdb");
            byte[]? pdb = File.Exists(pdbPath) ? File.ReadAllBytes(pdbPath) : null;

            // Creating a new AssemblyLoadContext 
            // I want to potentially replace this new assembly at runtime so
            // I am setting isCollectible to true
            loadContext = new AssemblyLoadContext("HotReloadContext", isCollectible: true);
            loadContextWeak = new WeakReference(loadContext);

            // Set up assembly resolution
            // If the temp DLL references the main DLL GameScripts,
            // the main DLL will be loaded from disk
            loadContext.Resolving += (ctx, name) =>
            {
                // First, check temp DLL folder
                string depPath = Path.Combine(Path.GetDirectoryName(path)!, name.Name + ".dll");
                if (File.Exists(depPath))
                    return ctx.LoadFromAssemblyPath(depPath);

                // Then check main DLL path
                if (name.Name == "GameScripts" && File.Exists(mainDll))
                    return ctx.LoadFromAssemblyPath(mainDll);

                return null;
            };

            // Wrap the bytes we got the DLL previously in a MemoryStream
            // to be used to load the new temp assembly
            var stream = new MemoryStream(bytes);
            MemoryStream? pdbStream = pdb != null ? new MemoryStream(pdb) : null;

            loadedAssembly = pdbStream != null
                ? loadContext.LoadFromStream(stream, pdbStream)
                : loadContext.LoadFromStream(stream);
            
            // Assign proxy to new loaded assembly
            Proxy = new ScriptProxy(loadedAssembly);

            Console.WriteLine("Loaded assembly via memory stream (file unlocked)!");
        }

        public void Unload()
        {
            if (loadContext == null)
                return;
            
            loadedAssembly = null;

            loadContext.Unload();
            loadContext = null;

            // Cleanup
            for (int i = 0; i < 10; ++i)
            {
                GC.Collect();
                GC.WaitForPendingFinalizers();

                if (loadContextWeak?.IsAlive == false)
                    break;
                
                Thread.Sleep(50);
            }

            if (loadContextWeak?.IsAlive == true)
                Console.WriteLine("WARNING: AssemblyLoadContext still alive (someone holds a reference).");
            else
                Console.WriteLine("Unloaded successfully.");
        }

        public string? CompileToTemp(string usPath, string dllPath)
        {
            string dir = Path.GetDirectoryName(dllPath!)!;
            if (string.IsNullOrEmpty(tempDll))
            {
                // Create temp dll when it is not available
                tempDll = Path.Combine(dir, "GameScripts_temp.dll");
            }

            string tempPdb = Path.ChangeExtension(tempDll, ".pdb");

            Console.WriteLine("Currently Compiling...");

            bool result = HotReloadCompiler.Compile(usPath, tempDll);
            if (!result)
            {
                Console.WriteLine("Compilation failed!");
                return null;
            }

            tempDllPath = tempDll;

            Console.WriteLine($"Compiled to temp: { tempDll }");
            return tempDll;
        }

        public void ReplaceDLL()
        {
            if (string.IsNullOrEmpty(tempDllPath) || string.IsNullOrEmpty(mainDll))
                return;

            try
            {
                if (File.Exists(mainDll))
                    File.Delete(mainDll);

                File.Move(tempDll!, mainDll!);

                Console.WriteLine("Main DLL replaced successfully");
                tempDllPath = null;
            }
            catch
            {
                // If delete fails because the file is locked, we still proceed:
                Console.WriteLine("Could not remove old DLL — still locked. Loading new DLL directly from temp!");
                return;
            }
        }
    }
}
