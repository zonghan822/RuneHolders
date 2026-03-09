/*============================================================================================
* @file         raycast_renderer.cpp
* @brief        GPU ray-cast renderer — implementation.
*               Uses EnoShader / EnoFramebuffer from the engine's graphics system and
*               EnoLoader to retrieve the pre-scanned shader sources.
*//*========================================================================================*/

#include "raycast_renderer.hpp"
#include "../resources/eno_loader.hpp"  // full type needed here only

#include <iostream>

namespace Raycast {

// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------

RaycastRenderer::~RaycastRenderer()
{
    if (mapSSBO_) { glDeleteBuffers(1, &mapSSBO_);      mapSSBO_ = 0; }
    if (vbo_)     { glDeleteBuffers(1, &vbo_);          vbo_     = 0; }
    if (vao_)     { glDeleteVertexArrays(1, &vao_);     vao_     = 0; }
    // EnoShader and EnoFramebuffer clean up their own GL resources in their dtors.
}

// ---------------------------------------------------------------------------
// initialize
// ---------------------------------------------------------------------------

void RaycastRenderer::initialize(int w, int h,
                                  Resources::EnoLoader& loader,
                                  const Map::TileMapSystem& tilemap)
{
    width_  = w;
    height_ = h;

    // -- Retrieve shader sources that the engine already scanned from assets/shaders/ --
    const std::string& vertSrc = loader.getVertShader("raycast");
    const std::string& fragSrc = loader.getFragShader("raycast");

    if (vertSrc.empty() || fragSrc.empty()) {
        std::cerr << "[RaycastRenderer] 'raycast' shader pair not found in loader. "
                     "Ensure raycast.vert and raycast.frag exist in assets/shaders/.\n";
        return;
    }

    if (!shader_.loadFromStrings(vertSrc, fragSrc)) {
        std::cerr << "[RaycastRenderer] Shader compilation / linking failed.\n";
        return;
    }

    // -- Off-screen render target --
    if (!framebuffer_.create(w, h)) {
        std::cerr << "[RaycastRenderer] Framebuffer creation failed.\n";
        return;
    }

    // -- Fullscreen quad --
    createQuad();

    // -- Collision bitmap + GPU SSBO --
    rebuildBitmap(tilemap);
    uploadSSBO();

    initialized_ = true;
}

// ---------------------------------------------------------------------------
// updateMap
// ---------------------------------------------------------------------------

void RaycastRenderer::updateMap(const Map::TileMapSystem& tilemap)
{
    rebuildBitmap(tilemap);
    uploadSSBO();
}

// ---------------------------------------------------------------------------
// render
// ---------------------------------------------------------------------------

void RaycastRenderer::render()
{
    if (!initialized_) return;

    // Save GL state that we will modify
    GLint prevFBO = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFBO);
    GLint prevVP[4];
    glGetIntegerv(GL_VIEWPORT, prevVP);
    GLint prevProg = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &prevProg);

    // ---- Render into our off-screen framebuffer ----
    framebuffer_.bind();
    glViewport(0, 0, width_, height_);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    shader_.use();

    // Upload uniforms via EnoShader helpers
    shader_.setVector2("playerPos",   playerPos.x,   playerPos.y);
    shader_.setVector2("playerDir",   playerDir.x,   playerDir.y);
    shader_.setVector2("cameraPlane", cameraPlane.x, cameraPlane.y);
    shader_.setVector2("iResolution", static_cast<float>(width_),
                                      static_cast<float>(height_));
    shader_.setInt("mapWidth",           mapWidth_);
    shader_.setInt("mapHeight",          mapHeight_);
    shader_.setInt("enableSideShading",  enableSideShading  ? 1 : 0);
    shader_.setInt("enableFloorCeiling", enableFloorCeiling ? 1 : 0);

    // Bind the map SSBO to layout(binding = 0) in the fragment shader
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mapSSBO_);

    // Draw fullscreen quad (6 vertices = 2 triangles)
    glBindVertexArray(vao_);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    // ---- Restore previous GL state ----
    shader_.unuse();                                   // glUseProgram(0)
    glBindFramebuffer(GL_FRAMEBUFFER, prevFBO);        // restore caller's FBO
    glViewport(prevVP[0], prevVP[1], prevVP[2], prevVP[3]);
    if (prevProg) glUseProgram(prevProg);              // restore engine's shader if any
}

// ---------------------------------------------------------------------------
// isWall — CPU-side bitmap lookup (safe for script use, no GPU round-trip)
// ---------------------------------------------------------------------------

bool RaycastRenderer::isWall(int x, int y) const
{
    if (x < 0 || x >= mapWidth_ || y < 0 || y >= mapHeight_) return true;
    return collisionMap_[y * mapWidth_ + x] != 0;
}

// ---------------------------------------------------------------------------
// createQuad — two CCW triangles covering NDC [-1,1]^2
// ---------------------------------------------------------------------------

void RaycastRenderer::createQuad()
{
    float verts[] = {
        -1.0f, -1.0f,
         1.0f, -1.0f,
         1.0f,  1.0f,
        -1.0f, -1.0f,
         1.0f,  1.0f,
        -1.0f,  1.0f,
    };

    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);

    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
    glBindVertexArray(0);
}

// ---------------------------------------------------------------------------
// rebuildBitmap — build CPU collision map from tilemap (all layers combined)
// ---------------------------------------------------------------------------

void RaycastRenderer::rebuildBitmap(const Map::TileMapSystem& tilemap)
{
    mapWidth_  = tilemap.width();
    mapHeight_ = tilemap.height();

    collisionMap_.assign(mapWidth_ * mapHeight_, 0);

    // A tile is a wall if ANY layer marks it as non-walkable.
    for (const auto& layer : tilemap.layers()) {
        for (int y = 0; y < mapHeight_; ++y) {
            for (int x = 0; x < mapWidth_; ++x) {
                if (!layer.isWalkable(x, y))
                    collisionMap_[y * mapWidth_ + x] = 1;
            }
        }
    }
}

// ---------------------------------------------------------------------------
// uploadSSBO — send collision bitmap to the GPU
// ---------------------------------------------------------------------------

void RaycastRenderer::uploadSSBO()
{
    if (mapSSBO_ == 0)
        glGenBuffers(1, &mapSSBO_);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, mapSSBO_);
    glBufferData(GL_SHADER_STORAGE_BUFFER,
                 static_cast<GLsizeiptr>(collisionMap_.size() * sizeof(int)),
                 collisionMap_.data(),
                 GL_DYNAMIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

} // namespace Raycast
