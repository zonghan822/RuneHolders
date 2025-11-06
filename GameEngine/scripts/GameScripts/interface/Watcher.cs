using System;
using System.IO;


namespace Eno
{
    public static class ScriptWatcher
    {
        private static FileSystemWatcher? watcher;
        private static string? sp;
        private static string? dp;

        // Source(s):
        // https://stackoverflow.com/questions/13014704/how-to-work-out-if-a-file-has-been-modified
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
            
            // wait for file write to finish
            System.Threading.Thread.Sleep(200);

            // Compile
            if (!string.IsNullOrEmpty(sp) || !string.IsNullOrEmpty(sp))
            {
                if (LiveCompiler.Compile(sp, dp))
                {
                    Console.WriteLine("Reloading scripts...");
                    ScriptLoader.ReloadScripts(sp);
                }
            }
            else
            {
                throw new InvalidOperationException("No User Script source path or dll path for compiling!");
            }


            // Watcher resumes watching
            watcher.EnableRaisingEvents = true;
        }
    }
}