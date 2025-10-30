#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace cosmic {
namespace rendering {

/**
 * @brief First-person camera for 3D navigation
 *
 * Supports WASD movement, mouse look, and Q/E for vertical movement
 */
class Camera {
public:
    /**
     * @brief Construct camera
     * @param position Initial position
     * @param fov Field of view in degrees
     * @param aspect Aspect ratio (width/height)
     * @param nearPlane Near clipping plane
     * @param farPlane Far clipping plane
     */
    Camera(const glm::vec3& position = glm::vec3(0.0f, 0.0f, 20.0f),
           float fov = 45.0f,
           float aspect = 16.0f / 9.0f,
           float nearPlane = 0.1f,
           float farPlane = 1000.0f);

    /**
     * @brief Update camera (called each frame)
     * @param deltaTime Time since last frame in seconds
     */
    void update(float deltaTime);

    /**
     * @brief Process keyboard input
     * @param forward Move forward/backward (-1 to 1)
     * @param right Move left/right (-1 to 1)
     * @param up Move down/up (-1 to 1)
     * @param deltaTime Time since last frame
     */
    void processMovement(float forward, float right, float up, float deltaTime);

    /**
     * @brief Process mouse movement
     * @param xOffset Mouse movement in x direction
     * @param yOffset Mouse movement in y direction
     * @param constrainPitch Prevent camera from flipping (default: true)
     */
    void processMouseMovement(float xOffset, float yOffset, bool constrainPitch = true);

    /**
     * @brief Process mouse scroll (for FOV/speed adjustment)
     * @param yOffset Scroll amount
     */
    void processMouseScroll(float yOffset);

    /**
     * @brief Get view matrix
     * @return View transformation matrix
     */
    glm::mat4 getViewMatrix() const;

    /**
     * @brief Get projection matrix
     * @return Projection transformation matrix
     */
    glm::mat4 getProjectionMatrix() const;

    /**
     * @brief Get camera position
     * @return Current position
     */
    glm::vec3 getPosition() const { return position_; }

    /**
     * @brief Get camera forward direction
     * @return Forward unit vector
     */
    glm::vec3 getFront() const { return front_; }

    /**
     * @brief Get camera up direction
     * @return Up unit vector
     */
    glm::vec3 getUp() const { return up_; }

    /**
     * @brief Get camera right direction
     * @return Right unit vector
     */
    glm::vec3 getRight() const { return right_; }

    /**
     * @brief Generate ray direction for a screen coordinate
     * @param screenX Normalized screen x coordinate (-1 to 1)
     * @param screenY Normalized screen y coordinate (-1 to 1)
     * @return Ray direction in world space
     */
    glm::vec3 getRayDirection(float screenX, float screenY) const;

    /**
     * @brief Set camera position
     * @param position New position
     */
    void setPosition(const glm::vec3& position) { position_ = position; }

    /**
     * @brief Set movement speed
     * @param speed New movement speed
     */
    void setMovementSpeed(float speed) { movementSpeed_ = speed; }

    /**
     * @brief Get movement speed
     * @return Current movement speed
     */
    float getMovementSpeed() const { return movementSpeed_; }

    /**
     * @brief Set mouse sensitivity
     * @param sensitivity New sensitivity
     */
    void setMouseSensitivity(float sensitivity) { mouseSensitivity_ = sensitivity; }

    /**
     * @brief Set field of view
     * @param fov New FOV in degrees
     */
    void setFov(float fov);

    /**
     * @brief Set aspect ratio
     * @param aspect New aspect ratio
     */
    void setAspect(float aspect);

    /**
     * @brief Get field of view
     * @return FOV in degrees
     */
    float getFov() const { return fov_; }

private:
    // Camera attributes
    glm::vec3 position_;
    glm::vec3 front_;
    glm::vec3 up_;
    glm::vec3 right_;
    glm::vec3 worldUp_;

    // Euler angles
    float yaw_;         // Rotation around Y axis
    float pitch_;       // Rotation around X axis

    // Camera options
    float movementSpeed_;
    float mouseSensitivity_;
    float fov_;
    float aspect_;
    float nearPlane_;
    float farPlane_;

    /**
     * @brief Update camera vectors from Euler angles
     */
    void updateCameraVectors();
};

} // namespace rendering
} // namespace cosmic
