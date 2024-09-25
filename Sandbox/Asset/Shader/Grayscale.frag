#version 460 core

in vec2 varying_tex_coords;

out vec4 out_color;

uniform sampler2D uniform_texture_slot;

void main()
{
	out_color = texture( uniform_texture_slot, varying_tex_coords );
    float average = 0.2126 * out_color.r + 0.7152 * out_color.g + 0.0722 * out_color.b;
    out_color = vec4( average, average, average, 1.0 );
}
