#version 450

layout (push_constant) uniform ModelPush
{
	mat4 model;
} modelData;

layout (binding = 0) uniform CameraViewportInfo
{
	mat4 view;
	mat4 projection;
} cameraViewport;

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
layout (location = 2) in vec2 texCoord;

layout (location = 0) out vec3 fragColor;
layout (location = 1) out vec2 fragTexCoord;

void main()
{
	gl_Position = cameraViewport.projection * cameraViewport.view * modelData.model * vec4(position, 1.0);
	fragColor = color;
	fragTexCoord = texCoord;
}