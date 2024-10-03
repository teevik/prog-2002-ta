#version 430 core

out vec4 color;

uniform ivec2 board_tiles;

const vec4 white = vec4(1, 1, 1, 1);
const vec4 black = vec4(0, 0, 0, 1);

void main() {
    int index = gl_PrimitiveID / 2;
    int x = index % board_tiles.x;
    int y = index / board_tiles.y;

    bool is_black = x % 2 == y % 2;

    color = is_black ? black : white;
}
