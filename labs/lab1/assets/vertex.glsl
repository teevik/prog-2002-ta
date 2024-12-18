#version 430 core

in vec2 position;
in vec3 color;

out VertexData {
    vec3 color;
} vertex_data;

void main() {
    gl_Position = vec4(position, 0.0, 1.0);
    vertex_data.color = color;
}
