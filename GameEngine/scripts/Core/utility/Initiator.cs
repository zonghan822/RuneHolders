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

            // Make sure the watcher also uses the same manager
            ScriptWatcher.SetHRManager(manager);
        }

        public static void InitSequence(string usPath, string dllPath)
        {
            // Start watching scripts
            ScriptWatcher.BeginWatching(usPath, dllPath);

            // Hot Reloading
            // Compile main DLL to a temporary dll
            HRManager!.CompileToTemp(usPath, dllPath);
            
            // Load temporary dll
            HRManager!.Init(dllPath);
            HRManager!.LoadTemp();
        }

        public static void ReloadSequence(string usPath, string dllPath)
        {
            // Unload
            HRManager!.Unload();

            // Recompile user scripts
            HRManager!.CompileToTemp(usPath, dllPath);

            // Reload
            HRManager!.Init(dllPath);
            HRManager!.LoadTemp();
        }

        [UnmanagedCallersOnly(EntryPoint = "Shutdown")]
        public static void ShutdownSequence()
        {
            HRManager?.Unload();
        }
    }
}