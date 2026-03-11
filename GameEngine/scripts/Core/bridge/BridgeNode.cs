using System.Runtime.InteropServices;

namespace Eno
{
    public static partial class Bridge
    {
        // -----------------------------------------------------------------------
        // Node Management  (implemented in eno_bridge_node.cpp)
        // -----------------------------------------------------------------------
        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern uint CreateNode(uint id);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern void FreeNode(uint id);

        [DllImport("Engine_shared.dll", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.StdCall)]
        public static extern uint GetNode(string name);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern uint GetChild(uint parentID, uint index);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern uint GetParentNode(uint childID);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern uint GetNodeType(uint id);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern void AddChild(uint cid, uint pid);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern void Unparent(uint cid);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern void Reparent(uint cid, uint pid);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern uint GetChildCount(uint id);


        // Spawn a user prefab by name at world position (x, y).
        // parentID: uint.MaxValue means no parent (root node).
        // Returns the sceneObjectID of the spawned node, or uint.MaxValue on failure.
        [DllImport("Engine_shared.dll", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.StdCall)]
        public static extern uint SpawnPrefab(string prefabName, float x, float y, uint parentID);

        // [DllImport("Engine_shared.dll", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.StdCall)]
        // public static extern uint SpawnNode(string presetName, string nodeName, uint parentID);

        // [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        // public static extern void DestroyNode(uint sceneObjectID);
    }
}
