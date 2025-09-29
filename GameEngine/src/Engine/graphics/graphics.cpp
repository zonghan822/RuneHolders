#include "graphics.hpp"
#include <iostream>
#include <cmath>

namespace Graphics
{
    // Static color definitions
    const Color Color::White(1.0f, 1.0f, 1.0f, 1.0f);
    const Color Color::Black(0.0f, 0.0f, 0.0f, 1.0f);
    const Color Color::Red(1.0f, 0.0f, 0.0f, 1.0f);
    const Color Color::Green(0.0f, 1.0f, 0.0f, 1.0f);
    const Color Color::Blue(0.0f, 0.0f, 1.0f, 1.0f);

    // =============================================================================
    // EnoGLContext Implementation - Low-level OpenGL state management
    // =============================================================================

    bool EnoGLContext::initialized_ = false;

    bool EnoGLContext::initialize(GLFWwindow* window)
    {
        if (initialized_) return true;

        glfwMakeContextCurrent(window);

        // GLEW Init
        GLenum glewError = glewInit();
        if (glewError != GLEW_OK)
        {
            std::cerr << "Failed to initialize GLEW: " << glewGetErrorString(glewError) << std::endl;
            return false;
        }

        // Verify
        const GLubyte* version = glGetString(GL_VERSION);
        if (!version)
        {
            std::cerr << "Failed to get OpenGL version - context may not be properly initialized!" << std::endl;
            return false;
        }

        // Print OpenGL version info for debugging and verification
        std::cout << "OpenGL Version: " << version << std::endl;
        std::cout << "GPU: " << glGetString(GL_RENDERER) << std::endl;
        std::cout << "OpenGL Vendor: " << glGetString(GL_VENDOR) << std::endl;

        // Check that we have at least OpenGL 3.3 or not thsi shit crash and burns
        if (!GLEW_VERSION_3_3)
        {
            std::cerr << "OpenGL 3.3 not available! Your graphics card may be too old." << std::endl;
            return false;
        }

        // Set up basic OpenGL state for 2D rendering
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Enable point size control from shaders for drawPoint
        glEnable(GL_PROGRAM_POINT_SIZE); // Can disable if not needed

        initialized_ = true;
        std::cout << "OpenGL context initialized successfully!" << std::endl;
        return true;
    }

    void EnoGLContext::setViewport(int width, int height)
    {
        glViewport(0, 0, width, height);
    }

    void EnoGLContext::clear(const Color& clearColor)
    {
        glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    void EnoGLContext::enableBlending()
    {
        glEnable(GL_BLEND);
    }

    void EnoGLContext::disableBlending()
    {
        glDisable(GL_BLEND);
    }

    // =============================================================================
    // EnoShader Implementation - Shader compilation and management
    // =============================================================================

    EnoShader::~EnoShader()
    {
        if (programID_ != 0)
        {
            glDeleteProgram(programID_);
        }
    }

    bool EnoShader::loadFromStrings(const std::string& vertexSource, const std::string& fragmentSource)
    {
        // Compile individual shaders
        GLuint vertexShader = compileShader(vertexSource, GL_VERTEX_SHADER);
        if (vertexShader == 0) return false;

        GLuint fragmentShader = compileShader(fragmentSource, GL_FRAGMENT_SHADER);
        if (fragmentShader == 0)
        {
            glDeleteShader(vertexShader);
            return false;
        }

        // Link shaders into a program
        bool success = linkProgram(vertexShader, fragmentShader);

        // Clean up individual shaders
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        return success;
    }

    void EnoShader::use() const
    {
        if (programID_ != 0)
        {
            glUseProgram(programID_);
        }
    }

    void EnoShader::unuse() const
    {
        glUseProgram(0);
    }

    void EnoShader::setMatrix4(const std::string& name, const float* matrix) const
    {
        GLint location = getUniformLocation(name);
        if (location != -1)
        {
            glUniformMatrix4fv(location, 1, GL_FALSE, matrix);
        }
    }

    void EnoShader::setVector2(const std::string& name, float x, float y) const
    {
        GLint location = getUniformLocation(name);
        if (location != -1)
        {
            glUniform2f(location, x, y);
        }
    }

    void EnoShader::setColor(const std::string& name, const Color& color) const
    {
        GLint location = getUniformLocation(name);
        if (location != -1)
        {
            glUniform4f(location, color.r, color.g, color.b, color.a);
        }
    }

    void EnoShader::setFloat(const std::string& name, float value) const
    {
        GLint location = getUniformLocation(name);
        if (location != -1)
        {
            glUniform1f(location, value);
        }
    }

    GLuint EnoShader::compileShader(const std::string& source, GLenum shaderType) const
    {
        GLuint shader = glCreateShader(shaderType);
        const char* sourcePtr = source.c_str();
        glShaderSource(shader, 1, &sourcePtr, nullptr);
        glCompileShader(shader);

        // Check compilation status
        GLint success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            GLchar infoLog[512];
            glGetShaderInfoLog(shader, 512, nullptr, infoLog);
            std::cerr << "Shader compilation failed: " << infoLog << std::endl;
            glDeleteShader(shader);
            return 0;
        }

        return shader;
    }

    bool EnoShader::linkProgram(GLuint vertexShader, GLuint fragmentShader)
    {
        programID_ = glCreateProgram();
        glAttachShader(programID_, vertexShader);
        glAttachShader(programID_, fragmentShader);
        glLinkProgram(programID_);

        // Check linking status
        GLint success;
        glGetProgramiv(programID_, GL_LINK_STATUS, &success);
        if (!success)
        {
            GLchar infoLog[512];
            glGetProgramInfoLog(programID_, 512, nullptr, infoLog);
            std::cerr << "Shader program linking failed: " << infoLog << std::endl;
            glDeleteProgram(programID_);
            programID_ = 0;
            return false;
        }

        return true;
    }

    GLint EnoShader::getUniformLocation(const std::string& name) const
    {
        return glGetUniformLocation(programID_, name.c_str());
    }

    // =============================================================================
    // EnoVertexBuffer Implementation - vertex data management
    // =============================================================================

    EnoVertexBuffer::EnoVertexBuffer() : VAO_(0), VBO_(0), vertexCount_(0)
    {
        // Generate vertex array object and vertex buffer object
        glGenVertexArrays(1, &VAO_);
        glGenBuffers(1, &VBO_);

        // Set up vertex attributes
        setupVertexAttributes();
    }

    EnoVertexBuffer::~EnoVertexBuffer()
    {
        if (VBO_ != 0) glDeleteBuffers(1, &VBO_);
        if (VAO_ != 0) glDeleteVertexArrays(1, &VAO_);
    }

    void EnoVertexBuffer::uploadData(const std::vector<Vertex>& vertices)
    {
        vertexCount_ = vertices.size();

        bind();
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_DYNAMIC_DRAW);
        unbind();
    }

    void EnoVertexBuffer::bind() const
    {
        glBindVertexArray(VAO_);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_);
    }

    void EnoVertexBuffer::unbind() const
    {
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void EnoVertexBuffer::draw(GLenum primitiveType) const
    {
        if (vertexCount_ > 0)
        {
            bind();
            glDrawArrays(primitiveType, 0, static_cast<GLsizei>(vertexCount_));
            unbind();
        }
    }

    void EnoVertexBuffer::setupVertexAttributes()
    {
        bind();

        // Position attribute (location 0)
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
        glEnableVertexAttribArray(0);

        // Color attribute (location 1)
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
        glEnableVertexAttribArray(1);

        unbind();
    }

    // =============================================================================
    // EnoGraphicsManager Implementation - Main graphics interface
    // =============================================================================

    EnoGraphicsManager::EnoGraphicsManager(Event::EnoEventBus& bus, GLFWwindow* window)
        : bus_(bus), window_(window), initialized_(false)
    {
        // Reserve space for batch rendering
        batchVertices_.reserve(MAX_BATCH_SIZE);

        // Initialize projection matrix as identity
        projectionMatrix_.fill(0.0f);
        projectionMatrix_[0] = projectionMatrix_[5] = projectionMatrix_[10] = projectionMatrix_[15] = 1.0f;
    }

    EnoGraphicsManager::~EnoGraphicsManager()
    {
        shutdown();
    }

    void EnoGraphicsManager::Update()
    {
        // Will add window resize stuff later
    }

    bool EnoGraphicsManager::initialize()
    {
        if (initialized_) return true;

        // Initialize OpenGL context
        if (!EnoGLContext::initialize(window_))
        {
            std::cerr << "Failed to initialize OpenGL context!" << std::endl;
            return false;
        }

        // Create default shader for primitive rendering
        if (!createDefaultShader())
        {
            std::cerr << "Failed to create default shader!" << std::endl;
            return false;
        }

        // Create vertex buffer for batch rendering
        vertexBuffer_ = std::make_unique<EnoVertexBuffer>();

        // Set up initial projection matrix
        int width, height;
        glfwGetFramebufferSize(window_, &width, &height);
        setupProjectionMatrix(width, height);

        initialized_ = true;
        std::cout << "Graphics system initialized successfully!" << std::endl;
        return true;
    }

    void EnoGraphicsManager::shutdown()
    {
        defaultShader_.reset();
        vertexBuffer_.reset();
        initialized_ = false;
    }

    void EnoGraphicsManager::beginFrame(const Color& clearColor)
    {
        // Clear the screen
        EnoGLContext::clear(clearColor);

        // Clear batch for new frame
        batchVertices_.clear();

        // Publish frame begin event
        bus_.publish(Events::RenderFrameBegin{});
    }

    void EnoGraphicsManager::endFrame()
    {
        if (!batchVertices_.empty())
        {
            flushBatch(currentBatchPrimitiveType_);  // Use tracked type
        }

        glfwSwapBuffers(window_);
        bus_.publish(Events::RenderFrameEnd{});
    }

    void EnoGraphicsManager::drawPoint(const Math::eVec2<float>& position, const Color& color, float size)
    {
        // Flush immediately
        if (!batchVertices_.empty())
        {
            flushBatch(GL_TRIANGLES);  // Flush any previous geometry
        }

        addVertexToBatch(Vertex(position, color));

        defaultShader_->use();
        defaultShader_->setMatrix4("u_projection", projectionMatrix_.data());
        defaultShader_->setFloat("u_pointSize", size);

        flushBatch(GL_POINTS);

        defaultShader_->unuse();
    }

    void EnoGraphicsManager::drawLine(const Math::eVec2<float>& start, const Math::eVec2<float>& end, const Color& color, float thickness)
    {
        // Simple line implementation
        addVertexToBatch(Vertex(start, color));
        addVertexToBatch(Vertex(end, color));
    }

    void EnoGraphicsManager::drawTriangle(const Math::eVec2<float>& p1, const Math::eVec2<float>& p2, const Math::eVec2<float>& p3, const Color& color, bool filled)
    {
        GLenum primitiveType = filled ? GL_TRIANGLES : GL_LINES;

        // Flush if primitive type changes
        if (!batchVertices_.empty() && currentBatchPrimitiveType_ != primitiveType)
        {
            flushBatch(currentBatchPrimitiveType_);
        }

        currentBatchPrimitiveType_ = primitiveType;

        if (filled)
        {
            addVertexToBatch(Vertex(p1, color));
            addVertexToBatch(Vertex(p2, color));
            addVertexToBatch(Vertex(p3, color));
        }
        else
        {
            // Draw outline as three lines
            addVertexToBatch(Vertex(p1, color));
            addVertexToBatch(Vertex(p2, color));

            addVertexToBatch(Vertex(p2, color));
            addVertexToBatch(Vertex(p3, color));

            addVertexToBatch(Vertex(p3, color));
            addVertexToBatch(Vertex(p1, color));
        }
    }

    void EnoGraphicsManager::drawRect(const Math::eVec2<float>& position, const Math::eVec2<float>& size, const Color& color, bool filled)
    {
        GLenum primitiveType = filled ? GL_TRIANGLES : GL_LINES;

        // Flush if primitive type changes
        if (!batchVertices_.empty() && currentBatchPrimitiveType_ != primitiveType)
        {
            flushBatch(currentBatchPrimitiveType_);
        }

        currentBatchPrimitiveType_ = primitiveType;

        auto vertices = generateRectVertices(position, size, color, filled);
        for (const auto& vertex : vertices)
        {
            addVertexToBatch(vertex);
        }
    }

    void EnoGraphicsManager::drawCircle(const Math::eVec2<float>& center, float radius, const Color& color, bool filled, int segments)
    {
        GLenum primitiveType = filled ? GL_TRIANGLES : GL_LINES;

        // Flush if primitive type changes
        if (!batchVertices_.empty() && currentBatchPrimitiveType_ != primitiveType)
        {
            flushBatch(currentBatchPrimitiveType_);
        }

        currentBatchPrimitiveType_ = primitiveType;

        auto vertices = generateCircleVertices(center, radius, color, segments, filled);
        for (const auto& vertex : vertices)
        {
            addVertexToBatch(vertex);
        }
    }

    void EnoGraphicsManager::setProjectionMatrix(int windowWidth, int windowHeight)
    {
        setupProjectionMatrix(windowWidth, windowHeight);
    }

    void EnoGraphicsManager::setProjectionMatrix(const float* matrix)
    {
        std::copy(matrix, matrix + 16, projectionMatrix_.begin());
    }

    bool EnoGraphicsManager::createDefaultShader()
    {
        // Simple 2D vertex shader
        std::string vertexShaderSource = R"(
            #version 330 core
            
            layout (location = 0) in vec2 a_position;
            layout (location = 1) in vec4 a_color;
            
            uniform mat4 u_projection;
            uniform float u_pointSize = 1.0;
            
            out vec4 v_color;
            
            void main()
            {
                gl_Position = u_projection * vec4(a_position, 0.0, 1.0);
                gl_PointSize = u_pointSize;
                v_color = a_color;
            }
        )";

        // Simple fragment shader
        std::string fragmentShaderSource = R"(
            #version 330 core
            
            in vec4 v_color;
            out vec4 FragColor;
            
            void main()
            {
                FragColor = v_color;
            }
        )";

        defaultShader_ = std::make_unique<EnoShader>();
        return defaultShader_->loadFromStrings(vertexShaderSource, fragmentShaderSource);
    }

    void EnoGraphicsManager::setupProjectionMatrix(int width, int height)
    {
        // Create orthographic projection matrix for 2D rendering
        // This maps screen coordinates to normalized device coordinates (-1 to 1)
        float left = 0.0f;
        float right = static_cast<float>(width);
        float bottom = static_cast<float>(height);
        float top = 0.0f;
        float near = -1.0f;
        float far = 1.0f;

        // Fill projection matrix column-major order because openGL uses this shi
        projectionMatrix_.fill(0.0f);
        projectionMatrix_[0] = 2.0f / (right - left);
        projectionMatrix_[5] = 2.0f / (top - bottom);
        projectionMatrix_[10] = -2.0f / (far - near);
        projectionMatrix_[12] = -(right + left) / (right - left);
        projectionMatrix_[13] = -(top + bottom) / (top - bottom);
        projectionMatrix_[14] = -(far + near) / (far - near);
        projectionMatrix_[15] = 1.0f;
    }
    // heheheha
    void EnoGraphicsManager::flushBatch(GLenum primitiveType)
    {
        if (batchVertices_.empty()) return;

        vertexBuffer_->uploadData(batchVertices_);

        defaultShader_->use();
        defaultShader_->setMatrix4("u_projection", projectionMatrix_.data());

        vertexBuffer_->draw(primitiveType);

        defaultShader_->unuse();

        batchVertices_.clear();
        currentBatchPrimitiveType_ = primitiveType;  // Track what we just flushed
    }

    void EnoGraphicsManager::addVertexToBatch(const Vertex& vertex)
    {
        batchVertices_.push_back(vertex);

        // Auto-flush if we're approaching batch size limit
        if (batchVertices_.size() >= MAX_BATCH_SIZE - 3)  // Leave room for a triangle
        {
            flushBatch(GL_TRIANGLES);
        }
    }

    std::vector<Vertex> EnoGraphicsManager::generateCircleVertices(const Math::eVec2<float>& center, float radius, const Color& color, int segments, bool filled)
    {
        std::vector<Vertex> vertices;

        if (filled)
        {
            // Generate triangles (not triangle fan) for batching
            for (int i = 0; i < segments; ++i)
            {
                float angle1 = 2.0f * M_PI * i / segments;
                float angle2 = 2.0f * M_PI * (i + 1) / segments;

                Math::eVec2<float> point1{
                    center.v1 + radius * cos(angle1),
                    center.v2 + radius * sin(angle1)
                };
                Math::eVec2<float> point2{
                    center.v1 + radius * cos(angle2),
                    center.v2 + radius * sin(angle2)
                };

                // Create triangle: center -> point1 -> point2
                vertices.push_back(Vertex(center, color));
                vertices.push_back(Vertex(point1, color));
                vertices.push_back(Vertex(point2, color));
            }
        }
        else
        {
            // Outline code is correct
            for (int i = 0; i < segments; ++i)
            {
                float angle1 = 2.0f * M_PI * i / segments;
                float angle2 = 2.0f * M_PI * (i + 1) / segments;

                Math::eVec2<float> point1{
                    center.v1 + radius * cos(angle1),
                    center.v2 + radius * sin(angle1)
                };
                Math::eVec2<float> point2{
                    center.v1 + radius * cos(angle2),
                    center.v2 + radius * sin(angle2)
                };

                vertices.push_back(Vertex(point1, color));
                vertices.push_back(Vertex(point2, color));
            }
        }

        return vertices;
    }

    std::vector<Vertex> EnoGraphicsManager::generateRectVertices(const Math::eVec2<float>& position, const Math::eVec2<float>& size, const Color& color, bool filled)
    {
        std::vector<Vertex> vertices;

        Math::eVec2<float> topLeft = position;
        Math::eVec2<float> topRight = { position.v1 + size.v1, position.v2 };
        Math::eVec2<float> bottomRight = { position.v1 + size.v1, position.v2 + size.v2 };
        Math::eVec2<float> bottomLeft = { position.v1, position.v2 + size.v2 };

        if (filled)
        {
            // Two triangles to form a filled rectangle
            // Triangle 1: top-left, top-right, bottom-left
            vertices.push_back(Vertex(topLeft, color));
            vertices.push_back(Vertex(topRight, color));
            vertices.push_back(Vertex(bottomLeft, color));

            // Triangle 2: top-right, bottom-right, bottom-left
            vertices.push_back(Vertex(topRight, color));
            vertices.push_back(Vertex(bottomRight, color));
            vertices.push_back(Vertex(bottomLeft, color));
        }
        else
        {
            // Four lines for rectangle outline
            vertices.push_back(Vertex(topLeft, color));
            vertices.push_back(Vertex(topRight, color));

            vertices.push_back(Vertex(topRight, color));
            vertices.push_back(Vertex(bottomRight, color));

            vertices.push_back(Vertex(bottomRight, color));
            vertices.push_back(Vertex(bottomLeft, color));

            vertices.push_back(Vertex(bottomLeft, color));
            vertices.push_back(Vertex(topLeft, color));
        }

        return vertices;
    }
}