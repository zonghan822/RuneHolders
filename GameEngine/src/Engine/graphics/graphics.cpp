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

    EnoTexture::EnoTexture() : textureID_(0), width_(0), height_(0), channels_(0) { /*empty by design*/ }

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

        // CRITICAL: Set texture parameters for sprite sheets
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

        // DO NOT generate mipmaps for sprite sheets - this causes bleeding!
        // glGenerateMipmap(GL_TEXTURE_2D);  // KEEP THIS COMMENTED OUT

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

        GLenum format = (channels == 1 ? GL_RED : GL_RGBA);
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


    bool EnoGraphicsManager::initialize(Window::EnoWindow& window)
    {
        if (initialized_) return true;

        // Set up basic OpenGL state for 2D rendering
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Enable point size control from shaders for drawPoint
        glEnable(GL_PROGRAM_POINT_SIZE); // Can disable if not needed

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
        glfwGetFramebufferSize(window.getWindow(), &width, &height);
        setupProjectionMatrix(width, height);

        initialized_ = true;

#ifdef _DEBUG
        std::cout << "Graphics system initialized successfully!" << std::endl;
#endif
        return true;
    }

    void EnoGraphicsManager::shutdown()
    {
        defaultShader_.reset();
        vertexBuffer_.reset();
        boundTextures_.clear();
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

        // Publish frame begin event
        bus_.publish(Events::RenderFrameBegin{});
    }

    void EnoGraphicsManager::endFrame(Window::EnoWindow& window)
    {
        if (!batchVertices_.empty())
        {
            flushBatch(currentBatchPrimitiveType_);  // Use tracked type
        }

        glfwSwapBuffers(window.getWindow());
        bus_.publish(Events::RenderFrameEnd{});
    }

    Math::eVec2<float> rotatePoint(const Math::eVec2<float>& point,
        const Math::eVec2<float>& center,
        float rotation)
    {
        if (rotation == 0.0f) return point;

        float cosR = cos(rotation);
        float sinR = sin(rotation);

        // Translate to origin
        float x = point.v1 - center.v1;
        float y = point.v2 - center.v2;

        // Rotate
        float xNew = x * cosR - y * sinR;
        float yNew = x * sinR + y * cosR;

        // Translate back
        return Math::eVec2<float>{xNew + center.v1, yNew + center.v2};
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

    void EnoGraphicsManager::drawRect(const Math::eVec2<float>& position, const Math::eVec2<float>& size, const Color& color, bool filled, float rotation)
    {
        GLenum primitiveType = filled ? GL_TRIANGLES : GL_LINES;

        if (!batchVertices_.empty() && currentBatchPrimitiveType_ != primitiveType)
        {
            flushBatch(currentBatchPrimitiveType_);
        }

        currentBatchPrimitiveType_ = primitiveType;

        auto vertices = generateRectVertices(position, size, color, filled, rotation);
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

        auto vertices = generateTexturedQuadVertices(position, size, tint, static_cast<float>(texSlot), uvStart, uvEnd, rotation);
        for (const auto& vertex : vertices)
        {
            addVertexToBatch(vertex);
        }
    }

    void EnoGraphicsManager::drawText(const std::shared_ptr<Font::EnoFont>& font,
        const std::string& text,
        const Math::eVec2<float>& position,
        float scale,
        const Color& color)
    {
        if (!font || !font->getAtlasTexture() || !font->getAtlasTexture()->isValid()) {
            std::cerr << "Invalid font atlas" << std::endl;
            return;
        }

        auto atlas = font->getAtlasTexture();
        float xCursor = position.v1;
        float yCursor = position.v2;

        for (char c : text) {
            try {
                const Font::Character& ch = font->getChar(c);

                Math::eVec2<float> charPos(
                    xCursor + ch.Bearing.v1 * scale,
                    yCursor - (ch.Size.v2 - ch.Bearing.v2) * scale
                );
                Math::eVec2<float> charSize(ch.Size.v1 * scale, ch.Size.v2 * scale);

                drawTexturedQuadUV(charPos, charSize,
                    atlas,
                    ch.UVStart,
                    ch.UVEnd,
                    color);

                xCursor += (ch.Advance >> 6) * scale; // Advance in 1/64 pixels
            }
            catch (const std::out_of_range&) {
                continue;
            }
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
        // Vertex shader with texture support
        std::string vertexShaderSource = R"(
            #version 330 core
            
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
            #version 330 core
            
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

        defaultShader_->use();
        defaultShader_->setMatrix4("u_projection", projectionMatrix_.data());

        vertexBuffer_->draw(primitiveType);

        defaultShader_->unuse();

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

    std::vector<Vertex> EnoGraphicsManager::generateRectVertices(const Math::eVec2<float>& position,
        const Math::eVec2<float>& size,
        const Color& color,
        bool filled,
        float rotation)
    {
        std::vector<Vertex> vertices;

        // Calculate center for rotation
        Math::eVec2<float> center = { position.v1 + size.v1 * 0.5f, position.v2 + size.v2 * 0.5f };

        Math::eVec2<float> topLeft = position;
        Math::eVec2<float> topRight = { position.v1 + size.v1, position.v2 };
        Math::eVec2<float> bottomRight = { position.v1 + size.v1, position.v2 + size.v2 };
        Math::eVec2<float> bottomLeft = { position.v1, position.v2 + size.v2 };

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
            vertices.push_back(Vertex(topLeft, color));
            vertices.push_back(Vertex(topRight, color));
            vertices.push_back(Vertex(bottomLeft, color));

            vertices.push_back(Vertex(topRight, color));
            vertices.push_back(Vertex(bottomRight, color));
            vertices.push_back(Vertex(bottomLeft, color));
        }
        else
        {
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

    std::vector<Vertex> EnoGraphicsManager::generateTexturedQuadVertices(
        const Math::eVec2<float>& position,
        const Math::eVec2<float>& size,
        const Color& tint,
        float texID,
        const Math::eVec2<float>& uvStart,
        const Math::eVec2<float>& uvEnd,
        float rotation)
    {
        std::vector<Vertex> vertices;

        // Calculate center for rotation
        Math::eVec2<float> center = { position.v1 + size.v1 * 0.5f, position.v2 + size.v2 * 0.5f };

        Math::eVec2<float> topLeft = position;
        Math::eVec2<float> topRight = { position.v1 + size.v1, position.v2 };
        Math::eVec2<float> bottomRight = { position.v1 + size.v1, position.v2 + size.v2 };
        Math::eVec2<float> bottomLeft = { position.v1, position.v2 + size.v2 };

        // Apply rotation if needed
        if (rotation != 0.0f)
        {
            topLeft = rotatePoint(topLeft, center, rotation);
            topRight = rotatePoint(topRight, center, rotation);
            bottomRight = rotatePoint(bottomRight, center, rotation);
            bottomLeft = rotatePoint(bottomLeft, center, rotation);
        }

        // UV coordinates (unchanged)
        Math::eVec2<float> uvTopLeft = { uvStart.v1, uvStart.v2 };
        Math::eVec2<float> uvTopRight = { uvEnd.v1, uvStart.v2 };
        Math::eVec2<float> uvBottomRight = { uvEnd.v1, uvEnd.v2 };
        Math::eVec2<float> uvBottomLeft = { uvStart.v1, uvEnd.v2 };

        vertices.push_back(Vertex(topLeft, tint, uvTopLeft, texID));
        vertices.push_back(Vertex(topRight, tint, uvTopRight, texID));
        vertices.push_back(Vertex(bottomLeft, tint, uvBottomLeft, texID));

        vertices.push_back(Vertex(topRight, tint, uvTopRight, texID));
        vertices.push_back(Vertex(bottomRight, tint, uvBottomRight, texID));
        vertices.push_back(Vertex(bottomLeft, tint, uvBottomLeft, texID));

        return vertices;
    }
}