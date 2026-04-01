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
            // Detect if running as editor or shipped game
            string exeName = Path.GetFileNameWithoutExtension(Environment.ProcessPath ?? "");
            bool isEditor = exeName.Equals("Editor", StringComparison.OrdinalIgnoreCase);

            if (isEditor)
            {
                // Hot Reloading — compile to a temporary dll
                var path = HRManager!.CompileToTemp(usPath, dllPath);
                if (path == null)
                {
                    Console.WriteLine("Hot reload to be skipped due to failed compilation");
                    return;
                }

                HRManager!.Init(dllPath);
                HRManager!.Load(path);
            }
            else
            {
                // Shipped game — load the pre-built GameScripts.dll directly
                string mainDll = Path.Combine(Path.GetDirectoryName(dllPath)!, "GameScripts.dll");
                HRManager!.Init(dllPath);
                HRManager!.Load(mainDll);
            }
        }

        public static void ReloadSequence(string usPath, string dllPath)
        {
            // Hot reload is editor-only — skip entirely for the shipped game
            string exeName = Path.GetFileNameWithoutExtension(Environment.ProcessPath ?? "");
            if (!exeName.Equals("Editor", StringComparison.OrdinalIgnoreCase))
                return;

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