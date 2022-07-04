#version 460 core
layout(binding = 0) uniform sampler2D textureSampler;
layout(location = 0) in vec2 inPos;
layout(location = 0) out vec4 FragColor;


void main()
{
	// FragColor = vec4(inColor,1);
    FragColor = texture(textureSampler,vec2(inPos.s,1-inPos.t));
}