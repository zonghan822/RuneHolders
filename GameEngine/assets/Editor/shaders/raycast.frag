#version 450 core

// ============================================================
// Wolfenstein-3D style ray-caster — pure fragment shader
//
// Map is a flat 1-D array indexed [y * mapWidth + x].
//   0  = open (walkable)
//   1+ = wall (any non-zero value)
//
// Coordinate system:
//   playerPos / playerDir / cameraPlane are in *tile-space*.
//   gl_FragCoord.x = screen column (0 = left)
//   gl_FragCoord.y = screen row    (0 = bottom, OpenGL convention)
// ============================================================

layout(std430, binding = 0) buffer WorldMap {
    int map[];
};

uniform vec2 playerPos;      // camera position in tile-space
uniform vec2 playerDir;      // unit direction vector
uniform vec2 cameraPlane;    // perpendicular to dir, half-width = tan(FOV/2)
uniform vec2 iResolution;    // (width, height) of the render target in pixels
uniform int  mapWidth;
uniform int  mapHeight;
uniform int  enableSideShading;   // 1 = N/S walls at 70% brightness
uniform int  enableFloorCeiling;  // 1 = draw perspective floor & ceiling

out vec4 FragColor;

// ---- helpers -------------------------------------------------------

bool isWall(int x, int y)
{
    if (x < 0 || x >= mapWidth || y < 0 || y >= mapHeight)
        return true;           // treat out-of-bounds as solid wall
    return map[y * mapWidth + x] != 0;
}

// Hash-based wall colour — matches the CPU implementation palette
vec3 tileColor(int tx, int ty, int side)
{
    uint h = uint(tx) * 2246822519u ^ uint(ty) * 2654435761u;
    float r = (80.0 + float(h & 0x3Fu))        / 255.0;
    float g = (60.0 + float((h >>  6u) & 0x3Fu)) / 255.0;
    float b = (40.0 + float((h >> 12u) & 0x1Fu)) / 255.0;
    if (side == 1 && enableSideShading != 0) {
        r *= 0.7; g *= 0.7; b *= 0.7;
    }
    return vec3(r, g, b);
}

// ---- main ----------------------------------------------------------

void main()
{
    float col = gl_FragCoord.x;
    float row = gl_FragCoord.y;   // 0 = bottom, height-1 = top

    float halfH = iResolution.y * 0.5;

    // ---- DDA ray casting for this screen column ----
    float cameraX = 2.0 * col / iResolution.x - 1.0;   // -1..+1
    vec2  rayDir  = playerDir + cameraPlane * cameraX;

    ivec2 mapPos = ivec2(floor(playerPos));

    vec2 deltaDist = vec2(
        rayDir.x == 0.0 ? 1e30 : abs(1.0 / rayDir.x),
        rayDir.y == 0.0 ? 1e30 : abs(1.0 / rayDir.y)
    );

    ivec2 step;
    vec2  sideDist;

    if (rayDir.x < 0.0) { step.x = -1; sideDist.x = (playerPos.x - float(mapPos.x))         * deltaDist.x; }
    else                 { step.x =  1; sideDist.x = (float(mapPos.x) + 1.0 - playerPos.x)   * deltaDist.x; }

    if (rayDir.y < 0.0) { step.y = -1; sideDist.y = (playerPos.y - float(mapPos.y))         * deltaDist.y; }
    else                 { step.y =  1; sideDist.y = (float(mapPos.y) + 1.0 - playerPos.y)   * deltaDist.y; }

    int  side = 0;
    bool hit  = false;
    int  maxSteps = mapWidth + mapHeight + 4;

    for (int i = 0; i < maxSteps && !hit; i++)
    {
        if (sideDist.x < sideDist.y) {
            sideDist.x += deltaDist.x;
            mapPos.x   += step.x;
            side        = 0;
        } else {
            sideDist.y += deltaDist.y;
            mapPos.y   += step.y;
            side        = 1;
        }
        if (isWall(mapPos.x, mapPos.y)) hit = true;
    }

    // ---- Perpendicular wall distance (no fish-eye) ----
    float perpWallDist = 1e10;
    if (hit)
        perpWallDist = (side == 0) ? (sideDist.x - deltaDist.x)
                                   : (sideDist.y - deltaDist.y);
    perpWallDist = max(perpWallDist, 0.001);

    // ---- Wall slice boundaries (pixel rows) ----
    float wallHeight = iResolution.y / perpWallDist;
    float wallBottom = halfH - wallHeight * 0.5;
    float wallTop    = halfH + wallHeight * 0.5;

    // ---- Shared floor/ceiling ray directions ----
    vec2 rayDirLeft  = playerDir - cameraPlane;
    vec2 rayDirRight = playerDir + cameraPlane;

    // ---- Per-fragment region colouring ----
    if (row < wallBottom)
    {
        // ---- FLOOR (below wall) ----
        if (enableFloorCeiling != 0)
        {
            // Perspective-correct floor projection
            float p       = halfH - row;                     // distance below horizon (>0)
            float rowDist = halfH / max(p, 0.001);

            vec2 floorPos = playerPos
                + rowDist * mix(rayDirLeft, rayDirRight, col / iResolution.x);

            int cellX   = int(floor(floorPos.x));
            int cellY   = int(floor(floorPos.y));
            int checker = (cellX ^ cellY) & 1;

            float v = (checker != 0) ? 0.32 : 0.24;
            FragColor = vec4(v, v, v, 1.0);
        }
        else
        {
            FragColor = vec4(0.24, 0.24, 0.24, 1.0);
        }
    }
    else if (row <= wallTop)
    {
        // ---- WALL ----
        vec3 c = hit ? tileColor(mapPos.x, mapPos.y, side) : vec3(0.0);
        float brightness = clamp(1.0 / (1.0 + perpWallDist * 0.15), 0.15, 1.0);
        FragColor = vec4(c * brightness, 1.0);
    }
    else
    {
        // ---- CEILING (above wall) ----
        if (enableFloorCeiling != 0)
        {
            // Perspective-correct ceiling projection
            float p       = row - halfH;                     // distance above horizon (>0)
            float rowDist = halfH / max(p, 0.001);

            vec2 ceilPos = playerPos
                + rowDist * mix(rayDirLeft, rayDirRight, col / iResolution.x);

            int cellX   = int(floor(ceilPos.x));
            int cellY   = int(floor(ceilPos.y));
            int checker = (cellX ^ cellY) & 1;

            FragColor = vec4(
                (checker != 0) ? 0.078 : 0.059,
                (checker != 0) ? 0.078 : 0.059,
                (checker != 0) ? 0.137 : 0.098,
                1.0
            );
        }
        else
        {
            FragColor = vec4(0.059, 0.059, 0.098, 1.0);
        }
    }
}
