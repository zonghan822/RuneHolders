using System;
using System.Runtime.InteropServices;

namespace Eno
{
    public class EnemyController : ScriptBase
    {
        [UnmanagedCallersOnly]
        public static void Begin()
        {
            Console.WriteLine("Begin Enemy Controller");
        }

        [UnmanagedCallersOnly]
        public static void Update()
        {
            Console.WriteLine("Enemy Controller Update");
        }
    }
}