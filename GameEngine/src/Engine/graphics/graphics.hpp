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

        // Text drawing function
        void drawText(const std::shared_ptr<Font::EnoFont>& font,
            const std::string& text,
            const Math::eVec2<float>& position,
            float scale,
            const Color& color);

        // Camera/projection management
        void setProjectionMatrix(int windowWidth, int windowHeight);
        void setProjectionMatrix(const float* matrix);

        // Getters
        bool isInitialized() const { return initialized_; }
        std::unordered_map<std::string, std::shared_ptr<EnoTexture>> texMap() const { return texMap_; }

        // Map texture
        void mapTexture(const TextureMapping& e);
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