#include "Renderer.h"
#include "ShaderManager.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <random>
#include <cmath>

// Простые шейдеры для звездного поля
const char* star_vertex_shader = R"(
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    gl_PointSize = 2.0;
}
)";

const char* star_fragment_shader = R"(
#version 330 core
out vec4 FragColor;

void main() {
    float alpha = 1.0 - smoothstep(0.0, 1.0, length(gl_PointCoord - vec2(0.5)) * 2.0);
    FragColor = vec4(1.0, 1.0, 1.0, alpha);
}
)";

// Шейдер для небесных тел
const char* body_vertex_shader = R"(
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec3 FragPos;

void main() {
    FragPos = vec3(model * vec4(aPos, 1.0));
    gl_Position = projection * view * vec4(FragPos, 1.0);
    gl_PointSize = 8.0;
}
)";

const char* body_fragment_shader = R"(
#version 330 core
out vec4 FragColor;

in vec3 FragPos;

uniform vec3 bodyColor;

void main() {
    vec2 coord = gl_PointCoord - vec2(0.5);
    float dist = length(coord);
    
    if (dist > 0.5) discard;
    
    float intensity = 1.0 - smoothstep(0.3, 0.5, dist);
    vec3 color = bodyColor * intensity;
    
    float core = 1.0 - smoothstep(0.0, 0.2, dist);
    color += bodyColor * core * 0.5;
    
    FragColor = vec4(color, 1.0);
}
)";

Renderer::Renderer(int width, int height) 
    : window_(nullptr), width_(width), height_(height),
      black_hole_shader_(0), accretion_shader_(0), star_shader_(0), body_shader_(0),
      black_hole_vao_(0), black_hole_vbo_(0),
      accretion_vao_(0), accretion_vbo_(0),
      star_vao_(0), star_vbo_(0),
      body_vao_(0), body_vbo_(0) {}

Renderer::~Renderer() {
    shutdown();
}

bool Renderer::initialize() {
    // Инициализация GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }
    
    // Настройка GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    
    // Создание окна
    window_ = glfwCreateWindow(width_, height_, "Interstellar Black Hole Simulation", nullptr, nullptr);
    if (!window_) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    
    glfwMakeContextCurrent(window_);
    
    // Инициализация GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return false;
    }
    
    // Настройка OpenGL
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_PROGRAM_POINT_SIZE);
    
    // Инициализация компонентов рендеринга
    setup_star_field_rendering();
    setup_accretion_disk_rendering();
    setup_black_hole_rendering();
    setup_body_rendering();
    
    std::cout << "Renderer initialized successfully" << std::endl;
    return true;
}

void Renderer::setup_star_field_rendering() {
    // Генерация случайных позиций звезд
    std::vector<Eigen::Vector3f> stars;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> pos_dist(-500.0f, 500.0f);
    
    for (int i = 0; i < 2000; ++i) {
        Eigen::Vector3f star(
            pos_dist(gen),
            pos_dist(gen) * 0.3f,
            pos_dist(gen)
        );
        stars.push_back(star);
    }
    
    // Создание буферов
    glGenVertexArrays(1, &star_vao_);
    glGenBuffers(1, &star_vbo_);
    
    glBindVertexArray(star_vao_);
    glBindBuffer(GL_ARRAY_BUFFER, star_vbo_);
    glBufferData(GL_ARRAY_BUFFER, stars.size() * sizeof(Eigen::Vector3f), 
                 stars.data(), GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Eigen::Vector3f), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Компиляция шейдера
    star_shader_ = compile_shader(star_vertex_shader, star_fragment_shader);
    
    glBindVertexArray(0);
}

void Renderer::setup_accretion_disk_rendering() {
    // Создание геометрии аккреционного диска
    std::vector<float> vertices;
    int segments = 200;
    float inner_radius = 2.5f;
    float outer_radius = 25.0f;
    
    for (int i = 0; i <= segments; ++i) {
        float angle1 = 2.0f * M_PI * i / segments;
        float angle2 = 2.0f * M_PI * (i + 1) / segments;
        
        // Внутренняя точка 1
        float x_in1 = inner_radius * cos(angle1);
        float z_in1 = inner_radius * sin(angle1);
        
        // Внешняя точка 1
        float x_out1 = outer_radius * cos(angle1);
        float z_out1 = outer_radius * sin(angle1);
        
        // Внутренняя точка 2
        float x_in2 = inner_radius * cos(angle2);
        float z_in2 = inner_radius * sin(angle2);
        
        // Внешняя точка 2
        float x_out2 = outer_radius * cos(angle2);
        float z_out2 = outer_radius * sin(angle2);
        
        // Первый треугольник
        vertices.insert(vertices.end(), {x_in1, 0.0f, z_in1});
        vertices.insert(vertices.end(), {x_out1, 0.0f, z_out1});
        vertices.insert(vertices.end(), {x_in2, 0.0f, z_in2});
        
        // Второй треугольник
        vertices.insert(vertices.end(), {x_in2, 0.0f, z_in2});
        vertices.insert(vertices.end(), {x_out1, 0.0f, z_out1});
        vertices.insert(vertices.end(), {x_out2, 0.0f, z_out2});
    }
    
    // Создание буферов
    glGenVertexArrays(1, &accretion_vao_);
    glGenBuffers(1, &accretion_vbo_);
    
    glBindVertexArray(accretion_vao_);
    glBindBuffer(GL_ARRAY_BUFFER, accretion_vbo_);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), 
                 vertices.data(), GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Загрузка шейдера аккреционного диска
    accretion_shader_ = ShaderManager::load_shader("accretion");
    
    glBindVertexArray(0);
}

void Renderer::setup_black_hole_rendering() {
    // Создание сферы для черной дыры
    std::vector<float> vertices;
    int stacks = 50;
    int slices = 50;
    float radius = 1.8f;
    
    for (int i = 0; i <= stacks; ++i) {
        float phi = M_PI * i / stacks;
        for (int j = 0; j <= slices; ++j) {
            float theta = 2.0f * M_PI * j / slices;
            
            float x = radius * sin(phi) * cos(theta);
            float y = radius * sin(phi) * sin(theta);
            float z = radius * cos(phi);
            
            vertices.insert(vertices.end(), {x, y, z});
        }
    }
    
    // Создание буферов
    glGenVertexArrays(1, &black_hole_vao_);
    glGenBuffers(1, &black_hole_vbo_);
    
    glBindVertexArray(black_hole_vao_);
    glBindBuffer(GL_ARRAY_BUFFER, black_hole_vbo_);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), 
                 vertices.data(), GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Загрузка шейдера черной дыры
    black_hole_shader_ = ShaderManager::load_shader("blackhole");
    
    glBindVertexArray(0);
}

void Renderer::setup_body_rendering() {
    // Простая геометрия для небесных тел
    std::vector<float> vertices = {0.0f, 0.0f, 0.0f};
    
    glGenVertexArrays(1, &body_vao_);
    glGenBuffers(1, &body_vbo_);
    
    glBindVertexArray(body_vao_);
    glBindBuffer(GL_ARRAY_BUFFER, body_vbo_);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), 
                 vertices.data(), GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Компиляция шейдера для небесных тел
    body_shader_ = compile_shader(body_vertex_shader, body_fragment_shader);
    
    glBindVertexArray(0);
}

void Renderer::render(const std::shared_ptr<BlackHole>& black_hole, 
                     const PhysicsEngine& physics_engine) {
    // Очистка буферов
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    if (black_hole) {
        // Рендеринг в правильном порядке
        render_star_field();
        render_accretion_disk(*black_hole);
        render_celestial_bodies(physics_engine);
        render_black_hole(*black_hole);
    }
    
    // Обмен буферов и обработка событий
    glfwSwapBuffers(window_);
    glfwPollEvents();
}

void Renderer::render_star_field() {
    glUseProgram(star_shader_);
    
    // Получение матриц
    Eigen::Matrix4f projection = create_projection_matrix();
    Eigen::Matrix4f view = create_view_matrix();
    Eigen::Matrix4f model = Eigen::Matrix4f::Identity();
    
    // Установка uniform-переменных
    GLuint projection_loc = glGetUniformLocation(star_shader_, "projection");
    GLuint view_loc = glGetUniformLocation(star_shader_, "view");
    GLuint model_loc = glGetUniformLocation(star_shader_, "model");
    
    glUniformMatrix4fv(projection_loc, 1, GL_FALSE, projection.data());
    glUniformMatrix4fv(view_loc, 1, GL_FALSE, view.data());
    glUniformMatrix4fv(model_loc, 1, GL_FALSE, model.data());
    
    // Рендеринг звезд
    glBindVertexArray(star_vao_);
    glDrawArrays(GL_POINTS, 0, 2000);
    glBindVertexArray(0);
}

void Renderer::render_accretion_disk(const BlackHole& black_hole) {
    glUseProgram(accretion_shader_);
    glEnable(GL_BLEND);
    
    // Получение матриц
    Eigen::Matrix4f projection = create_projection_matrix();
    Eigen::Matrix4f view = create_view_matrix();
    Eigen::Matrix4f model = Eigen::Matrix4f::Identity();
    
    // Uniform-переменные для трансформаций
    GLuint model_loc = glGetUniformLocation(accretion_shader_, "model");
    GLuint view_loc = glGetUniformLocation(accretion_shader_, "view");
    GLuint projection_loc = glGetUniformLocation(accretion_shader_, "projection");
    
    glUniformMatrix4fv(model_loc, 1, GL_FALSE, model.data());
    glUniformMatrix4fv(view_loc, 1, GL_FALSE, view.data());
    glUniformMatrix4fv(projection_loc, 1, GL_FALSE, projection.data());
    
    // Параметры черной дыры
    GLuint black_hole_pos_loc = glGetUniformLocation(accretion_shader_, "blackHolePos");
    GLuint inner_radius_loc = glGetUniformLocation(accretion_shader_, "innerRadius");
    GLuint outer_radius_loc = glGetUniformLocation(accretion_shader_, "outerRadius");
    GLuint time_loc = glGetUniformLocation(accretion_shader_, "time");
    
    Eigen::Vector3f bh_pos = black_hole.get_parameters().position.cast<float>();
    float event_horizon = static_cast<float>(black_hole.get_event_horizon_radius());
    float inner_radius = event_horizon * 2.5f;
    float outer_radius = event_horizon * 25.0f;
    float time = static_cast<float>(glfwGetTime());
    
    glUniform3f(black_hole_pos_loc, bh_pos.x(), bh_pos.y(), bh_pos.z());
    glUniform1f(inner_radius_loc, inner_radius);
    glUniform1f(outer_radius_loc, outer_radius);
    glUniform1f(time_loc, time);
    
    // Рендеринг аккреционного диска
    glBindVertexArray(accretion_vao_);
    glDrawArrays(GL_TRIANGLES, 0, 1200);
    glBindVertexArray(0);
    
    glDisable(GL_BLEND);
}

void Renderer::render_black_hole(const BlackHole& black_hole) {
    glUseProgram(black_hole_shader_);
    
    // Получение матриц
    Eigen::Matrix4f projection = create_projection_matrix();
    Eigen::Matrix4f view = create_view_matrix();
    Eigen::Matrix4f model = Eigen::Matrix4f::Identity();
    
    // Uniform-переменные для трансформаций
    GLuint model_loc = glGetUniformLocation(black_hole_shader_, "model");
    GLuint view_loc = glGetUniformLocation(black_hole_shader_, "view");
    GLuint projection_loc = glGetUniformLocation(black_hole_shader_, "projection");
    
    glUniformMatrix4fv(model_loc, 1, GL_FALSE, model.data());
    glUniformMatrix4fv(view_loc, 1, GL_FALSE, view.data());
    glUniformMatrix4fv(projection_loc, 1, GL_FALSE, projection.data());
    
    // Параметры черной дыры
    GLuint black_hole_pos_loc = glGetUniformLocation(black_hole_shader_, "blackHolePos");
    GLuint event_horizon_loc = glGetUniformLocation(black_hole_shader_, "eventHorizonRadius");
    GLuint time_loc = glGetUniformLocation(black_hole_shader_, "time");
    
    Eigen::Vector3f bh_pos = black_hole.get_parameters().position.cast<float>();
    float event_horizon = static_cast<float>(black_hole.get_event_horizon_radius());
    float time = static_cast<float>(glfwGetTime());
    
    glUniform3f(black_hole_pos_loc, bh_pos.x(), bh_pos.y(), bh_pos.z());
    glUniform1f(event_horizon_loc, event_horizon);
    glUniform1f(time_loc, time);
    
    // Рендеринг черной дыры
    glBindVertexArray(black_hole_vao_);
    glDrawArrays(GL_POINTS, 0, 2601);
    glBindVertexArray(0);
}

void Renderer::render_celestial_bodies(const PhysicsEngine& physics_engine) {
    glUseProgram(body_shader_);
    glEnable(GL_BLEND);
    
    // Получение матриц
    Eigen::Matrix4f projection = create_projection_matrix();
    Eigen::Matrix4f view = create_view_matrix();
    
    // Uniform-переменные для трансформаций
    GLuint projection_loc = glGetUniformLocation(body_shader_, "projection");
    GLuint view_loc = glGetUniformLocation(body_shader_, "view");
    
    glUniformMatrix4fv(projection_loc, 1, GL_FALSE, projection.data());
    glUniformMatrix4fv(view_loc, 1, GL_FALSE, view.data());
    
    // Uniform для цвета
    GLuint body_color_loc = glGetUniformLocation(body_shader_, "bodyColor");
    
    // Рендеринг каждого небесного тела
    glBindVertexArray(body_vao_);
    
    int body_index = 0;
    for (const auto& body : physics_engine.get_bodies()) {
        // Модельная матрица для позиционирования тела
        Eigen::Matrix4f model = Eigen::Matrix4f::Identity();
        model.block<3,1>(0,3) = body.position.cast<float>();
        
        GLuint model_loc = glGetUniformLocation(body_shader_, "model");
        glUniformMatrix4fv(model_loc, 1, GL_FALSE, model.data());
        
        // Разные цвета для разных тел
        Eigen::Vector3f color;
        if (body_index == 0) {
            color = Eigen::Vector3f(0.2f, 0.6f, 1.0f);
        } else {
            color = Eigen::Vector3f(1.0f, 0.8f, 0.2f);
        }
        
        glUniform3f(body_color_loc, color.x(), color.y(), color.z());
        
        // Рендеринг тела
        glDrawArrays(GL_POINTS, 0, 1);
        
        body_index++;
    }
    
    glBindVertexArray(0);
    glDisable(GL_BLEND);
}

Eigen::Matrix4f Renderer::create_projection_matrix() const {
    float fov = 45.0f;
    float aspect = static_cast<float>(width_) / height_;
    float near_plane = 0.1f;
    float far_plane = 1000.0f;
    
    return create_perspective_matrix(fov, aspect, near_plane, far_plane);
}

Eigen::Matrix4f Renderer::create_view_matrix() const {
    Eigen::Vector3f camera_pos(0.0f, 5.0f, 30.0f);
    Eigen::Vector3f camera_target(0.0f, 0.0f, 0.0f);
    Eigen::Vector3f camera_up(0.0f, 1.0f, 0.0f);
    
    return create_lookAt_matrix(camera_pos, camera_target, camera_up);
}

Eigen::Matrix4f Renderer::create_lookAt_matrix(const Eigen::Vector3f& position, 
                                              const Eigen::Vector3f& target, 
                                              const Eigen::Vector3f& up) const {
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

Eigen::Matrix4f Renderer::create_perspective_matrix(float fov, float aspect, 
                                                   float near_plane, float far_plane) const {
    float tan_half_fov = tan(fov * M_PI / 360.0f);
    
    Eigen::Matrix4f result = Eigen::Matrix4f::Zero();
    result(0,0) = 1.0f / (aspect * tan_half_fov);
    result(1,1) = 1.0f / tan_half_fov;
    result(2,2) = -(far_plane + near_plane) / (far_plane - near_plane);
    result(2,3) = -(2.0f * far_plane * near_plane) / (far_plane - near_plane);
    result(3,2) = -1.0f;
    
    return result;
}

GLuint Renderer::compile_shader(const std::string& vertex_source, 
                               const std::string& fragment_source) {
    const char* vshader_code = vertex_source.c_str();
    const char* fshader_code = fragment_source.c_str();
    
    // Компиляция вершинного шейдера
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vshader_code, NULL);
    glCompileShader(vertex_shader);
    
    GLint success;
    GLchar info_log[512];
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertex_shader, 512, NULL, info_log);
        std::cerr << "Vertex shader compilation failed: " << info_log << std::endl;
    }
    
    // Компиляция фрагментного шейдера
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fshader_code, NULL);
    glCompileShader(fragment_shader);
    
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment_shader, 512, NULL, info_log);
        std::cerr << "Fragment shader compilation failed: " << info_log << std::endl;
    }
    
    // Линковка шейдерной программы
    GLuint shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);
    
    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shader_program, 512, NULL, info_log);
        std::cerr << "Shader program linking failed: " << info_log << std::endl;
    }
    
    // Очистка
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    
    return shader_program;
}

void Renderer::shutdown() {
    // Очистка шейдеров
    if (star_shader_) glDeleteProgram(star_shader_);
    if (body_shader_) glDeleteProgram(body_shader_);
    if (accretion_shader_) glDeleteProgram(accretion_shader_);
    if (black_hole_shader_) glDeleteProgram(black_hole_shader_);
    
    // Очистка VAO и VBO
    if (star_vao_) glDeleteVertexArrays(1, &star_vao_);
    if (star_vbo_) glDeleteBuffers(1, &star_vbo_);
    if (body_vao_) glDeleteVertexArrays(1, &body_vao_);
    if (body_vbo_) glDeleteBuffers(1, &body_vbo_);
    if (accretion_vao_) glDeleteVertexArrays(1, &accretion_vao_);
    if (accretion_vbo_) glDeleteBuffers(1, &accretion_vbo_);
    if (black_hole_vao_) glDeleteVertexArrays(1, &black_hole_vao_);
    if (black_hole_vbo_) glDeleteBuffers(1, &black_hole_vbo_);
    
    // Очистка менеджера шейдеров
    ShaderManager::cleanup();
    
    // Уничтожение окна и завершение GLFW
    if (window_) {
        glfwDestroyWindow(window_);
        window_ = nullptr;
    }
    glfwTerminate();
}
