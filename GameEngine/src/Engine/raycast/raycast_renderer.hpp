/*============================================================================================
* @file         raycast_renderer.hpp
* @brief        Wolfenstein-3D-style GPU ray-cast renderer.
*
*               Architecture:
*                 - Uses EnoLoader to retrieve the pre-loaded raycast.vert / raycast.frag
*                   shader sources; EnoShader compiles and owns the program.
*                 - Renders a fullscreen quad each frame into an EnoFramebuffer whose
*                   colour texture is exposed via getTextureID() for ImGui::Image.
*                 - The fragment shader (assets/shaders/raycast.frag) performs DDA ray
*                   casting per screen column; floor/ceiling projection is also on the GPU.
*                 - The tilemap is mirrored on the GPU as a Shader Storage Buffer Object
*                   (SSBO); call updateMap() after any map edit.
*
*               Player control:
*                 Set playerPos / playerDir / cameraPlane directly from scripts.
*                 The renderer does NOT move or rotate the player.
*
*               Collision bitmap:
*                 collisionMap_ is a flat CPU-side array (0=open, 1=wall) built from all
*                 tilemap layers combined. Use isWall(x, y) for script-side queries.
*//*========================================================================================*/

#pragma once

#include <vector>
#include <GL/glew.h>
#include "../graphics/graphics.hpp"   // EnoShader, EnoFramebuffer
#include "../tilemap/tilemap.hpp"
#include "../math/math.hpp"

// Forward declaration — full header included only in the .cpp
namespace Resources { class EnoLoader; }

namespace Raycast {

class RaycastRenderer {
public:
    // ---- Camera state — written by scripts / editor ----
    Math::eVec2<float> playerPos   { 2.0f, 2.0f };   // tile-space position
    Math::eVec2<float> playerDir   { 1.0f, 0.0f };   // unit forward vector
    Math::eVec2<float> cameraPlane { 0.0f, 0.66f };  // perpendicular: |plane|=tan(FOV/2)

    // ---- Visual settings ----
    bool enableFloorCeiling = true;   // perspective-correct floor/ceiling
    bool enableSideShading  = true;   // N/S walls at 70% brightness

    // ---- Lifetime ----
    ~RaycastRenderer();

    // ---- Initialise — call once after the engine's setupResourceLoading() ----
    // loader  : engine_.getResourceLoader()  (shaders already scanned)
    // tilemap : engine_.getTileMap()         (builds initial SSBO + CPU bitmap)
    void initialize(int width, int height,
                    Resources::EnoLoader& loader,
                    const Map::TileMapSystem& tilemap);

    // ---- Map sync — call after any tilemap edit (or press "Refresh Map") ----
    void updateMap(const Map::TileMapSystem& tilemap);

    // ---- Render one frame into the internal framebuffer ----
    void render();

    // ---- Queries ----
    GLuint getTextureID()  const { return framebuffer_.getTextureID(); }
    int    getWidth()      const { return width_; }
    int    getHeight()     const { return height_; }
    bool   isInitialized() const { return initialized_; }

    // ---- CPU-side collision bitmap ----
    bool isWall(int x, int y) const;
    int  bitmapWidth()  const { return mapWidth_; }
    int  bitmapHeight() const { return mapHeight_; }

private:
    int  width_     = 0;
    int  height_    = 0;
    bool initialized_ = false;

    int  mapWidth_  = 0;
    int  mapHeight_ = 0;

    // ---- Engine graphics objects ----
    Graphics::EnoShader      shader_;       // raycast shader program
    Graphics::EnoFramebuffer framebuffer_;  // off-screen render target

    // ---- Raw GL objects (no higher-level abstraction covers these) ----
    GLuint vao_     = 0;   // fullscreen quad geometry
    GLuint vbo_     = 0;
    GLuint mapSSBO_ = 0;   // GPU mirror of collisionMap_

    // ---- CPU collision bitmap ----
    // Flat array indexed [y * mapWidth_ + x]; 0 = open, 1 = wall.
    // Built by combining all tilemap layers (any layer non-walkable → wall).
    std::vector<int> collisionMap_;

    // ---- Internal helpers ----
    void createQuad();
    void rebuildBitmap(const Map::TileMapSystem& tilemap);
    void uploadSSBO();
};

} // namespace Raycast
