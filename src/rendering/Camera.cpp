#include "rendering/Camera.hpp"
#include <algorithm>

namespace cosmic {
namespace rendering {

Camera::Camera(const glm::vec3& position, float fov, float aspect,
               float nearPlane, float farPlane)
    : position_(position),
      front_(0.0f, 0.0f, -1.0f),
      up_(0.0f, 1.0f, 0.0f),
      right_(1.0f, 0.0f, 0.0f),
      worldUp_(0.0f, 1.0f, 0.0f),
      yaw_(-90.0f),
      pitch_(0.0f),
      movementSpeed_(10.0f),
      mouseSensitivity_(0.1f),
      fov_(fov),
      aspect_(aspect),
      nearPlane_(nearPlane),
      farPlane_(farPlane) {
    updateCameraVectors();
}

void Camera::update(float deltaTime) {
    // Nothing to do here for now
    // Could add smooth interpolation or other updates
}

void Camera::processMovement(float forward, float right, float up, float deltaTime) {
    float velocity = movementSpeed_ * deltaTime;

    position_ += front_ * forward * velocity;
    position_ += right_ * right * velocity;
    position_ += worldUp_ * up * velocity;
}

void Camera::processMouseMovement(float xOffset, float yOffset, bool constrainPitch) {
    xOffset *= mouseSensitivity_;
    yOffset *= mouseSensitivity_;

    yaw_ += xOffset;
    pitch_ += yOffset;

    // Constrain pitch to prevent screen flipping
    if (constrainPitch) {
        pitch_ = std::clamp(pitch_, -89.0f, 89.0f);
    }

    updateCameraVectors();
}

void Camera::processMouseScroll(float yOffset) {
    // Adjust movement speed with scroll
    movementSpeed_ += yOffset * 0.5f;
    movementSpeed_ = std::max(0.1f, movementSpeed_);
}

glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(position_, position_ + front_, up_);
}

glm::mat4 Camera::getProjectionMatrix() const {
    return glm::perspective(glm::radians(fov_), aspect_, nearPlane_, farPlane_);
}

glm::vec3 Camera::getRayDirection(float screenX, float screenY) const {
    // Convert screen coordinates to world space ray direction
    // screenX, screenY in range [-1, 1]

    float tanHalfFov = std::tan(glm::radians(fov_ * 0.5f));

    glm::vec3 rayDir = front_ +
                       right_ * (screenX * aspect_ * tanHalfFov) +
                       up_ * (screenY * tanHalfFov);

    return glm::normalize(rayDir);
}

void Camera::setFov(float fov) {
    fov_ = std::clamp(fov, 1.0f, 120.0f);
}

void Camera::setAspect(float aspect) {
    aspect_ = aspect;
}

void Camera::updateCameraVectors() {
    // Calculate new front vector
    glm::vec3 newFront;
    newFront.x = std::cos(glm::radians(yaw_)) * std::cos(glm::radians(pitch_));
    newFront.y = std::sin(glm::radians(pitch_));
    newFront.z = std::sin(glm::radians(yaw_)) * std::cos(glm::radians(pitch_));
    front_ = glm::normalize(newFront);

    // Recalculate right and up vectors
    right_ = glm::normalize(glm::cross(front_, worldUp_));
    up_ = glm::normalize(glm::cross(right_, front_));
}

} // namespace rendering
} // namespace cosmic
