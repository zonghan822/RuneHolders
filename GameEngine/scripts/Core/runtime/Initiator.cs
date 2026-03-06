using System.Reflection;
using System.Runtime.Loader;
using System.Runtime.InteropServices;

namespace Eno
{
    public static class Initiator
    {
        private static IHotReloadManager? HRManager;

        public static void SetHRManager(IHotReloadManager manager)
        {
            HRManager = manager;
        }

        public static void InitSequence(string usPath, string dllPath)
        {
            // Hot Reloading
            // Compile main DLL to a temporary dll
            var path = HRManager!.CompileToTemp(usPath, dllPath);
            if (path == null)
            {
                // Compilation failed
                Console.WriteLine("Hot reload to be skipped due to failed compilation");
                return;
            }

            // Load temporary dll
            HRManager!.Init(dllPath);
            HRManager!.Load(path);
        }

        public static void ReloadSequence(string usPath, string dllPath)
        {
             // Unload
            HRManager!.Unload();

            // Recompile user scripts
            var path = HRManager!.CompileToTemp(usPath, dllPath);
            if (path == null)
            {
                // Compilation failed
                Console.WriteLine("Hot reload to be skipped due to failed compilation");
                return;
            }

            // Reload
            HRManager!.Init(dllPath);
            HRManager!.Load(path);
        }

        [UnmanagedCallersOnly(EntryPoint = "Shutdown")]
        public static void ShutdownSequence()
        {
            HRManager?.Unload();
        }
    }
}