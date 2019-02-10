#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragUVCoord;

layout(location = 0) out vec4 outColor; 

layout(set = 1, binding = 0) uniform sampler2D tex;

void main() 
{
	//outColor = vec4(fragColor, 1.0);
	outColor = texture(tex, fragUVCoord);
}