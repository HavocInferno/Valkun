#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_KHR_vulkan_glsl : enable

out gl_PerVertex {
	vec4 gl_Position;
};

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inUVCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragUVCoord;

layout (set = 0, binding = 0) uniform UboView 
{
	mat4 projection;
	mat4 view;
} uboView;

layout (set = 0, binding = 1) uniform UboInstance 
{
	mat4 model; 
} uboInstance;

void main() 
{
	fragColor = inColor;
	fragUVCoord = inUVCoord;
	mat4 modelView = uboView.view * uboInstance.model;
	vec3 worldPos = vec3(modelView * vec4(inPos, 1.0));
	gl_Position = uboView.projection * modelView * vec4(inPos.xyz, 1.0);
}