#pragma once

#include <string>
#include <glm/glm.hpp>

namespace cosmic {
namespace rendering {

/**
 * @brief Shader program management
 *
 * Handles loading, compiling, linking, and using OpenGL shaders
 */
class Shader {
public:
    /**
     * @brief Default constructor
     */
    Shader();

    /**
     * @brief Construct and load shader from files
     * @param vertexPath Path to vertex shader
     * @param fragmentPath Path to fragment shader
     */
    Shader(const std::string& vertexPath, const std::string& fragmentPath);

    /**
     * @brief Destructor - cleanup OpenGL resources
     */
    ~Shader();

    // Prevent copying
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    // Allow moving
    Shader(Shader&& other) noexcept;
    Shader& operator=(Shader&& other) noexcept;

    /**
     * @brief Load shader from source files
     * @param vertexPath Path to vertex shader
     * @param fragmentPath Path to fragment shader
     * @return True if successful
     */
    bool load(const std::string& vertexPath, const std::string& fragmentPath);

    /**
     * @brief Load shader from source strings
     * @param vertexSource Vertex shader source code
     * @param fragmentSource Fragment shader source code
     * @return True if successful
     */
    bool loadFromSource(const std::string& vertexSource,
                        const std::string& fragmentSource);

    /**
     * @brief Use/activate the shader program
     */
    void use() const;

    /**
     * @brief Get OpenGL program ID
     * @return Shader program ID
     */
    unsigned int getProgram() const { return program_; }

    /**
     * @brief Check if shader is valid
     * @return True if shader loaded successfully
     */
    bool isValid() const { return program_ != 0; }

    // Uniform setters
    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setVec2(const std::string& name, const glm::vec2& value) const;
    void setVec2(const std::string& name, float x, float y) const;
    void setVec3(const std::string& name, const glm::vec3& value) const;
    void setVec3(const std::string& name, float x, float y, float z) const;
    void setVec4(const std::string& name, const glm::vec4& value) const;
    void setVec4(const std::string& name, float x, float y, float z, float w) const;
    void setMat2(const std::string& name, const glm::mat2& mat) const;
    void setMat3(const std::string& name, const glm::mat3& mat) const;
    void setMat4(const std::string& name, const glm::mat4& mat) const;

private:
    unsigned int program_;  // OpenGL shader program ID

    /**
     * @brief Compile shader from source
     * @param source Shader source code
     * @param type Shader type (GL_VERTEX_SHADER, GL_FRAGMENT_SHADER)
     * @return Shader ID, or 0 on failure
     */
    unsigned int compileShader(const std::string& source, unsigned int type);

    /**
     * @brief Link compiled shaders into program
     * @param vertexShader Compiled vertex shader ID
     * @param fragmentShader Compiled fragment shader ID
     * @return Program ID, or 0 on failure
     */
    unsigned int linkProgram(unsigned int vertexShader, unsigned int fragmentShader);

    /**
     * @brief Read file contents
     * @param filePath Path to file
     * @return File contents as string
     */
    std::string readFile(const std::string& filePath) const;

    /**
     * @brief Check for shader compilation errors
     * @param shader Shader ID
     * @param type Shader type name for error messages
     * @return True if successful
     */
    bool checkCompileErrors(unsigned int shader, const std::string& type);

    /**
     * @brief Check for program linking errors
     * @param program Program ID
     * @return True if successful
     */
    bool checkLinkErrors(unsigned int program);
};

} // namespace rendering
} // namespace cosmic
