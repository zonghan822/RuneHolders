using System;
using System.Runtime.InteropServices;

namespace Eno
{
	public class ButtonScript : ScriptBase
    {
	
		public void _when_node32_onclick()
        {
			Console.WriteLine("[ButtonScript] Button clicked! Loading Level1...");
			Bridge.LoadScene("Level1.txt");
        }
	}

}