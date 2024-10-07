#version 460 core

in vec3 varying_tex_coords;

out vec4 out_color;

uniform samplerCube uniform_texture_slot;

void main()
{
	out_color = texture( uniform_texture_slot, varying_tex_coords );
}
