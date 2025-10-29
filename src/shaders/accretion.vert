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
    
    // Add slight warping due to frame dragging
    float warp = sin(DiskRadius * 0.5 - time * 2.0) * 0.1;
    vec3 warpedPos = WorldPos + vec3(0.0, warp, 0.0);
    
    gl_Position = projection * view * vec4(warpedPos, 1.0);
}
