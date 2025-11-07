using System;
using System.IO;


namespace Eno
{
    public class ScriptWatcher
    {
        private static FileSystemWatcher? watcher;
        private static string? sp;
        private static string? dp;

        private static HotReloadManager? HRManager;

        public static void SetHRManager(HotReloadManager manager)
        {
            HRManager = manager;
        }

        // Source(s):
        // https://stackoverflow.com/questions/13014704/how-to-work-out-if-a-file-has-been-modified
        // https://learn.microsoft.com/en-us/dotnet/api/system.io.filesystemeventargs?view=net-9.0
        public static void BeginWatching(string scriptsPath, string dllPath)
        {
            // Set up the watcher to watch for notifications
            // specifically on .cs files at the user_scripts path
            sp = scriptsPath;
            dp = dllPath;

            watcher = new FileSystemWatcher(scriptsPath, "*.cs")
            {
                NotifyFilter =    NotifyFilters.LastWrite        // When the file is modified(e.g saved)
                                | NotifyFilters.FileName,       // When file is renamed/created/deleted
                IncludeSubdirectories = true,                   // Include subfolders too
                EnableRaisingEvents = true                      // Start watcher
            };

            // Appends the OnChanged functionality to
            // each event handler of the watcher
            watcher.Changed += OnChanged;
            watcher.Created += OnChanged;
            watcher.Renamed += OnChanged;

            Console.WriteLine("Watching for script changes in " + scriptsPath);
        }

        private static void OnChanged(object sender, FileSystemEventArgs e)
        {
            Console.WriteLine($"Detected change: {e.FullPath}");

            if (watcher == null)
                throw new InvalidOperationException("No watcher available to trigger changes!");
            
            // Disable watcher while there is an event going on
            watcher.EnableRaisingEvents = false;
            
            // Wait for file to be released
            Thread.Sleep(50);
            
            // Compile to temp dll
            if (string.IsNullOrEmpty(sp) || string.IsNullOrEmpty(dp))
                throw new InvalidOperationException("Script or DLL path is null.");
                
            HRManager.CompileToTemp(sp, dp);

            // Load temp Dll
            HRManager.Init(dp!);
            HRManager.LoadTemp();

            // Watcher resumes watching
            watcher.EnableRaisingEvents = true;
        }
    }
}