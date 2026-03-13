/*============================================================================================
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
* @file         tilemap_events.hpp
* @author       Muhammad Raziq Bin Mohd Rafig (muhammadraziq.b@digipen.edu)   67%
* @co-author    ZongHan Law (zonghan.law@digipen.edu)   33%
* @brief        See file name.
*//*========================================================================================*/

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