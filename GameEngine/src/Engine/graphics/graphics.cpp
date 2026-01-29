/*============================================================================================
* @file         graphics.cpp
* @author       Law Zong Han (zonghan.law@digipen.edu),
* @author       Muhammad Raziq Bin Mohd Rafig ()
* @date         03-10-2025
* @brief
*//*========================================================================================*/

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "graphics.hpp"
#include <iostream>
#include <cmath>
#include <fstream>

namespace Graphics
{
    // Static color definitions
    const Color Color::White(1.0f, 1.0f, 1.0f, 1.0f);
    const Color Color::Black(0.0f, 0.0f, 0.0f, 1.0f);
    const Color Color::Red(1.0f, 0.0f, 0.0f, 1.0f);
    const Color Color::Green(0.0f, 1.0f, 0.0f, 1.0f);
    const Color Color::Blue(0.0f, 0.0f, 1.0f, 1.0f);

    // =============================================================================
    // EnoTexture Implementation - Texture loading and management
    // =============================================================================

    EnoTexture::EnoTexture() : textureID_(0), width_(0), height_(0), channels_(0) 
    { 

    }

    EnoTexture::~EnoTexture()
    {
        if (textureID_ != 0)
        {
            glDeleteTextures(1, &textureID_);
        }
    }

    bool EnoTexture::loadFromFile(const std::string& filepath)
    {
        std::cout << "Attempting to load texture from: " << filepath << std::endl;

        // Check if file exists first
        std::ifstream fileCheck(filepath);
        if (!fileCheck.good())
        {
            std::cerr << "File does not exist or cannot be opened: " << filepath << std::endl;
            return false;
        }
        fileCheck.close();

        // Check if texture already has a shared pointer mapped

        std::cout << "File exists, attempting stbi_load..." << std::endl;

        // Load image using stb_image
        stbi_set_flip_vertically_on_load(false);
        unsigned char* data = stbi_load(filepath.c_str(), &width_, &height_, &channels_, 0);

        std::cout << "stbi_load returned, checking data..." << std::endl;

        if (!data)
        {
            const char* reason = stbi_failure_reason();
            std::cerr << "Failed to load texture: " << filepath << std::endl;
            std::cerr << "STB Image Error: " << (reason ? reason : "Unknown error") << std::endl;
            return false;
        }

        std::cout << "Data loaded successfully: " << width_ << "x" << height_ << " with " << channels_ << " channels" << std::endl;

        // Generate OpenGL texture
        std::cout << "Generating OpenGL texture..." << std::endl;
        glGenTextures(1, &textureID_);
        std::cout << "Texture ID: " << textureID_ << std::endl;

        glBindTexture(GL_TEXTURE_2D, textureID_);

        // Set texture parameters for sprite sheets
        std::cout << "Setting texture parameters..." << std::endl;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  // Prevent wrapping
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);  // Prevent wrapping
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);    // No interpolation
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);    // No interpolation

        // Determine format based on channels
        std::cout << "Determining texture format..." << std::endl;
        GLenum format = GL_RGB;
        if (channels_ == 1)
            format = GL_RED;
        else if (channels_ == 3)
            format = GL_RGB;
        else if (channels_ == 4)
            format = GL_RGBA;

        std::cout << "Format determined (format=" << format << "), uploading texture data..." << std::endl;

        // Upload texture data
        glTexImage2D(GL_TEXTURE_2D, 0, format, width_, height_, 0, format, GL_UNSIGNED_BYTE, data);

        std::cout << "Freeing image data..." << std::endl;

        if (data != nullptr)
        {
            stbi_image_free(data);
            std::cout << "Image data freed successfully" << std::endl;
        }

        std::cout << "Unbinding texture..." << std::endl;
        glBindTexture(GL_TEXTURE_2D, 0);

        std::cout << "Loaded texture successfully: " << filepath << " (" << width_ << "x" << height_ << ", " << channels_ << " channels)" << std::endl;
        return true;
    }

    bool EnoTexture::loadFromMemory(int width, int height, int channels, const unsigned char* data) {
        width_ = width;
        height_ = height;
        channels_ = channels;

        glGenTextures(1, &textureID_);
        glBindTexture(GL_TEXTURE_2D, textureID_);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        GLenum format = GL_RGB;

        if (channels_ == 1)
            format = GL_RED;
        else if (channels_ == 3)
            format = GL_RGB;
        else if (channels_ == 4)
            format = GL_RGBA;

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

        glBindTexture(GL_TEXTURE_2D, 0);
        return true;
    }


    void EnoTexture::bind(unsigned int slot) const
    {
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, textureID_);
    }

    void EnoTexture::unbind() const
    {
        glBindTexture(GL_TEXTURE_2D, 0);
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

    void EnoShader::setInt(const std::string& name, int value) const
    {
        GLint location = getUniformLocation(name);
        if (location != -1)
        {
            glUniform1i(location, value);
        }
    }

    void EnoShader::setIntArray(const std::string& name, int* values, int count) const
    {
        GLint location = getUniformLocation(name);
        if (location != -1)
        {
            glUniform1iv(location, count, values);
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
        glGenVertexArrays(1, &VAO_);
        glGenBuffers(1, &VBO_);

        setupVertexAttributes();

        // PRE-ALLOCATE BUFFER FOR BETTER PERFORMANCE
        bind();
        glBufferData(GL_ARRAY_BUFFER,
            50000 * sizeof(Vertex),  // MAX_BATCH_SIZE
            nullptr,
            GL_DYNAMIC_DRAW);
        unbind();
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
        // USE glBufferSubData instead of glBufferData (faster!)
        glBufferSubData(GL_ARRAY_BUFFER, 0,
            vertices.size() * sizeof(Vertex),
            vertices.data());
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

        // TexCoords attribute (location 2)
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));
        glEnableVertexAttribArray(2);

        // TexID attribute (location 3)
        glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texID));
        glEnableVertexAttribArray(3);

        unbind();
    }

    // =============================================================================
    // EnoGraphicsManager Implementation - Main graphics interface
    // =============================================================================

    EnoGraphicsManager::EnoGraphicsManager(Event::EnoEventBus& bus)
        : bus_(bus), initialized_(false)
    {
        // Reserve space for batch rendering
        batchVertices_.reserve(MAX_BATCH_SIZE);
        boundTextures_.reserve(MAX_TEXTURE_SLOTS);

        // Initialize projection matrix as identity
        projectionMatrix_.fill(0.0f);
        projectionMatrix_[0] = projectionMatrix_[5] = projectionMatrix_[10] = projectionMatrix_[15] = 1.0f;
    }

    EnoGraphicsManager::~EnoGraphicsManager()
    {
        shutdown();
    }


    void EnoGraphicsManager::initialize(Window::EnoWindow& window)
    {
        if (initialized_) return;

        // Set up basic OpenGL state for 2D rendering
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Enable point size control from shaders for drawPoint
        glEnable(GL_PROGRAM_POINT_SIZE); // Can disable if not needed

        // Create default shader for primitive rendering
        if (!createDefaultShader())
        {
            throw std::runtime_error("Failed to create default shader!");
        }

        // Create shaders when shaders is loaded in resource loader
        bus_.subscribe<Events::ShaderLoaded>(
            [this](const Events::ShaderLoaded& e) {
                if (!createShader(e.name, e.vertexSource, e.fragmentSource)) {
                    throw std::runtime_error("Failed to create loaded shader!");
                };
            }
        );

        bus_.subscribe<TextureMapping>(
            [this](const TextureMapping& e) {
                mapTexture(e);
            }
        );

        bus_.subscribe<Window::FramebufferResizeEvent>(
            [this](const Window::FramebufferResizeEvent& e) {
                // Update viewport
                glViewport(0, 0, e.width, e.height);

                // Update projection matrix
                setupProjectionMatrix(e.width, e.height);
            }
        );

        // Create vertex buffer for batch rendering
        vertexBuffer_ = std::make_unique<EnoVertexBuffer>();

        // Set up initial projection matrix
        int width = window.getFramebufferSize().first;
        int height = window.getFramebufferSize().second;

        glViewport(0, 0, width, height);
        setupProjectionMatrix(width, height);

        // Push projection matrix to default shader
        if (defaultShader_ && defaultShader_->isValid())
        {
            defaultShader_->use();
            defaultShader_->setMatrix4("u_projection", projectionMatrix_.data());
            defaultShader_->unuse();
        }

        initialized_ = true;

#ifdef _DEBUG
        std::cout << "Graphics system initialized successfully!" << std::endl;
#endif
    }

    void EnoGraphicsManager::shutdown()
    {
        defaultShader_.reset();
        vertexBuffer_.reset();
        boundTextures_.clear();
        texMap_.clear();
        initialized_ = false;
    }

    void EnoGraphicsManager::beginFrame(const Color& clearColor)
    {
        // Clear the screen
        glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
        glClear(GL_COLOR_BUFFER_BIT);;

        // Clear batch for new frame
        batchVertices_.clear();
        boundTextures_.clear();

        // BIND SHADER ONCE FOR ENTIRE FRAME
        if (defaultShader_ && defaultShader_->isValid()) {
            defaultShader_->use();
            defaultShader_->setMatrix4("u_projection", projectionMatrix_.data());
        }
    }

    void EnoGraphicsManager::endFrame(Window::EnoWindow& window)
    {
        // Flush any remaining batched geometry
        if (!batchVertices_.empty())
        {
            flushBatch(currentBatchPrimitiveType_);
        }

        // UNBIND SHADER ONCE AT END
        if (defaultShader_ && defaultShader_->isValid()) {
            defaultShader_->unuse();
        }

        window.swapBuffers();
    }

    // Map texture
    void EnoGraphicsManager::mapTexture(const TextureMapping& e)
    {
        auto it = texMap_.find(e.path);
        
        // Texture shared pointer does not exist
        // Inserting
        if (it == texMap_.end())
        {
            texMap_[e.path] = e.tex;
        }
    }

    // Helper function for rotation
    static Math::eVec2<float> rotatePoint(const Math::eVec2<float>& point,
        const Math::eVec2<float>& center,
        float rotation)
    {
        if (rotation == 0.0f) return point;

        float cosR = cos(rotation);
        float sinR = sin(rotation);

        // Translate to origin
        float x = point.x - center.x;
        float y = point.y - center.y;

        // Rotate
        float xNew = x * cosR - y * sinR;
        float yNew = x * sinR + y * cosR;

        // Translate back
        return Math::eVec2<float>{xNew + center.x, yNew + center.y};
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
        (void)thickness;

        // Simple line implementation
        GLenum primitiveType = GL_LINES;

        // Flush if primitive type changes
        if (!batchVertices_.empty() && currentBatchPrimitiveType_ != primitiveType)
        {
            flushBatch(currentBatchPrimitiveType_);
        }

        currentBatchPrimitiveType_ = primitiveType;

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

    void EnoGraphicsManager::drawRect(const Math::eVec2<float>& position, const Math::eVec2<float>& size, const Color& color, bool filled, float rotation)
    {
        GLenum primitiveType = filled ? GL_TRIANGLES : GL_LINES;

        if (!batchVertices_.empty() && currentBatchPrimitiveType_ != primitiveType)
        {
            flushBatch(currentBatchPrimitiveType_);
        }

        currentBatchPrimitiveType_ = primitiveType;

        // Calculate center for rotation
        Math::eVec2<float> center = { position.x + size.x * 0.5f, position.y + size.y * 0.5f };

        Math::eVec2<float> topLeft = position;
        Math::eVec2<float> topRight = { position.x + size.x, position.y };
        Math::eVec2<float> bottomRight = { position.x + size.x, position.y + size.y };
        Math::eVec2<float> bottomLeft = { position.x, position.y + size.y };

        // Apply rotation if needed
        if (rotation != 0.0f)
        {
            topLeft = rotatePoint(topLeft, center, rotation);
            topRight = rotatePoint(topRight, center, rotation);
            bottomRight = rotatePoint(bottomRight, center, rotation);
            bottomLeft = rotatePoint(bottomLeft, center, rotation);
        }

        if (filled)
        {
            // Reserve space for 6 vertices (2 triangles)
            batchVertices_.reserve(batchVertices_.size() + 6);

            batchVertices_.emplace_back(topLeft, color);
            batchVertices_.emplace_back(topRight, color);
            batchVertices_.emplace_back(bottomLeft, color);

            batchVertices_.emplace_back(topRight, color);
            batchVertices_.emplace_back(bottomRight, color);
            batchVertices_.emplace_back(bottomLeft, color);
        }
        else
        {
            // Reserve space for 8 vertices (4 lines)
            batchVertices_.reserve(batchVertices_.size() + 8);

            batchVertices_.emplace_back(topLeft, color);
            batchVertices_.emplace_back(topRight, color);

            batchVertices_.emplace_back(topRight, color);
            batchVertices_.emplace_back(bottomRight, color);

            batchVertices_.emplace_back(bottomRight, color);
            batchVertices_.emplace_back(bottomLeft, color);

            batchVertices_.emplace_back(bottomLeft, color);
            batchVertices_.emplace_back(topLeft, color);
        }

        // Check if we need to flush
        if (batchVertices_.size() >= MAX_BATCH_SIZE - 8)
        {
            flushBatch(currentBatchPrimitiveType_);
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

        if (filled)
        {
            // Reserve space for triangles (3 vertices per segment)
            batchVertices_.reserve(batchVertices_.size() + (segments * 3));

            // Generate triangles directly into batch
            for (int i = 0; i < segments; ++i)
            {
                float angle1 = static_cast<float>(2.0f * M_PI * static_cast<float>(i) / static_cast<float>(segments));
                float angle2 = static_cast<float>(2.0f * M_PI * static_cast<float>(i + 1) / static_cast<float>(segments));

                Math::eVec2<float> point1{
                    center.x + radius * cos(angle1),
                    center.y + radius * sin(angle1)
                };
                Math::eVec2<float> point2{
                    center.x + radius * cos(angle2),
                    center.y + radius * sin(angle2)
                };

                // Create triangle: center -> point1 -> point2
                batchVertices_.emplace_back(center, color);
                batchVertices_.emplace_back(point1, color);
                batchVertices_.emplace_back(point2, color);
            }
        }
        else
        {
            // Reserve space for lines (2 vertices per segment)
            batchVertices_.reserve(batchVertices_.size() + (segments * 2));

            for (int i = 0; i < segments; ++i)
            {
                float angle1 = static_cast<float>(2.0f * M_PI * static_cast<float>(i) / static_cast<float>(segments));
                float angle2 = static_cast<float>(2.0f * M_PI * static_cast<float>(i + 1) / static_cast<float>(segments));

                Math::eVec2<float> point1{
                    center.x + radius * cos(angle1),
                    center.y + radius * sin(angle1)
                };
                Math::eVec2<float> point2{
                    center.x + radius * cos(angle2),
                    center.y + radius * sin(angle2)
                };

                batchVertices_.emplace_back(point1, color);
                batchVertices_.emplace_back(point2, color);
            }
        }

        // Check if we need to flush
        if (batchVertices_.size() >= MAX_BATCH_SIZE - (segments * 3))
        {
            flushBatch(currentBatchPrimitiveType_);
        }
    }

    void EnoGraphicsManager::drawTexturedQuad(const Math::eVec2<float>& position,
        const Math::eVec2<float>& size,
        std::shared_ptr<EnoTexture> texture,
        const Color& tint,
        float rotation)
    {
        drawTexturedQuadUV(position, size, texture, { 0.0f, 0.0f }, { 1.0f, 1.0f }, tint, rotation);
    }

    void EnoGraphicsManager::drawTexturedQuadUV(const Math::eVec2<float>& position,
        const Math::eVec2<float>& size,
        std::shared_ptr<EnoTexture> texture,
        const Math::eVec2<float>& uvStart,
        const Math::eVec2<float>& uvEnd,
        const Color& tint,
        float rotation)
    {
        if (!texture || !texture->isValid())
        {
            std::cerr << "Invalid texture provided to drawTexturedQuad!" << std::endl;
            return;
        }

        if (!batchVertices_.empty() && currentBatchPrimitiveType_ != GL_TRIANGLES)
        {
            flushBatch(currentBatchPrimitiveType_);
        }

        currentBatchPrimitiveType_ = GL_TRIANGLES;

        int texSlot = getTextureSlot(texture);
        float texID = static_cast<float>(texSlot);

        // Calculate center for rotation
        Math::eVec2<float> center = { position.x + size.x * 0.5f, position.y + size.y * 0.5f };

        Math::eVec2<float> topLeft = position;
        Math::eVec2<float> topRight = { position.x + size.x, position.y };
        Math::eVec2<float> bottomRight = { position.x + size.x, position.y + size.y };
        Math::eVec2<float> bottomLeft = { position.x, position.y + size.y };

        // Apply rotation if needed
        if (rotation != 0.0f)
        {
            topLeft = rotatePoint(topLeft, center, rotation);
            topRight = rotatePoint(topRight, center, rotation);
            bottomRight = rotatePoint(bottomRight, center, rotation);
            bottomLeft = rotatePoint(bottomLeft, center, rotation);
        }

        // UV coordinates
        Math::eVec2<float> uvTopLeft = { uvStart.x, uvStart.y };
        Math::eVec2<float> uvTopRight = { uvEnd.x, uvStart.y };
        Math::eVec2<float> uvBottomRight = { uvEnd.x, uvEnd.y };
        Math::eVec2<float> uvBottomLeft = { uvStart.x, uvEnd.y };

        // Reserve space for 6 vertices
        batchVertices_.reserve(batchVertices_.size() + 6);

        batchVertices_.emplace_back(topLeft, tint, uvTopLeft, texID);
        batchVertices_.emplace_back(topRight, tint, uvTopRight, texID);
        batchVertices_.emplace_back(bottomLeft, tint, uvBottomLeft, texID);

        batchVertices_.emplace_back(topRight, tint, uvTopRight, texID);
        batchVertices_.emplace_back(bottomRight, tint, uvBottomRight, texID);
        batchVertices_.emplace_back(bottomLeft, tint, uvBottomLeft, texID);

        // Check if we need to flush
        if (batchVertices_.size() >= MAX_BATCH_SIZE - 6)
        {
            flushBatch(currentBatchPrimitiveType_);
        }
    }

    void EnoGraphicsManager::drawText(const std::shared_ptr<Font::EnoFont>& font,
        const std::string& text,
        const Math::eVec2<float>& position,
        float scale,
        const Color& color)
    {
        if (!font || !font->getAtlasTexture() || !font->getAtlasTexture()->isValid()) {
            throw std::runtime_error("Invalid font atlas");
        }

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        EnoShader* shdr = getShader("text");

        if (!shdr || !shdr->isValid()) {
            throw std::runtime_error("Text shader not loaded properly");
        }

        shdr->use();
        // Uses whatever projectionMatrix_ is currently set (world / camera)
        shdr->setMatrix4("u_projection", projectionMatrix_.data());

        auto atlas = font->getAtlasTexture();
        atlas->bind(0);
        shdr->setInt("u_text", 0);

        float xCursor = position.x;
        float yCursor = position.y;

        for (char c : text) {
            try {
                const Font::Character& ch = font->getChar(c);

                // Horizontal: still respect bearing.x
                float xPos = xCursor + ch.bearing.x * scale;

                // Vertical: treat position.y as the TOP of glyph box in our
                // top-left origin system. This avoids "flying off the screen".
                float yPos = yCursor - ch.size.y * scale;

                Math::eVec2<float> charPos(xPos, yPos);
                Math::eVec2<float> charSize(ch.size.x * scale, ch.size.y * scale);

                // Quad corners
                Math::eVec2<float> topLeft = charPos;
                Math::eVec2<float> topRight = { charPos.x + charSize.x, charPos.y };
                Math::eVec2<float> bottomRight = { charPos.x + charSize.x, charPos.y + charSize.y };
                Math::eVec2<float> bottomLeft = { charPos.x,               charPos.y + charSize.y };

                // UV coordinates (assuming font atlas is bottom-left origin now)
                Math::eVec2<float> uvTopLeft = { ch.uvStart.x, ch.uvStart.y };
                Math::eVec2<float> uvTopRight = { ch.uvEnd.x,   ch.uvStart.y };
                Math::eVec2<float> uvBottomRight = { ch.uvEnd.x,   ch.uvEnd.y };
                Math::eVec2<float> uvBottomLeft = { ch.uvStart.x, ch.uvEnd.y };

                std::vector<Vertex> vertices;
                vertices.reserve(6);

                vertices.emplace_back(topLeft, color, uvTopLeft, 0.f);
                vertices.emplace_back(topRight, color, uvTopRight, 0.f);
                vertices.emplace_back(bottomLeft, color, uvBottomLeft, 0.f);

                vertices.emplace_back(topRight, color, uvTopRight, 0.f);
                vertices.emplace_back(bottomRight, color, uvBottomRight, 0.f);
                vertices.emplace_back(bottomLeft, color, uvBottomLeft, 0.f);

                vertexBuffer_->uploadData(vertices);
                vertexBuffer_->draw(GL_TRIANGLES);

                // Advance cursor (FreeType advance is in 1/64 pixels)
                xCursor += (ch.advance >> 6) * scale;
            }
            catch (const std::out_of_range&) {
                // Character wasn’t in the atlas, skip it
                continue;
            }
        }

        atlas->unbind();
        shdr->unuse();
    }

    void EnoGraphicsManager::drawTextScreen(const std::shared_ptr<Font::EnoFont>& font,
        const std::string& text,
        const Math::eVec2<float>& position,
        float scale,
        const Color& color,
        float screenWidth,
        float screenHeight)
    {
        if (!font || !font->getAtlasTexture() || !font->getAtlasTexture()->isValid()) {
            throw std::runtime_error("Invalid font atlas");
        }

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        EnoShader* shdr = getShader("text");
        if (!shdr || !shdr->isValid()) {
            throw std::runtime_error("Text shader not loaded properly");
        }

        // Build a dedicated screen-space projection:
        // (0,0)   -> top-left
        // (screenWidth, screenHeight) -> bottom-right
        std::array<float, 16> uiProj{};
        float left = 0.0f;
        float right = screenWidth;
        float bottom = screenHeight;
        float top = 0.0f;
        float near = -1.0f;
        float far = 1.0f;

        uiProj.fill(0.0f);
        uiProj[0] = 2.0f / (right - left);
        uiProj[5] = 2.0f / (top - bottom);
        uiProj[10] = -2.0f / (far - near);
        uiProj[12] = -(right + left) / (right - left);
        uiProj[13] = -(top + bottom) / (top - bottom);
        uiProj[15] = 1.0f;

        shdr->use();
        shdr->setMatrix4("u_projection", uiProj.data());

        auto atlas = font->getAtlasTexture();
        atlas->bind(0);
        shdr->setInt("u_text", 0);

        float xCursor = position.x;
        float yCursor = position.y;

        for (char c : text) {
            try {
                const Font::Character& ch = font->getChar(c);

                float xPos = xCursor + ch.bearing.x * scale;
                float yPos = yCursor - ch.size.y * scale;

                Math::eVec2<float> charPos(xPos, yPos);
                Math::eVec2<float> charSize(ch.size.x * scale, ch.size.y * scale);

                Math::eVec2<float> topLeft = charPos;
                Math::eVec2<float> topRight = { charPos.x + charSize.x, charPos.y };
                Math::eVec2<float> bottomRight = { charPos.x + charSize.x, charPos.y + charSize.y };
                Math::eVec2<float> bottomLeft = { charPos.x,               charPos.y + charSize.y };

                Math::eVec2<float> uvTopLeft = { ch.uvStart.x, ch.uvStart.y };
                Math::eVec2<float> uvTopRight = { ch.uvEnd.x,   ch.uvStart.y };
                Math::eVec2<float> uvBottomRight = { ch.uvEnd.x,   ch.uvEnd.y };
                Math::eVec2<float> uvBottomLeft = { ch.uvStart.x, ch.uvEnd.y };

                std::vector<Vertex> vertices;
                vertices.reserve(6);

                vertices.emplace_back(topLeft, color, uvTopLeft, 0.f);
                vertices.emplace_back(topRight, color, uvTopRight, 0.f);
                vertices.emplace_back(bottomLeft, color, uvBottomLeft, 0.f);

                vertices.emplace_back(topRight, color, uvTopRight, 0.f);
                vertices.emplace_back(bottomRight, color, uvBottomRight, 0.f);
                vertices.emplace_back(bottomLeft, color, uvBottomLeft, 0.f);

                vertexBuffer_->uploadData(vertices);
                vertexBuffer_->draw(GL_TRIANGLES);

                xCursor += (ch.advance >> 6) * scale;
            }
            catch (const std::out_of_range&) {
                continue;
            }
        }

        atlas->unbind();
        shdr->unuse();
    }

    void EnoGraphicsManager::setProjectionMatrix(int windowWidth, int windowHeight)
    {
        setupProjectionMatrix(windowWidth, windowHeight);

        // Update the shader uniform with the new projection matrix
        if (defaultShader_ && defaultShader_->isValid()) {
            defaultShader_->use();
            defaultShader_->setMatrix4("u_projection", projectionMatrix_.data());
        }
    }

    void EnoGraphicsManager::setProjectionMatrix(const float* matrix)
    {
        std::copy(matrix, matrix + 16, projectionMatrix_.begin());

        // Update the shader uniform with the new projection matrix
        if (defaultShader_ && defaultShader_->isValid()) {
            defaultShader_->use();
            defaultShader_->setMatrix4("u_projection", projectionMatrix_.data());
        }
    }

    bool EnoGraphicsManager::createDefaultShader()
    {
        // Vertex shader with texture support
        std::string vertexShaderSource = R"(
            #version 450 core
            
            layout (location = 0) in vec2 a_position;
            layout (location = 1) in vec4 a_color;
            layout (location = 2) in vec2 a_texCoords;
            layout (location = 3) in float a_texID;
            
            uniform mat4 u_projection;
            uniform float u_pointSize = 1.0;
            
            out vec4 v_color;
            out vec2 v_texCoords;
            out float v_texID;
            
            void main()
            {
                gl_Position = u_projection * vec4(a_position, 0.0, 1.0);
                gl_PointSize = u_pointSize;
                v_color = a_color;
                v_texCoords = a_texCoords;
                v_texID = a_texID;
            }
        )";

        // Fragment shader with texture support
        std::string fragmentShaderSource = R"(
            #version 450 core
            
            in vec4 v_color;
            in vec2 v_texCoords;
            in float v_texID;
            
            uniform sampler2D u_textures[32];
            
            out vec4 FragColor;
            
            void main()
            {
                int texIndex = int(v_texID);
                
                // If texID is 0 and texCoords are 0, it's a non-textured primitive
                if (v_texID < 0.5 && length(v_texCoords) < 0.01)
                {
                    FragColor = v_color;
                }
                else
                {
                    vec4 texColor = texture(u_textures[texIndex], v_texCoords);
                    FragColor = texColor * v_color;
                }
            }
        )";

        defaultShader_ = std::make_unique<EnoShader>();
        if (!defaultShader_->loadFromStrings(vertexShaderSource, fragmentShaderSource))
        {
            return false;
        }

        // Set up texture samplers
        defaultShader_->use();
        int samplers[MAX_TEXTURE_SLOTS];
        for (int i = 0; i < MAX_TEXTURE_SLOTS; ++i)
        {
            samplers[i] = i;
        }
        defaultShader_->setIntArray("u_textures", samplers, MAX_TEXTURE_SLOTS);
        defaultShader_->unuse();

        return true;
    }

    bool EnoGraphicsManager::createShader(const std::string& name, const std::string& vertSrc, const std::string& fragSrc) {

        auto shdr = std::make_unique<EnoShader>();
        if (!shdr->loadFromStrings(vertSrc, fragSrc)) return false;

        shadersMap_[name] = std::move(shdr);
        return true;
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

    void EnoGraphicsManager::flushBatch(GLenum primitiveType)
    {
        if (batchVertices_.empty()) return;

        // Bind all textures
        bindTextures();

        vertexBuffer_->uploadData(batchVertices_);

        // SHADER ALREADY BOUND - Just draw
        vertexBuffer_->draw(primitiveType);

        batchVertices_.clear();
        currentBatchPrimitiveType_ = primitiveType;
    }

    void EnoGraphicsManager::addVertexToBatch(const Vertex& vertex)
    {
        batchVertices_.push_back(vertex);

        // Auto-flush if we're approaching batch size limit
        if (batchVertices_.size() >= MAX_BATCH_SIZE - 6)  // Leave room for a quad
        {
            flushBatch(currentBatchPrimitiveType_);
        }
    }

    EnoShader* EnoGraphicsManager::getShader(const std::string& name) {
        if (name == "default") return defaultShader_.get();

        auto it = shadersMap_.find(name);
        if (it != shadersMap_.end()) {
            return it->second.get();
        }

        std::cerr << "[GraphicsManager] Shader '" << name
            << "' not found, using default shader.\n";
        return defaultShader_.get();
    }

    int EnoGraphicsManager::getTextureSlot(std::shared_ptr<EnoTexture> texture)
    {
        // Check if texture is already bound
        for (size_t i = 0; i < boundTextures_.size(); ++i)
        {
            if (boundTextures_[i] == texture)
            {
                return static_cast<int>(i);
            }
        }

        // If we've reached max slots, flush and reset
        if (boundTextures_.size() >= MAX_TEXTURE_SLOTS)
        {
            flushBatch(currentBatchPrimitiveType_);
            boundTextures_.clear();
        }

        // Add texture to bound list
        boundTextures_.push_back(texture);
        return static_cast<int>(boundTextures_.size() - 1);
    }

    void EnoGraphicsManager::bindTextures()
    {
        for (size_t i = 0; i < boundTextures_.size(); ++i)
        {
            if (boundTextures_[i] && boundTextures_[i]->isValid())
            {
                boundTextures_[i]->bind(static_cast<unsigned int>(i));
            }
        }
    }

    EnoFramebuffer::EnoFramebuffer()
        : fbo_(0), textureID_(0), rbo_(0), width_(0), height_(0) {
    }

    EnoFramebuffer::~EnoFramebuffer() {
        if (fbo_) glDeleteFramebuffers(1, &fbo_);
        if (textureID_) glDeleteTextures(1, &textureID_);
        if (rbo_) glDeleteRenderbuffers(1, &rbo_);
    }

    bool EnoFramebuffer::create(int width, int height) {
        width_ = width;
        height_ = height;

        // Create framebuffer
        glGenFramebuffers(1, &fbo_);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_);

        // Create color texture
        glGenTextures(1, &textureID_);
        glBindTexture(GL_TEXTURE_2D, textureID_);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureID_, 0);

        // Create renderbuffer for depth/stencil
        glGenRenderbuffers(1, &rbo_);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo_);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo_);

        // Check framebuffer is complete
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "Framebuffer not complete!" << std::endl;
            return false;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return true;
    }

    void EnoFramebuffer::bind() const {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
        glViewport(0, 0, width_, height_);
    }

    void EnoFramebuffer::unbind() const {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void EnoFramebuffer::resize(int width, int height) {
        if (width_ == width && height_ == height) return;

        // Cleanup old resources
        if (textureID_) glDeleteTextures(1, &textureID_);
        if (rbo_) glDeleteRenderbuffers(1, &rbo_);

        // Recreate with new size
        width_ = width;
        height_ = height;

        glBindFramebuffer(GL_FRAMEBUFFER, fbo_);

        // Recreate texture
        glGenTextures(1, &textureID_);
        glBindTexture(GL_TEXTURE_2D, textureID_);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureID_, 0);

        // Recreate renderbuffer
        glGenRenderbuffers(1, &rbo_);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo_);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo_);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}