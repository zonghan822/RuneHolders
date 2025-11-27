#pragma once

namespace Map {

    struct TileChanged {
        int layer;
        int x;
        int y;
        bool walkable;
    };

    struct TilemapResized {
        int newWidth;
        int newHeight;
        float cellSize;
    };

}