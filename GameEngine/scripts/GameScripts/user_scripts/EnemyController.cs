using System;
using System.Runtime.InteropServices;

namespace Eno
{
	public class EnemyController : ScriptBase
    {
        public void Begin()
        {
            Console.WriteLine("Enemy controller Begin invoked!");
		}

        public void Update()
        {
            Console.WriteLine("Enemy controller Update invoked!");
		}
	
		public void _when_node1_started()
        {
            Console.WriteLine("Node1 has started with audio playing!");
        }
	}

}