using System.Runtime.InteropServices;

namespace Eno
{
    public static partial class Bridge
    {
        // -----------------------------------------------------------------------
        // Collision  (implemented in eno_bridge_collision.cpp)
        // -----------------------------------------------------------------------

        // Returns true if the entity's collider overlaps the world-space point.
        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern bool IsOverlappingPoint(uint sceneObjectID, float x, float y);

        // Returns true if both entities' colliders overlap each other.
        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern bool IsColliding(uint sceneObjectIDA, uint sceneObjectIDB);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern bool GetColliderActive(uint sceneObjectID);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern void SetColliderActive(uint sceneObjectID, bool active);
    }
}
