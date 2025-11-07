using System.Reflection;
using System.Runtime.Loader;
using System.Runtime.InteropServices;

namespace Eno
{
    public static class Initiator
    {
        private static HotReloadManager? HRManager;

        [UnmanagedCallersOnly(EntryPoint = "Init")]
        public static void InitSequence(IntPtr pUSPath, IntPtr pDLLPath)
        {
            // Conversion to managed string
            string? usPath = Marshal.PtrToStringUni(pUSPath);
            string? dllPath = Marshal.PtrToStringUni(pDLLPath);

            if (string.IsNullOrEmpty(usPath) || string.IsNullOrEmpty(dllPath))
                throw new InvalidOperationException("User scripts or Assembly DLL cannot be found in Initiator Sequence!");

            // Hot reload manager instance creation
            HRManager = new HotReloadManager();

            // Start watching scripts
            ScriptWatcher.SetHRManager(HRManager);
            ScriptWatcher.BeginWatching(usPath, dllPath);

            // Hot Reloading
            // Compile main DLL to a temporary dll
            HRManager.CompileToTemp(usPath, dllPath);
           
            // Load temporary dll
            HRManager.Init(dllPath);
            HRManager.LoadTemp();
        }

        [UnmanagedCallersOnly(EntryPoint = "Reload")]
        public static void ReloadSequence(IntPtr pUSPath, IntPtr pDLLPath)
        {
            // Conversion to managed string
            string? usPath = Marshal.PtrToStringUni(pUSPath);
            string? dllPath = Marshal.PtrToStringUni(pDLLPath);

            if (string.IsNullOrEmpty(usPath) || string.IsNullOrEmpty(dllPath))
                throw new InvalidOperationException("User scripts or Assembly DLL cannot be found in Initiator Sequence!");

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