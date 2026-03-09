 /*============================================================================================
* @file         graphics.hpp
* @author       Law Zong Han (zonghan.law@digipen.edu),
* @author       Muhammad Raziq Bin Mohd Rafig ()
* @date         03-10-2025
* @brief
*//*========================================================================================*/

#pragma once

// OpenGL and GLFW
#include <GL/glew.h>
#include <GLFW/glfw3.h>

// Engine components
#include "../event/event_bus.hpp"
#include "../math/math.hpp"
#include "../window/eno_window.hpp"
#include "font/font.hpp"

// Standard libraries
#include <vector>
#include <string>
#include <memory>
#include <array>
#include <algorithm>
#include <unordered_map>

// Forward declarations
namespace Engine  { class EnoEngine; }
namespace Graphics { class VideoPlayer; }

// Math constants
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace Graphics
{
    // Forward declarations
    class EnoRenderer;
    class EnoShader;
    class EnoVertexBuffer;
    class EnoTexture;
    class EnoFrameBuffer;

    // Color struct for drawing functions, basic colours alr added
    struct Color
    {
        float r, g, b, a;

        Color(float red = 1.0f, float green = 1.0f, float blue = 1.0f, float alpha = 1.0f)
            : r(red), g(green), b(blue), a(alpha) {
        }

        static const Color White;
        static const Color Black;
        static const Color Red;
        static const Color Green;
        static const Color Blue;
    };

    // Output stream operator for Color serialization
    inline std::ostream& operator<<(std::ostream& os, const Color& c)
    {
        os << "r : " << c.r << " | g : " << c.g << " | b : " << c.b << " | a : " << c.a;
        return os;
    }

    // Vertex struct for rendering with texture support
    struct Vertex
    {
        Math::eVec2<float> position;
        Color color;
        Math::eVec2<float> texCoords;
        float texID;

        Vertex() = default;
        Vertex(const Math::eVec2<float>& pos, const Color& col = Color::White,
            const Math::eVec2<float>& tex = { 0.0f, 0.0f }, float texId = 0.0f)
            : position(pos), color(col), texCoords(tex), texID(texId) {
        }
    };

    // Texture class for loading and managing textures
    class EnoTexture
    {
    public:
        EnoTexture();
        ~EnoTexture();

        bool loadFromFile(const std::string& filepath);
        bool loadFromMemory(int width, int height, int channels, const unsigned char* data);
        // Stream-update pixels in-place (no reallocation). Texture must already be valid.
        // Used by VideoPlayer to upload decoded frames without creating a new GL texture.
        void updatePixels(int width, int height, int channels, const unsigned char* data);
        void bind(unsigned int slot = 0) const;
        void unbind() const;

        int getWidth() const { return width_; }
        int getHeight() const { return height_; }
        GLuint getID() const { return textureID_; }
        bool isValid() const { return textureID_ != 0; }

    private:
        GLuint textureID_;
        int width_, height_, channels_;
    };

    class EnoFramebuffer
    {
    public:
        EnoFramebuffer();
        ~EnoFramebuffer();

        bool create(int width, int height);
        void bind() const;
        void unbind() const;
        void resize(int width, int height);

        GLuint getTextureID() const { return textureID_; }
        int getWidth() const { return width_; }
        int getHeight() const { return height_; }
        bool isValid() const { return fbo_ != 0; }

    private:
        GLuint fbo_;
        GLuint textureID_;
        GLuint rbo_;
        int width_, height_;
    };


    struct TextureData {
        std::shared_ptr<EnoTexture> texture;
        int rows = 1;
        int cols = 1;

        // Derived frame size
        int frameWidth = 0;
        int frameHeight = 0;

        void computeFrameSize() {
            if (texture && texture->getWidth() > 0 && texture->getHeight() > 0) {
                frameWidth = texture->getWidth() / cols;
                frameHeight = texture->getHeight() / rows;
            }
        }
    };

    struct TextureMapping {
        std::string path;
        std::shared_ptr<EnoTexture> tex;
    };
    // Shader management class
    class EnoShader
    {
    public:
        EnoShader() : programID_(0) {}
        ~EnoShader();

        bool loadFromStrings(const std::string& vertexSource, const std::string& fragmentSource);
        void use() const;
        void unuse() const;

        // Uniform setters
        void setMatrix4(const std::string& name, const float* matrix) const;
        void setVector2(const std::string& name, float x, float y) const;
        void setColor(const std::string& name, const Color& color) const;
        void setFloat(const std::string& name, float value) const;
        void setInt(const std::string& name, int value) const;
        void setIntArray(const std::string& name, int* values, int count) const;

        bool isValid() const { return programID_ != 0; }

    private:
        GLuint programID_;

        GLuint compileShader(const std::string& source, GLenum shaderType) const;
        bool linkProgram(GLuint vertexShader, GLuint fragmentShader);
        GLint getUniformLocation(const std::string& name) const;
    };

    // Vertex buffer management forrendering
    class EnoVertexBuffer
    {
    public:
        EnoVertexBuffer();
        ~EnoVertexBuffer();

        void uploadData(const std::vector<Vertex>& vertices);
        void bind() const;
        void unbind() const;
        void draw(GLenum primitiveType) const;

        size_t getVertexCount() const { return vertexCount_; }

    private:
        GLuint VAO_;  // Vertex Array Object
        GLuint VBO_;  // Vertex Buffer Object
        size_t vertexCount_;

        void setupVertexAttributes();
    };

    // Main graphics manager
    class EnoGraphicsManager 
    {
    public:
        EnoGraphicsManager(Event::EnoEventBus& bus);
        ~EnoGraphicsManager();

        // Initialize the graphics system
        void initialize(Window::EnoWindow& windowMng);
        void shutdown();

        // Frame management
        void beginFrame(const Color& clearColor = Color::Black);
        void endFrame(Window::EnoWindow& windowMng);
        void flush() {
            if (!batchVertices_.empty()) {
                flushBatch(currentBatchPrimitiveType_);
            }
        }
        void rebindDefaultShader() {
            if (defaultShader_ && defaultShader_->isValid()) {
                defaultShader_->use();
                defaultShader_->setMatrix4("u_projection", projectionMatrix_.data());
            }
        }

        // Immediate mode drawing functions
        void drawPoint(const Math::eVec2<float>& position, const Color& color = Color::White, float size = 1.0f);
        void drawLine(const Math::eVec2<float>& start, const Math::eVec2<float>& end, const Color& color = Color::White, float thickness = 1.0f);
        void drawTriangle(const Math::eVec2<float>& p1, const Math::eVec2<float>& p2, const Math::eVec2<float>& p3, const Color& color = Color::White, bool filled = true);
        void drawRect(const Math::eVec2<float>& position, const Math::eVec2<float>& size, const Color& color = Color::White, bool filled = true, float rotation = 0.0f);
        void drawCircle(const Math::eVec2<float>& center, float radius, const Color& color = Color::White, bool filled = true, int segments = 32);

        // Textured drawing functions
        void drawTexturedQuad(const Math::eVec2<float>& position,
            const Math::eVec2<float>& size,
            std::shared_ptr<EnoTexture> texture,
            const Color& tint = Color::White,
            float rotation = 0.0f);

        void drawTexturedQuadUV(const Math::eVec2<float>& position,
            const Math::eVec2<float>& size,
            std::shared_ptr<EnoTexture> texture,
            const Math::eVec2<float>& uvStart,
            const Math::eVec2<float>& uvEnd,
            const Color& tint = Color::White,
            float rotation = 0.0f);

        // Per-corner UV variant — used for tile rotation
        void drawTexturedQuadUVCorners(const Math::eVec2<float>& position,
            const Math::eVec2<float>& size,
            std::shared_ptr<EnoTexture> texture,
            const Math::eVec2<float>& uvTL,
            const Math::eVec2<float>& uvTR,
            const Math::eVec2<float>& uvBR,
            const Math::eVec2<float>& uvBL,
            const Color& tint = Color::White);

        // Text drawing function
        void drawText(const std::shared_ptr<Font::EnoFont>& font,
            const std::string& text,
            const Math::eVec2<float>& position,
            float scale,
            const Color& color);

        void drawTextScreen(const std::shared_ptr<Font::EnoFont>& font,
            const std::string& text,
            const Math::eVec2<float>& position,
            float scale,
            const Color& color,
            float screenWidth,
            float screenHeight);

        // Camera/projection management
        void setProjectionMatrix(int windowWidth, int windowHeight);
        void setProjectionMatrix(const float* matrix);

        // Override the pixel dimensions used for glViewport inside updateCamera().
        // Call before renderFrame() when rendering into a framebuffer whose pixel
        // size differs from the OS window (e.g. the editor FBO).
        // Pass (0, 0) to revert to the OS window framebuffer size.
        void setRenderTargetSize(int w, int h) { renderTargetW_ = w; renderTargetH_ = h; }

        // Set a fallback camera used by updateCamera() and render() when no
        // CameraComponent entity exists in the scene.  The editor calls this
        // every frame so entities are visible even without a camera node.
        // centerX/Y is the world-space centre of the viewport.
        void setFallbackCamera(float centerX, float centerY, float width, float height)
        {
            fallbackCamCenterX_ = centerX;
            fallbackCamCenterY_ = centerY;
            fallbackCamWidth_   = width;
            fallbackCamHeight_  = height;
        }

        // Getters
        bool isInitialized() const { return initialized_; }
        std::unordered_map<std::string, std::shared_ptr<EnoTexture>> texMap() const { return texMap_; }

        // Map texture
        void mapTexture(const TextureMapping& e);

        // Restore animation controllers from serialized component data (when dirty),
        // then tick all controllers. Call once per frame before updateCamera().
        void updateAnimations(Engine::EnoEngine& engine);

        // Mark animation controllers as dirty so they are restored next frame.
        void markAnimationsDirty() { animationsDirty_ = true; }

        // Update the projection matrix from the active CameraComponent.
        // Also syncs the script camera position via the engine bridge.
        // Call once per frame before render().
        void updateCamera(Engine::EnoEngine& engine);

        // Render all entities with graphics components, sorted by layer.
        // Draws textured quads, shapes, and text components.
        void render(Engine::EnoEngine& engine);

        // --- Video management ---
        // Load a video file by name. Returns false if FFmpeg fails to open the file.
        bool loadVideo(const std::string& name, const std::string& path);

        // Retrieve a loaded video player (nullptr if not found).
        std::shared_ptr<VideoPlayer> getVideo(const std::string& name);

        // Remove a video player and release its resources.
        void removeVideo(const std::string& name);

        // Tick all currently PLAYING video players. Call once per frame.
        void updateVideos(float dt);

        // Read-only access for the ImGui video browser.
        const std::unordered_map<std::string, std::shared_ptr<VideoPlayer>>& getAllVideos() const { return videoMap_; }

        // Mutable access needed for deletion from the ImGui video browser.
        std::unordered_map<std::string, std::shared_ptr<VideoPlayer>>& getVideosMap() { return videoMap_; }

    private:
        // Core systems
        Event::EnoEventBus& bus_;
        bool initialized_;

        std::unordered_map<std::string, std::shared_ptr<EnoTexture>> texMap_; // Texture path | Texture Shared_Ptr

        // Rendering components
        std::unique_ptr<EnoShader> defaultShader_;
        std::unordered_map<std::string, std::unique_ptr<EnoShader>> shadersMap_;
        std::unique_ptr<EnoVertexBuffer> vertexBuffer_;

        // Texture management
        std::vector<std::shared_ptr<EnoTexture>> boundTextures_;
        static constexpr size_t MAX_TEXTURE_SLOTS = 32;

        // Batch rendering (WORKS HALLELUJAH)
        std::vector<Vertex> batchVertices_;
        static constexpr size_t MAX_BATCH_SIZE = 50000;
        GLenum currentBatchPrimitiveType_ = GL_TRIANGLES;

        // Projection matrix for 2D rendering
        std::array<float, 16> projectionMatrix_;

        // Animation dirty flag — true on first frame and after scene load
        bool animationsDirty_ = true;

        // Pixel dimensions of the current render target.  0 = use OS window size.
        int renderTargetW_ = 0;
        int renderTargetH_ = 0;

        // Video players: name -> player
        std::unordered_map<std::string, std::shared_ptr<VideoPlayer>> videoMap_;
        // Fallback camera used when no CameraComponent entity is present.
        float fallbackCamCenterX_ = 0.f;
        float fallbackCamCenterY_ = 0.f;
        float fallbackCamWidth_   = 0.f;
        float fallbackCamHeight_  = 0.f;

        // Internal helper functions
        bool createDefaultShader();
        bool createShader(const std::string& name, const std::string& vertSrc, const std::string& fragSrc);
        void setupProjectionMatrix(int width, int height);
        void flushBatch(GLenum primitiveType);
        void addVertexToBatch(const Vertex& vertex);
        EnoShader* getShader(const std::string& name);
        int getTextureSlot(std::shared_ptr<EnoTexture> texture);
        void bindTextures();
    };

    // Conversion functions
    inline Math::eVec2<float> screenToWorld(const Math::eVec2<float>& screenPos, const Math::eVec2<float>& imageMin, const Math::eVec2<float>& imageMax, float viewPortWidth, float viewPortHeight, float cameraX, float cameraY)
    {
        // Get mouse position relative to the image (0.0 to 1.0)
        float relX = (screenPos.x - imageMin.x) / (imageMax.x - imageMin.x);
        float relY = (screenPos.y - imageMin.y) / (imageMax.y - imageMin.y);

        relX = std::clamp(relX, 0.0f, 1.0f);
        relY = std::clamp(relY, 0.0f, 1.0f);

        // Convert to screen space coordinates (within the editor viewport)
        float screenX = relX * viewPortWidth;
        float screenY = relY * viewPortHeight;

        // Add camera offset to convert screen space to world space
        float worldX = screenX + cameraX;
        float worldY = screenY + cameraY;

        return { worldX, worldY };
    }

    inline Math::eVec2<float> worldToScreen(const Math::eVec2<float>& worldPos, const Math::eVec2<float>& imageMin, const Math::eVec2<float>& imageMax, float viewPortWidth, float viewPortHeight, float cameraX, float cameraY)
    {
        // Subtract camera offset to convert world space to screen space
        float screenX = worldPos.x - cameraX;
        float screenY = worldPos.y - cameraY;

        // Convert to normalized coordinates (0.0 to 1.0)
        float relX = screenX / viewPortWidth;
        float relY = screenY / viewPortHeight;

        // Convert to actual screen position
        float finalX = imageMin.x + relX * (imageMax.x - imageMin.x);
        float finalY = imageMin.y + relY * (imageMax.y - imageMin.y);

        return { finalX, finalY };
    }


    // Graphics events
    namespace Events
    {
        struct ShaderLoaded {
            std::string name;
            std::string vertexSource;
            std::string fragmentSource;
        };
    }
}