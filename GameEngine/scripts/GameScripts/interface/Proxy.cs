using System;
using System.Reflection;

namespace Eno
{
    public class ScriptProxy
    {
        // Holds the currently loaded assembly
        private Assembly? currentAssembly;

        // Type cache to avoid repeated lookups
        private Dictionary<string, Type> typeCache = new();

        public ScriptProxy(Assembly assembly)
        {
            SetAssembly(assembly);
        }

        // Initialize or update the proxy with a new assembly
        public void SetAssembly(Assembly assembly)
        {
            currentAssembly = assembly;
            typeCache.Clear();

            foreach (var t in assembly.GetTypes())
            {
                typeCache[t.FullName!] = t;
            }
        }

        public void Call(string className, string methodName)
        {
            // Ensure that an assembly is present
            if (currentAssembly == null) return;

            // Get the value of the class name, where the call/method resides
            if (!typeCache.TryGetValue(className, out var type))
            {
                Console.WriteLine($"[ScriptProxy] Class '{className}' not found in assembly.");
                return;
            }

            // Specify method to be called
            var method = type.GetMethod(methodName, BindingFlags.Public | BindingFlags.Static);
            if (method == null)
            {
                Console.WriteLine($"[ScriptProxy] Method '{methodName}' not found in class '{className}'.");
                return;
            }

            method.Invoke(null, null);
        }
    }
}
