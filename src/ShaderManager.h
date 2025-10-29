#ifndef SHADERMANAGER_H
#define SHADERMANAGER_H

#include <GL/glew.h>
#include <string>
#include <unordered_map>

class ShaderManager {
public:
    static GLuint load_shader(const std::string& name);
    static void cleanup();
    
private:
    static std::unordered_map<std::string, GLuint> shaders_;
    
    static std::string load_shader_source(const std::string& filename);
    static GLuint compile_shader_program(const std::string& vertex_source, 
                                        const std::string& fragment_source);
};

#endif
