#version 460 core

layout(location = 0) in vec2 inPos;
layout(location = 1) in vec3 inColor;
layout(location = 0) out vec2 outUV;

void main()
{
    
    outUV = vec2((gl_VertexIndex<<1)&2,gl_VertexIndex &2);
	gl_Position = vec4(outUV*2.f - 1.f,0,1);
	// gl_Position.y = -gl_Position.y;	// 翻转Y坐标
}
