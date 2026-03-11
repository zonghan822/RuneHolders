namespace Eno
{
    // Global tilemap coordinate utilities — accessible from any script.
    public static class Tilemap
    {
        // Convert a world-space position to its tile coordinate.
        public static Vector2 worldToTile(float worldX, float worldY)
            => new Vector2(Bridge.AI_WorldToTileX(worldX, worldY),
                           Bridge.AI_WorldToTileY(worldX, worldY));

        public static Vector2 worldToTile(Vector2 world)
            => worldToTile(world.x, world.y);

        // Convert a tile coordinate to the world-space centre of that tile.
        public static Vector2 tileToWorld(int tileX, int tileY)
            => new Vector2(Bridge.AI_TileToWorldX(tileX, tileY),
                           Bridge.AI_TileToWorldY(tileX, tileY));

        public static Vector2 tileToWorld(Vector2 tile)
            => tileToWorld((int)tile.x, (int)tile.y);
    }
}
