using System;
using System.Runtime.InteropServices;

namespace Eno
{
	public class EnemyController : ScriptBase
    {
        public void Begin()
        {

		}

        public void Update()
        {
		}
	
		public void _when_node1_started()
        {
            Console.WriteLine("1 Node1 has started with audio playing! script.cs");
        }

		public void _when_node3_started()
        {
            Console.WriteLine("3 Node1 has started with audio playing! script.cs");
        }
	}

    // public class EnemyArm : ScriptBase
    // {
    // }
}