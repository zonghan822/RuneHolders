using System;
using System.Runtime.InteropServices;

namespace Eno
{   
    // Proxy needs to be instanced to belong to a particular dll
    // So this ProxyEntry class serves as a way into the proxy
    public static class ProxyEntry
    {
        private static ScriptProxy? proxy;

        public static void SetProxy(ScriptProxy p) => proxy = p;

        // Native-callable entry point
        [UnmanagedCallersOnly(EntryPoint = "CallProxyMethod")]
        public static void CallProxyMethod(IntPtr classNamePtr, IntPtr methodNamePtr)
        {
            try
            {
                if (proxy == null) return;

                string className = Marshal.PtrToStringUni(classNamePtr)!;
                string methodName = Marshal.PtrToStringUni(methodNamePtr)!;

                // Delegates the call into hot-reloaded assembly
                proxy.Call(className, methodName);
            }
            catch (Exception ex)
            {
                Console.WriteLine($"[ProxyEntry] CallProxyMethod failed: {ex}");
            }
        }

        // Native-callable entry point for registering script instances
        [UnmanagedCallersOnly(EntryPoint = "CallInstancedMethod")]
        public static void CallInstancedMethod(IntPtr classNamePtr, IntPtr methodNamePtr, uint entityID)
        {
            try
            {
                if (proxy == null) return;

                string className = Marshal.PtrToStringUni(classNamePtr)!;
                string methodName = Marshal.PtrToStringUni(methodNamePtr)!;

                // Delegates the call into hot-reloaded assembly
                proxy.Call(className, methodName, entityID);
            }
            catch (Exception ex)
            {
                Console.WriteLine($"[ProxyEntry] CallInstancedMethod failed: {ex}");
            }
        }

        // Native-callable entry point for registering script instances
        [UnmanagedCallersOnly(EntryPoint = "RegisterInstances")]
        public static void InstanceRegistration(IntPtr classNamePtr, uint entityID, byte operation)
        {
            try
            {
                if (proxy == null) return;

                bool op = operation != 0;
                string className = Marshal.PtrToStringUni(classNamePtr)!;

                if (op)
                    proxy.RegisterScriptInstance(className, entityID);
                else
                    proxy.DeregisterScriptInstance(className, entityID);
            }
            catch (Exception ex)
            {
                Console.WriteLine($"[ProxyEntry] InstanceRegistration failed: {ex}");
            }
        }
    }
}
