/**
 * Name: Kishan S Patel
 * Email: kishan.patel@digipen.edu
 * Assignment Number: 7
 * Course: CS200
 * Term: Fall 2024
 *
 * File: light.frag
 */

#version 130

in vec2 interp_world_position;
in vec2 interp_texcoord;

uniform vec4 light_position;
uniform float light_radius;
uniform float light_factor;
uniform float ambient_factor;
uniform sampler2D obj_texture;

out vec4 frag_color;

void main() {
    vec2 to_light = interp_world_position - light_position.xy;

    float dist2 = dot(to_light, to_light);
    float spotlight = min(1., light_radius * light_radius / dist2);

    float total_light = spotlight * light_factor + ambient_factor;

    vec4 color = texture(obj_texture, interp_texcoord) * total_light;
    frag_color = clamp(color, vec4(0.), vec4(1.));
}

// vim: set ft=glsl:
