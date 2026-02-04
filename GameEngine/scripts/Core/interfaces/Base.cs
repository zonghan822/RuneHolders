using System;
using System.IO;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;

namespace Eno
{
    public class ScriptBase
    {
        public static InputManager InputMng => InputManager.Instance;
        
        public static void Synchronize()
        {
            InputMng.Update();
        }

        public uint ID { get; set; }
    }
}
