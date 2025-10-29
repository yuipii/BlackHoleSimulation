#include "ShaderManager.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

std::unordered_map<std::string, GLuint> ShaderManager::shaders_;

GLuint ShaderManager::load_shader(const std::string& name) {
    auto it = shaders_.find(name);
    if (it != shaders_.end()) {
        return it->second;
    }
    
    std::string vertex_source, fragment_source;
    
    if (name == "blackhole") {
        vertex_source = R"(
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 FragPos;

void main() {
    FragPos = vec3(model * vec4(aPos, 1.0));
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
)";

        fragment_source = R"(
#version 330 core
out vec4 FragColor;

in vec3 FragPos;

uniform vec3 blackHolePos;
uniform float eventHorizonRadius;
uniform float time;

void main() {
    vec3 toBlackHole = blackHolePos - FragPos;
    float distance = length(toBlackHole);
    
    if (distance < eventHorizonRadius * 1.02) {
        FragColor = vec4(0.0, 0.0, 0.0, 1.0);
    } else if (distance < eventHorizonRadius * 1.5) {
        float intensity = (distance - eventHorizonRadius * 1.02) / (eventHorizonRadius * 0.48);
        intensity = pow(intensity, 0.5);
        float pulse = sin(time * 5.0) * 0.1 + 0.9;
        vec3 color = mix(vec3(0.3, 0.0, 0.0), vec3(0.8, 0.1, 0.0), intensity) * pulse;
        FragColor = vec4(color, 1.0);
    } else {
        float influence = eventHorizonRadius / distance;
        influence = clamp(influence, 0.0, 0.3);
        vec3 color = mix(vec3(0.1, 0.03, 0.0), vec3(0.05, 0.01, 0.0), influence);
        FragColor = vec4(color, 0.7);
    }
}
)";
    }
    else if (name == "accretion") {
        vertex_source = R"(
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float time;

out vec3 WorldPos;
out float DiskRadius;

void main() {
    WorldPos = vec3(model * vec4(aPos, 1.0));
    DiskRadius = length(WorldPos.xz);
    
    float warp = sin(DiskRadius * 0.5 - time * 2.0) * 0.1;
    vec3 warpedPos = WorldPos + vec3(0.0, warp, 0.0);
    
    gl_Position = projection * view * vec4(warpedPos, 1.0);
}
)";

        fragment_source = R"(
#version 330 core
out vec4 FragColor;

in vec3 WorldPos;
in float DiskRadius;

uniform vec3 blackHolePos;
uniform float innerRadius;
uniform float outerRadius;
uniform float time;

void main() {
    float t = (DiskRadius - innerRadius) / (outerRadius - innerRadius);
    t = clamp(t, 0.0, 1.0);
    
    float temperature = 1.0 - t;
    vec3 viewDir = normalize(-WorldPos);
    vec3 diskNormal = vec3(0.0, 1.0, 0.0);
    float doppler = dot(diskNormal, viewDir);
    
    vec3 color;
    if (temperature > 0.8) {
        float hotness = (temperature - 0.8) * 5.0;
        color = mix(vec3(0.5, 0.7, 1.0), vec3(1.0, 1.0, 1.2), hotness);
    } else if (temperature > 0.5) {
        float midness = (temperature - 0.5) * 3.33;
        color = mix(vec3(1.0, 0.8, 0.2), vec3(1.0, 1.0, 0.5), midness);
    } else if (temperature > 0.2) {
        float coolness = (temperature - 0.2) * 3.33;
        color = mix(vec3(1.0, 0.3, 0.0), vec3(1.0, 0.8, 0.2), coolness);
    } else {
        color = mix(vec3(0.3, 0.0, 0.0), vec3(1.0, 0.3, 0.0), temperature * 5.0);
    }
    
    if (doppler > 0.0) {
        color = mix(color, vec3(0.3, 0.5, 1.0), doppler * 0.6);
    } else {
        color = mix(color, vec3(1.0, 0.2, 0.1), -doppler * 0.6);
    }
    
    float alpha;
    if (DiskRadius < innerRadius * 1.1) {
        alpha = smoothstep(innerRadius * 0.9, innerRadius * 1.1, DiskRadius);
    } else {
        alpha = 1.0 - smoothstep(outerRadius * 0.7, outerRadius, DiskRadius);
    }
    
    alpha = clamp(alpha, 0.1, 1.0);
    FragColor = vec4(color, alpha);
}
)";
    }
    
    GLuint shader_program = compile_shader_program(vertex_source, fragment_source);
    shaders_[name] = shader_program;
    
    return shader_program;
}

std::string ShaderManager::load_shader_source(const std::string& filename) {
    std::ifstream file(filename);  // Исправлено: ifstream вместо iffile
    if (!file.is_open()) {
        std::cerr << "Failed to open shader file: " << filename << std::endl;
        return "";
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

GLuint ShaderManager::compile_shader_program(const std::string& vertex_source, 
                                           const std::string& fragment_source) {
    const char* vshader_code = vertex_source.c_str();
    const char* fshader_code = fragment_source.c_str();
    
    // Vertex shader
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
    
    // Fragment shader
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fshader_code, NULL);
    glCompileShader(fragment_shader);
    
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment_shader, 512, NULL, info_log);
        std::cerr << "Fragment shader compilation failed: " << info_log << std::endl;
    }
    
    // Shader program
    GLuint shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);
    
    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shader_program, 512, NULL, info_log);
        std::cerr << "Shader program linking failed: " << info_log << std::endl;
    }
    
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    
    return shader_program;
}

void ShaderManager::cleanup() {
    for (auto& shader : shaders_) {
        glDeleteProgram(shader.second);
    }
    shaders_.clear();
}
