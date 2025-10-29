#ifndef RENDERER_H
#define RENDERER_H

#include "BlackHole.h"
#include "PhysicsEngine.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <Eigen/Dense>
#include <memory>
#include <vector>

class Renderer {
public:
    Renderer(int width, int height);
    ~Renderer();
    
    bool initialize();
    void render(const std::shared_ptr<BlackHole>& black_hole, 
                const PhysicsEngine& physics_engine);
    void shutdown();
    
    GLFWwindow* get_window() const { return window_; }
    
private:
    GLFWwindow* window_;
    int width_, height_;
    
    // Shader programs
    GLuint black_hole_shader_;
    GLuint accretion_shader_;
    GLuint star_shader_;
    GLuint body_shader_;
    
    // Vertex arrays and buffers
    GLuint black_hole_vao_, black_hole_vbo_;
    GLuint accretion_vao_, accretion_vbo_;
    GLuint star_vao_, star_vbo_;
    GLuint body_vao_, body_vbo_;
    
    void setup_black_hole_rendering();
    void setup_accretion_disk_rendering();
    void setup_star_field_rendering();
    void setup_body_rendering();
    
    // Методы рендеринга без параметров матриц
    void render_black_hole(const BlackHole& black_hole);
    void render_accretion_disk(const BlackHole& black_hole);
    void render_star_field();
    void render_celestial_bodies(const PhysicsEngine& physics_engine);
    
    GLuint compile_shader(const std::string& vertex_source, 
                         const std::string& fragment_source);
    
    // Вспомогательные функции для матриц
    Eigen::Matrix4f create_projection_matrix() const;
    Eigen::Matrix4f create_view_matrix() const;
    Eigen::Matrix4f create_lookAt_matrix(const Eigen::Vector3f& position, 
                                        const Eigen::Vector3f& target, 
                                        const Eigen::Vector3f& up) const;
    Eigen::Matrix4f create_perspective_matrix(float fov, float aspect, 
                                             float near_plane, float far_plane) const;
};

#endif
