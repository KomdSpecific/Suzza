#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout (set = 1, binding = 1) uniform sampler2D bindlessTextures[];

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec2 fragTexCoord;
layout (location = 2) flat in uint textureSlot;

layout (location = 0) out vec4 outColor;

void main()
{
	outColor = texture(bindlessTextures[nonuniformEXT(textureSlot)], fragTexCoord);
}