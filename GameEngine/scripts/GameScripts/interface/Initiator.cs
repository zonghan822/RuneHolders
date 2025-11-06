using System.Reflection;
using System.Runtime.Loader;
using System.Runtime.InteropServices;

namespace Eno
{
    public static class Initiator
    {
        [UnmanagedCallersOnly(EntryPoint = "Init")]
        public static void InitSequence(IntPtr pUSPath, IntPtr pDLLPath)
        {
            // Conversion to managed string
            string? usPath = Marshal.PtrToStringUni(pUSPath);
            string? dllPath = Marshal.PtrToStringUni(pDLLPath);

            if (string.IsNullOrEmpty(usPath) || string.IsNullOrEmpty(dllPath))
                throw new InvalidOperationException("User scripts or Assembly DLL cannot be found in Initiator Sequence!");

            // Start watching scripts
            ScriptWatcher.BeginWatching(usPath, dllPath);

            // Load scripts
            ScriptLoader.LoadScripts(dllPath);
        }

        [UnmanagedCallersOnly(EntryPoint = "Reload")]
        public static void ReloadSequence(IntPtr pUSPath, IntPtr pDLLPath)
        {
            // Conversion to managed string
            string? usPath = Marshal.PtrToStringUni(pUSPath);
            string? dllPath = Marshal.PtrToStringUni(pDLLPath);

            if (string.IsNullOrEmpty(usPath) || string.IsNullOrEmpty(dllPath))
                throw new InvalidOperationException("User scripts or Assembly DLL cannot be found in Initiator Sequence!");

            // Recompile user scripts
            LiveCompiler.Compile(usPath, dllPath);

            // Reload
            ScriptLoader.ReloadScripts(dllPath);
        }

        [UnmanagedCallersOnly(EntryPoint = "Shutdown")]
        public static void ShutdownSequence()
        {
            ScriptLoader.UnloadScripts();
        }
    }
}