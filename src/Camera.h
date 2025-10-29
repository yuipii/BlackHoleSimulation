#ifndef CAMERA_H
#define CAMERA_H

#include <Eigen/Dense>
#include <GLFW/glfw3.h>
#include <cmath>

class Camera {
public:
    Camera();
    
    void update(double delta_time);
    void handle_input(GLFWwindow* window);
    
    Eigen::Matrix4f get_view_matrix() const;
    Eigen::Matrix4f get_projection_matrix(float aspect_ratio) const;
    
    void set_position(const Eigen::Vector3f& position) { position_ = position; }
    void set_target(const Eigen::Vector3f& target) { target_ = target; }
    
    const Eigen::Vector3f& get_position() const { return position_; }
    const Eigen::Vector3f& get_front() const { return front_; }
    const Eigen::Vector3f& get_right() const { return right_; }
    const Eigen::Vector3f& get_up() const { return up_; }
    
    void set_movement_speed(float speed) { movement_speed_ = speed; }
    void set_mouse_sensitivity(float sensitivity) { mouse_sensitivity_ = sensitivity; }
    
private:
    Eigen::Vector3f position_;
    Eigen::Vector3f target_;  // Добавлено поле target_
    Eigen::Vector3f front_;
    Eigen::Vector3f up_;
    Eigen::Vector3f right_;
    Eigen::Vector3f world_up_;
    
    float yaw_;
    float pitch_;
    float movement_speed_;
    float mouse_sensitivity_;
    
    void update_camera_vectors();
    void process_keyboard(GLFWwindow* window, float delta_time);
    void process_mouse_movement(float x_offset, float y_offset);
};

// Вспомогательные функции для матриц
Eigen::Matrix4f lookAt(const Eigen::Vector3f& position, const Eigen::Vector3f& target, const Eigen::Vector3f& up);
Eigen::Matrix4f perspective(float fov, float aspect, float near_plane, float far_plane);

#endif
