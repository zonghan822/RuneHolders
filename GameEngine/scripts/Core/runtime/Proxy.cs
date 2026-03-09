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
        private Dictionary<uint, Dictionary<string, ScriptBase>> scriptInstances = new();

        // Setting the assembly for the proxy
        public ScriptProxy(Assembly assembly) => SetAssembly(assembly);

        // Initialize or update the proxy with a new assembly
        public void SetAssembly(Assembly assembly)
        {
            currentAssembly = assembly;
            typeCache.Clear();
            // Clear old script instances to prevent stale references to types
            // from the previous assembly. Scripts will be re-registered by
            // registerAllScripts() from the C++ side after hot reload.
            scriptInstances.Clear();

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
                Console.WriteLine($"Class '{className}' cannot be found!");
                return;
            }

            // Create new instance of ScriptBase and
            // insert it into the container
            ScriptBase instance = (ScriptBase)Activator.CreateInstance(type)!;

            if (!scriptInstances.TryGetValue(entityID, out var innerDict))
            {
                innerDict = new Dictionary<string, ScriptBase>();
                scriptInstances[entityID] = innerDict;
            }

            instance.ID = entityID;
            innerDict[className] = instance;
        }

        // Removing script instance
        public void DeregisterScriptInstance(string className, uint entityID)
        {
            if (scriptInstances.TryGetValue(entityID, out var innerDict))
            {
                innerDict.Remove(className);

                // Remove the entity entry if it has no more scripts
                if (innerDict.Count == 0)
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
        public void Call(string className, string methodName, uint entityID)
        {
            // Get the value of the class name, where the call/method resides
            if (!scriptInstances.TryGetValue(entityID, out var innerDict) ||
                !innerDict.TryGetValue(className, out var script))
            {
                Console.WriteLine($"[ScriptProxy] Entity ID '{entityID}' or Class '{className}' not found in assembly.");
                return;
            }

            // Specify method to be called
            var method = script.GetType().GetMethod(methodName, BindingFlags.Public | BindingFlags.Instance);
            
            if (method == null)
            {
                Console.WriteLine($"[ScriptProxy] Method '{methodName}' not found in class '{className}'.");
                return;
            }

            method.Invoke(script, null);
        }
    }
}
