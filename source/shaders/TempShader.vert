#version 430 core

layout (location = 0) in vec2 vp;

// Uniform matrices
uniform mat4 MVP;

void main(){
    gl_Position = vec4 (vp, 0.0, 1.0);
}

