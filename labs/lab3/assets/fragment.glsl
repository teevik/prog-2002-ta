#version 430 core

in VertexData {
    vec2 grid_position;
} vertex_data;

out vec4 color;

uniform ivec2 board_tiles;
uniform ivec2 selected_tile;
layout(binding = 0) uniform sampler2D texture_sampler;

const vec4 white = vec4(1, 1, 1, 1);
const vec4 black = vec4(0, 0, 0, 1);
const vec4 green = vec4(0, 1, 0, 1);

void main() {
    int index = gl_PrimitiveID / 2;
    int x = index % board_tiles.x;
    int y = index / board_tiles.y;

    bool is_black = x % 2 == y % 2;
    bool is_selected = ivec2(x, y) == selected_tile;

    vec4 grid_color = is_selected ? green : (is_black ? black : white);
    vec4 texture_color = texture(texture_sampler, vertex_data.grid_position / 2.);

    color = mix(grid_color, texture_color, 0.8);
}
