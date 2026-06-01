#version 460 core

layout (std430, binding = 0) readonly buffer grid_buffer {
    uint chars_packed[1600];
    uint colors_packed[1600];
};

uniform vec2 window_size;

out flat uint char_idx;
out flat uint color_idx;
out      vec2 tex_coord;

const vec2 quad_verts[6] = vec2[6](
    vec2(0.0, 0.0), vec2(1.0, 0.0), vec2(1.0, 1.0),
    vec2(1.0, 1.0), vec2(0.0, 1.0), vec2(0.0, 0.0)
);

void main() {
    const uint array_idx = gl_InstanceID / 2;
    const bool byte_idx = (gl_InstanceID % 2) != 0;

    const uint tile_chunk  = chars_packed[array_idx];
    const uint color_chunk = colors_packed[array_idx];

    char_idx =  byte_idx? (tile_chunk >> 16 & 0xFF) : (tile_chunk & 0xFF);
    color_idx = byte_idx? (color_chunk >> 16) : (color_chunk & 0xFFFF);

    const int col = gl_InstanceID % 80;
    const int row = gl_InstanceID / 80;

    tex_coord = quad_verts[gl_VertexID];

    const vec2 cell_offset = vec2(col * 8.0, row * 16.0);
    const vec2 vertex_pixel_pos = cell_offset + (quad_verts[gl_VertexID] * vec2(8.0, 16.0));

    const vec2 ndc_pos = (vertex_pixel_pos / window_size) * 2.0 - 1.0;
    gl_Position = vec4(ndc_pos.x, -ndc_pos.y, 0.0, 1.0);
}
