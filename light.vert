/**
 * Name: Kishan S Patel
 * Email: kishan.patel@digipen.edu
 * Assignment Number: 7
 * Course: CS200
 * Term: Fall 2024
 *
 * File: light.vert
 */

#version 130

in vec4 position;
in vec2 texcoord;

uniform mat4 object_to_world;
uniform mat4 world_to_ndc;

out vec2 interp_world_position;
out vec2 interp_texcoord;

void main() {
    vec4 world_pos = object_to_world * position;

    interp_texcoord = texcoord;
    interp_world_position = world_pos.xy;
    gl_Position = world_to_ndc * world_pos;
}

// vim: set ft=glsl:
