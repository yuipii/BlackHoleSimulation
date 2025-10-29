#include "Camera.h"
#include <iostream>

// Вспомогательная функция для преобразования градусов в радианы
inline float radians(float degrees) {
    return degrees * M_PI / 180.0f;
}

Camera::Camera() 
    : position_(0.0f, 5.0f, 30.0f),
      target_(0.0f, 0.0f, 0.0f),  // Инициализирован target_
      world_up_(0.0f, 1.0f, 0.0f),
      yaw_(-90.0f),
      pitch_(0.0f),
      movement_speed_(10.0f),
      mouse_sensitivity_(0.1f) {
    update_camera_vectors();
}

void Camera::update(double delta_time) {
    // Можно добавить логику автоматического движения камеры
}

void Camera::handle_input(GLFWwindow* window) {
    static double last_x = 400.0, last_y = 300.0;
    static bool first_mouse = true;
    
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    
    if (first_mouse) {
        last_x = xpos;
        last_y = ypos;
        first_mouse = false;
    }
    
    float x_offset = static_cast<float>(xpos - last_x);
    float y_offset = static_cast<float>(last_y - ypos);
    last_x = xpos;
    last_y = ypos;
    
    process_mouse_movement(x_offset, y_offset);
    process_keyboard(window, 0.016f);
}

void Camera::update_camera_vectors() {
    // Вычисляем новый вектор front
    Eigen::Vector3f new_front;
    new_front.x() = cos(radians(yaw_)) * cos(radians(pitch_));  // Исправлено: radians вместо glm::radians
    new_front.y() = sin(radians(pitch_));
    new_front.z() = sin(radians(yaw_)) * cos(radians(pitch_));
    front_ = new_front.normalized();
    
    // Также пересчитываем right и up векторы
    right_ = front_.cross(world_up_).normalized();
    up_ = right_.cross(front_).normalized();
}

void Camera::process_keyboard(GLFWwindow* window, float delta_time) {
    float velocity = movement_speed_ * delta_time;
    
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        position_ += front_ * velocity;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        position_ -= front_ * velocity;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        position_ -= right_ * velocity;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        position_ += right_ * velocity;
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        position_ += world_up_ * velocity;
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        position_ -= world_up_ * velocity;
    
    // Обновляем target для сохранения направления взгляда
    target_ = position_ + front_;
    
    // Быстрое перемещение
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        movement_speed_ = 50.0f;
    else
        movement_speed_ = 10.0f;
}

void Camera::process_mouse_movement(float x_offset, float y_offset) {
    x_offset *= mouse_sensitivity_;
    y_offset *= mouse_sensitivity_;
    
    yaw_ += x_offset;
    pitch_ += y_offset;
    
    // Ограничиваем pitch, чтобы не было переворота
    if (pitch_ > 89.0f)
        pitch_ = 89.0f;
    if (pitch_ < -89.0f)
        pitch_ = -89.0f;
    
    update_camera_vectors();
}

Eigen::Matrix4f Camera::get_view_matrix() const {
    return lookAt(position_, position_ + front_, up_);
}

Eigen::Matrix4f Camera::get_projection_matrix(float aspect_ratio) const {
    return perspective(45.0f, aspect_ratio, 0.1f, 1000.0f);
}

// Реализация вспомогательных функций
Eigen::Matrix4f lookAt(const Eigen::Vector3f& position, const Eigen::Vector3f& target, const Eigen::Vector3f& up) {
    Eigen::Vector3f f = (target - position).normalized();
    Eigen::Vector3f u = up.normalized();
    Eigen::Vector3f s = f.cross(u).normalized();
    u = s.cross(f);
    
    Eigen::Matrix4f result = Eigen::Matrix4f::Identity();
    result(0,0) = s.x();
    result(0,1) = s.y();
    result(0,2) = s.z();
    result(1,0) = u.x();
    result(1,1) = u.y();
    result(1,2) = u.z();
    result(2,0) = -f.x();
    result(2,1) = -f.y();
    result(2,2) = -f.z();
    result(0,3) = -s.dot(position);
    result(1,3) = -u.dot(position);
    result(2,3) = f.dot(position);
    return result;
}

Eigen::Matrix4f perspective(float fov, float aspect, float near_plane, float far_plane) {
    float tan_half_fov = tan(fov * M_PI / 360.0f);
    
    Eigen::Matrix4f result = Eigen::Matrix4f::Zero();
    result(0,0) = 1.0f / (aspect * tan_half_fov);
    result(1,1) = 1.0f / tan_half_fov;
    result(2,2) = -(far_plane + near_plane) / (far_plane - near_plane);
    result(2,3) = -(2.0f * far_plane * near_plane) / (far_plane - near_plane);
    result(3,2) = -1.0f;
    
    return result;
}
