using System;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;

namespace Eno
{
    public static class ScriptRuntime
    {
        [UnmanagedCallersOnly(EntryPoint = "GameUpdate")]
        public static void Synchronize()
        {
            ScriptBase.Synchronize();
        }
    }
}