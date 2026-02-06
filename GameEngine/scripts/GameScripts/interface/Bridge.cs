using System;
using System.Runtime.InteropServices;

namespace Eno
{
    public static class Bridge
    {
        // Time
        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern float GetDeltaTime();

        // Input
        [DllImport("Engine_shared.dll")]
        public static extern bool IsKeyReleased(KeyCode key);

        [DllImport("Engine_shared.dll")]
        public static extern bool IsKeyPressed(KeyCode key);

        [DllImport("Engine_shared.dll")]
        public static extern bool IsKeyHeld(KeyCode key);

        [DllImport("Engine_shared.dll")]
        public static extern int ModsApplied();

        //[DllImport("Engine_shared.dll", CharSet = CharSet.Unicode)]
        [DllImport("Engine_shared.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
        public static extern bool SetEntityAnimation(uint entityID, string animationName);

        // State Machine
        [DllImport("Engine_shared.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
        public static extern void ChangeState(uint entityID, string stateName);

        [DllImport("Engine_shared.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
        public static extern IntPtr GetEntityAIState(uint entityID);

        // Scene Management
        [DllImport("Engine_shared.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
        public static extern void LoadScene(string sceneName);

        // AI and Pathfinding
        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern bool GetPlayerPosition(out float outX, out float outY);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern bool GetEntityPosition(uint entityID, out float outX, out float outY);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern bool SetEntityVelocity(uint entityID, float vx, float vy);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern bool RequestPathToPosition(uint entityID, float targetX, float targetY, bool allowDiagonal);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern bool GetNextWaypoint(uint entityID, out float outX, out float outY);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern float CalculateDistance(float x1, float y1, float x2, float y2);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern bool MoveTowards(uint entityID, float targetX, float targetY, float speed);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern bool IsNearPosition(uint entityID, float targetX, float targetY, float threshold);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern bool ClearPath(uint entityID);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern bool GetAIComponent(uint entityID, out float outAwarenessRadius, out float outChaseSpeed,
            out float outAttackRange, out float outWindupTime, out float outAttackDuration, out float outHurtDuration,
            out int outHealth, out bool outAllowDiagonal);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern bool SetAIHealth(uint entityID, int health);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern bool GetAIHealth(uint entityID, out int outHealth);
    }
}
