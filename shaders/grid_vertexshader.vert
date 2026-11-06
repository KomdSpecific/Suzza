#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (push_constant) uniform GridPush
{
	mat4 ViewProjection;
} gridData;

layout (binding = 0) uniform CameraViewportInfo
{
	mat4 view;
	mat4 projection;
} cameraViewport;

layout (location = 0) out vec4 vNearWorld;
layout (location = 1) out vec4 vFarWorld;
layout (location = 2) out mat4 viewProjMatrix;

void main()
{
	/* 三个顶点覆盖整个 NDC 的"大三角形" */
	vec2 positions[3] = vec2[3](
		vec2(-1.0, -1.0),
		vec2(3.0, -1.0),
		vec2(-1.0, 3.0)
	);

	vec2 NDC = positions[gl_VertexIndex];

	/* z 先给远平面, 真正的深度由片元着色器写 gl_FragDepth 覆盖 */
	gl_Position = vec4(NDC, 1.0, 1.0);

	/* 把近/远平面上的点以"齐次坐标"输出:
	   矩阵乘是线性运算, 所以齐次坐标在全屏三角形上是仿射的,
	   光栅化器(w 恒为 1, 无透视校正)能精确插值; 除法留到片元里做 */
	vNearWorld = gridData.ViewProjection * vec4(NDC, -1.0, 1.0);
	vFarWorld = gridData.ViewProjection * vec4(NDC, 1.0, 1.0);

	viewProjMatrix = cameraViewport.projection * cameraViewport.view;
}