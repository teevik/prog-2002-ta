#version 430 core

in VertexData {
    vec3 color;
} vertex_data;

out vec4 color;

void main() {
    color = vec4(vertex_data.color, 1);
}
