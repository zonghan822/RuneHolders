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

        // Script instances
        // Stores a dictionary of classes under each node id
        // private Dictionary<uint, Dictionary<string, ScriptBase>> scriptInstances = new();
        private Dictionary<uint, List<ScriptBase>> scriptInstances = new();

        // Setting the assembly for the proxy
        public ScriptProxy(Assembly assembly) => SetAssembly(assembly);

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

        // Registering script instance
        public void RegisterScriptInstance(string className, uint entityID)
        {
            if (!typeCache.TryGetValue(className, out var type))
            {
                Console.WriteLine($"Class '{className}' not found!");
                return;
            }

            // Create new instance of the script class
            ScriptBase instance;
            try 
            {
                instance = (ScriptBase)Activator.CreateInstance(type)!;
            }
            catch(Exception e)
            {
                Console.WriteLine($"Failed to create a new instance of '{className}' : {e.Message}");
                return;
            }

            // Assign entity ID to script instance
            instance.ID = entityID;

            if (!scriptInstances.TryGetValue(entityID, out var list))
            {
                list = new List<ScriptBase>();
                scriptInstances[entityID] = list;
            }

            list.Add(instance);
        }

        // Removing script instance
        public void DeregisterScriptInstance(uint entityID)
        {
            if (scriptInstances.TryGetValue(entityID, out var list))
            {
                scriptInstances.Remove(entityID);
            }
        }

        // For static calling
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

        // For per-instance calling
        public void Call(string methodName, uint entityID)
        {
            // Get the value of the class name, where the call/method resides
            if (!scriptInstances.TryGetValue(entityID, out var list))
            {
                Console.WriteLine($"[ScriptProxy] Entity ID '{entityID}' not found in assembly.");
                return;
            }

            // Specify method to be called
            foreach (var script in list)
            {
                var method = script.GetType().GetMethod(methodName, BindingFlags.Public | BindingFlags.Instance);
                if (method == null)
                {
                    Console.WriteLine($"[ScriptProxy] Method '{methodName}' not found in class '{script.GetType().Name}'.");
                    return;
                }

                method.Invoke(script, null);
            }
        }

        public void PrintScriptInstances()
        {
            Console.WriteLine("===== Script Instances =====");

            foreach (var kvp in scriptInstances)
            {
                uint entityID = kvp.Key;
                List<ScriptBase> scripts = kvp.Value;

                Console.WriteLine($"Entity ID: {entityID}, Script Count: {scripts.Count}");

                foreach (var script in scripts)
                {
                    // Print the runtime class name
                    Console.WriteLine($"  - Script Class: {script.GetType().Name}, ID: {script.ID}");
                }
            }

            Console.WriteLine("============================");
        }
    }
}
