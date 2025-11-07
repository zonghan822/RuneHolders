using System;
using System.Runtime.InteropServices;

namespace Eno
{
	public class EnemyController : ScriptBase
    {
        public static void Begin()
        {
            Console.WriteLine("Enemy Begin");
		}

        public static void Update()
        {
            Console.WriteLine("Enemy Update");
		}
	}
}