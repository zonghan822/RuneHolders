#pragma once

// OpenGL and GLFW
#define GLFW_INCLUDE_NONE
#include <GL/glew.h>
#include <GLFW/glfw3.h>

// Engine components
#include "../system_interface/sys_interface.hpp"
#include "../event/eventBus.hpp"
#include "../math/math.hpp"

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

    // Vertex struct for rendering
    struct Vertex
    {
        Math::eVec2<float> position;
        Color color;

        Vertex() = default;
        Vertex(const Math::eVec2<float>& pos, const Color& col = Color::White)
            : position(pos), color(col) {
        }
    };

    // Low-level OpenGL wrapper for managing render state
    class EnoGLContext
    {
    public:
        static bool initialize(GLFWwindow* window);
        static void setViewport(int width, int height);
        static void clear(const Color& clearColor = Color::Black);
        static void enableBlending();
        static void disableBlending();

    private:
        static bool initialized_;
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
    class EnoGraphicsManager : public Sys::ISystem
    {
    public:
        EnoGraphicsManager(Event::EnoEventBus& bus, GLFWwindow* window);
        ~EnoGraphicsManager();

        // System interface implementation
        void Update() override;

        // Initialize the graphics system
        bool initialize();
        void shutdown();

        // Frame management
        void beginFrame(const Color& clearColor = Color::Black);
        void endFrame();

        // Immediate mode drawing functions
        void drawPoint(const Math::eVec2<float>& position, const Color& color = Color::White, float size = 1.0f);
        void drawLine(const Math::eVec2<float>& start, const Math::eVec2<float>& end, const Color& color = Color::White, float thickness = 1.0f);
        void drawTriangle(const Math::eVec2<float>& p1, const Math::eVec2<float>& p2, const Math::eVec2<float>& p3, const Color& color = Color::White, bool filled = true);
        void drawRect(const Math::eVec2<float>& position, const Math::eVec2<float>& size, const Color& color = Color::White, bool filled = true);
        void drawCircle(const Math::eVec2<float>& center, float radius, const Color& color = Color::White, bool filled = true, int segments = 32);

        // Camera/projection management
        void setProjectionMatrix(int windowWidth, int windowHeight);
        void setProjectionMatrix(const float* matrix);

        // Getters
        bool isInitialized() const { return initialized_; }

    private:
        // Core systems
        Event::EnoEventBus& bus_;
        GLFWwindow* window_;
        bool initialized_;

        // Rendering components
        std::unique_ptr<EnoShader> defaultShader_;
        std::unique_ptr<EnoVertexBuffer> vertexBuffer_;

        // Batch rendering (still testing)
        std::vector<Vertex> batchVertices_;
        static constexpr size_t MAX_BATCH_SIZE = 10000;

        // Projection matrix for 2D rendering
        std::array<float, 16> projectionMatrix_;

        // Internal helper functions
        bool createDefaultShader();
        void setupProjectionMatrix(int width, int height);
        void flushBatch(GLenum primitiveType);
        void addVertexToBatch(const Vertex& vertex);

        // Math helpers functiosn for shape generation
        std::vector<Vertex> generateCircleVertices(const Math::eVec2<float>& center, float radius, const Color& color, int segments, bool filled);
        std::vector<Vertex> generateRectVertices(const Math::eVec2<float>& position, const Math::eVec2<float>& size, const Color& color, bool filled);
    };

    // Graphics events
    namespace Events
    {
        struct WindowResized
        {
            int width;
            int height;
        };

        struct RenderFrameBegin {};
        struct RenderFrameEnd {};
    }
}