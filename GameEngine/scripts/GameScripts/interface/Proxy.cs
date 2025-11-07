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
            if (currentAssembly == null)
            {
                Console.WriteLine("[ScriptProxy] No assembly loaded.");
                return;
            }

            if (!typeCache.TryGetValue(className, out var type))
            {
                Console.WriteLine($"[ScriptProxy] Class '{className}' not found in assembly.");
                foreach (var t in typeCache.Keys)
                    Console.WriteLine($"  {t}");
                return;
            }

            var method = type.GetMethod(methodName, BindingFlags.Public | BindingFlags.Static);
            if (method == null)
            {
                Console.WriteLine($"[ScriptProxy] Method '{methodName}' not found in class '{className}'.");
                foreach (var m in type.GetMethods(BindingFlags.Public | BindingFlags.Static))
                    Console.WriteLine($"  {m.Name}");
                return;
            }

            Console.WriteLine($"[ScriptProxy] Invoking {className}.{methodName}...");
            method.Invoke(null, null);
        }
    }
}
