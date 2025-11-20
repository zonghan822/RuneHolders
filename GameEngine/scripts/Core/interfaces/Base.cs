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
            //Console.WriteLine($"[Update] InputMng hash: {ScriptBase.InputMng.GetHashCode()}");
        }

        private readonly string name;

        protected ScriptBase([CallerFilePath] string filePath = "")
        {
            name = Path.GetFileNameWithoutExtension(filePath);
        }
    }
}
