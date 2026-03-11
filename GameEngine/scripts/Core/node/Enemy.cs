namespace Eno
{
    // Preset: ENEMY — ACTOR components + AIComponent for pathfinding and behaviour
    public class Enemy : Actor
    {
        public Enemy(uint id, NodeType type) : base(id, type) {}

        // ============= AIComponent fields =============

        public float awarenessRadius
        {
            get => Bridge.GetAwarenessRadius(ID);
            set => Bridge.SetAwarenessRadius(ID, value);
        }

        public float chaseSpeed
        {
            get => Bridge.GetChaseSpeed(ID);
            set => Bridge.SetChaseSpeed(ID, value);
        }

        public bool allowDiagonal
        {
            get => Bridge.GetAllowDiagonal(ID);
            set => Bridge.SetAllowDiagonal(ID, value);
        }

        // ============= Pathfinding =============

        // Compute a path from a world-space start to a world-space goal.
        // allowDiagonal is taken from this enemy's AIComponent automatically.
        public void computePath(float startX, float startY, float goalX, float goalY)
            => Bridge.AI_ComputePath(ID, startX, startY, goalX, goalY);

        // Convenience overload — uses this enemy's current position as the start.
        public void computePath(float goalX, float goalY)
            => Bridge.AI_ComputePath(ID, position.x, position.y, goalX, goalY);

        // Convenience overload — uses this enemy's current position as the start.
        public void computePath(Vector2 goal)
            => Bridge.AI_ComputePath(ID, position.x, position.y, goal.x, goal.y);

        // Discard the current path.
        public void clearPath()
            => Bridge.AI_ClearPath(ID);

        // True when the path has at least one unvisited tile remaining.
        public bool hasPath => Bridge.AI_HasPath(ID);

        // World-space position of the current waypoint (tile centre).
        // Returns Vector2.Zero when there is no path.
        public Vector2 nextWaypoint
            => new Vector2(Bridge.AI_GetNextWaypointX(ID), Bridge.AI_GetNextWaypointY(ID));

        // Advance the internal path index to the next tile.
        // Call this once the enemy has reached nextWaypoint.
        public void advance()
            => Bridge.AI_Advance(ID);

    }
}
