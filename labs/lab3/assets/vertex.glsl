#version 430 core

in vec2 position;

uniform mat4 projection_matrix;
uniform mat4 view_matrix;
uniform mat4 model_matrix;

void main() {
    vec4 world_position = model_matrix * vec4(position, 0, 1);
    gl_Position = projection_matrix * view_matrix * world_position;
}
