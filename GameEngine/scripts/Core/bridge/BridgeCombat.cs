using System.Runtime.InteropServices;

namespace Eno
{
    public static partial class Bridge
    {
        // -----------------------------------------------------------------------
        // Combat — Health  (implemented in eno_bridge_combat.cpp)
        // -----------------------------------------------------------------------
        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern float GetHealth(uint sceneObjectID);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern void SetHealth(uint sceneObjectID, float health);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern float GetMaxHealth(uint sceneObjectID);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern void SetMaxHealth(uint sceneObjectID, float maxHealth);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern bool IsDead(uint sceneObjectID);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern void HealEntity(uint sceneObjectID, float amount);

        // Deal flat damage; returns true if the entity died.
        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern bool TakeDamageFlat(uint sceneObjectID, float amount);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern bool IsInvulnerable(uint sceneObjectID);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern void SetInvulnerable(uint sceneObjectID, bool value);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern float GetInvulnerabilityTimer(uint sceneObjectID);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern void SetInvulnerabilityTimer(uint sceneObjectID, float value);
    }
}
