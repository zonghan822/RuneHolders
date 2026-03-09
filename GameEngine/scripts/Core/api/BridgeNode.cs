using System.Runtime.InteropServices;

namespace Eno
{
    public static partial class Bridge
    {
        // -----------------------------------------------------------------------
        // Node Management  (implemented in eno_bridge_node.cpp)
        // -----------------------------------------------------------------------
        [DllImport("EnoEngine.exe")]
        public static extern uint CreateNode(uint id);

        [DllImport("EnoEngine.exe")]
        public static extern void FreeNode(uint id);

        [DllImport("Engine_shared.dll", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.StdCall)]
        public static extern uint GetNode(string name);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern uint GetChild(uint parentID, uint index);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern uint GetParentNode(uint childID);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern uint GetNodeType(uint id);

        [DllImport("EnoEngine.exe")]
        public static extern uint AddChild(uint cid, uint pid);

        [DllImport("EnoEngine.exe")]
        public static extern uint Unparent(uint cid);

        [DllImport("EnoEngine.exe")]
        public static extern uint Reparent(uint cid, uint pid);

        [DllImport("EnoEngine.exe")]
        public static extern uint GetChildCount(uint id);


        // presetName: "DEFAULT", "SPRITE", "KINEMATIC", "RIGIDBODY", "ACTOR",
        //             "PUSHABLE_BOX", "BUTTON", "AUDIO", "CAMERA", "TEXT", "PARTICLE"
        // parentID:   uint.MaxValue means no parent (root node)
        // [DllImport("Engine_shared.dll", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.StdCall)]
        // public static extern uint SpawnNode(string presetName, string nodeName, uint parentID);

        // [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        // public static extern void DestroyNode(uint sceneObjectID);
    }
}
