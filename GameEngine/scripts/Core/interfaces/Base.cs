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

        public uint nodeID;

        public Node GetNode(string name)
        {
            return new Node(Bridge.GetNode(name));
        }

        public Node GetChild(uint index)
        {
            return new Node(Bridge.GetChild(nodeID, index));
        }

        public uint ID { get; set; }
    }
}
