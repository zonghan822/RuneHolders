using System.Runtime.InteropServices;

namespace Eno
{
    public static partial class Bridge
    {
        // -----------------------------------------------------------------------
        // Node Management  (implemented in eno_bridge_node.cpp)
        // -----------------------------------------------------------------------
        [DllImport("Engine_shared.dll", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.StdCall)]
        public static extern uint GetNode(string name);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern uint GetChild(uint parentID, uint index);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern uint GetNodeType(uint id);

        // presetName: "DEFAULT", "SPRITE", "KINEMATIC", "RIGIDBODY", "ACTOR",
        //             "PUSHABLE_BOX", "BUTTON", "AUDIO", "CAMERA", "TEXT", "PARTICLE"
        // parentID:   uint.MaxValue means no parent (root node)
        [DllImport("Engine_shared.dll", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.StdCall)]
        public static extern uint SpawnNode(string presetName, string nodeName, uint parentID);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern void DestroyNode(uint sceneObjectID);
    }
}
