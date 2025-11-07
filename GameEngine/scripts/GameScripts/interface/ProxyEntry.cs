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
            if (proxy == null) return;

            string className = Marshal.PtrToStringUni(classNamePtr)!;
            string methodName = Marshal.PtrToStringUni(methodNamePtr)!;

            // Delegates the call into hot-reloaded assembly
            proxy.Call(className, methodName);
        }
    }
}
