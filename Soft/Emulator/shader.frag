#version 460 core

in flat uint char_idx;
in flat uint color_idx; 
in      vec2 tex_coord;

out vec4 fragColor;

layout(binding = 0) uniform sampler2DArray font;

vec4 decodeColor(const uint color8) {
    const float r = float((color8 >> 5) & 7u) / 7.0;
    const float g = float((color8 >> 2) & 7u) / 7.0;
    const float b = float(color8 & 3u) / 3.0;
    return vec4(r, g, b, 1.0);
}

void main() {
    const vec3 arrayCoord = vec3(tex_coord, float(char_idx));
    
    // IGNORE '\0' GLYPH
    const float mask = mix(texture(font, arrayCoord).r, 0, char_idx == 0); 

    const vec4 fgColor = decodeColor(color_idx & 0xFFu);
    const vec4 bgColor = decodeColor((color_idx >> 8) & 0xFFu);

    fragColor = mix(bgColor, fgColor, mask);
}
