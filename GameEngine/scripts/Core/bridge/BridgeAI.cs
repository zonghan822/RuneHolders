using System.Runtime.InteropServices;

namespace Eno
{
    public static partial class Bridge
    {
        // -----------------------------------------------------------------------
        // AI — AIComponent fields  (implemented in eno_bridge_ai.cpp)
        // -----------------------------------------------------------------------
        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern float GetAwarenessRadius(uint sceneObjectID);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern void SetAwarenessRadius(uint sceneObjectID, float value);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern float GetChaseSpeed(uint sceneObjectID);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern void SetChaseSpeed(uint sceneObjectID, float value);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern bool GetAllowDiagonal(uint sceneObjectID);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern void SetAllowDiagonal(uint sceneObjectID, bool value);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern int GetPathClearance(uint sceneObjectID);

        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern void SetPathClearance(uint sceneObjectID, int value);

        // -----------------------------------------------------------------------
        // AI — Pathfinding  (implemented in eno_bridge_ai.cpp)
        // -----------------------------------------------------------------------

        // Compute a path from (startX, startY) to (goalX, goalY) in world space.
        // allowDiagonal is read from the entity's own AIComponent.
        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern void AI_ComputePath(uint sceneObjectID,
            float startX, float startY, float goalX, float goalY);

        // Recompute path while treating the current waypoint tile as blocked.
        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern void AI_ComputePathAvoidCurrentWaypoint(uint sceneObjectID,
            float startX, float startY, float goalX, float goalY);

        // Clear the stored path.
        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern void AI_ClearPath(uint sceneObjectID);

        // True when at least one unvisited tile remains.
        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern bool AI_HasPath(uint sceneObjectID);

        // Advance the path index to the next tile.
        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern void AI_Advance(uint sceneObjectID);

        // World-space X of the current waypoint tile centre.
        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern float AI_GetNextWaypointX(uint sceneObjectID);

        // World-space Y of the current waypoint tile centre.
        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern float AI_GetNextWaypointY(uint sceneObjectID);

        // Convert world-space position to tile column (X).
        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern int AI_WorldToTileX(float worldX, float worldY);

        // Convert world-space position to tile row (Y).
        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern int AI_WorldToTileY(float worldX, float worldY);

        // Convert tile column/row to world-space centre X.
        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern float AI_TileToWorldX(int tileX, int tileY);

        // Convert tile column/row to world-space centre Y.
        [DllImport("Engine_shared.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern float AI_TileToWorldY(int tileX, int tileY);
    }
}
