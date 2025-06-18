// Vertex Shader

#include "./common/types.hlsl"

cbuffer cbuffer0 : register(b0)	{
	float4x4 view_projection;
}

cbuffer cbuffer1 : register(b1)	{
	float4x4 world;
}

// s0 = sampler bound to slot 0
sampler sampler0 : register(s0);	

// t0 = shader resource bound to slot 0
Texture2D<float4> texture0 : register(t0);

PS_INPUT vs(VS_INPUT input) {
	PS_INPUT output;
	
	// rotation + pos transform
    output.pos = mul(float4(input.pos, 1), view_projection);
	
	// forward normal vector
	output.normal = input.normal;
	
	// texture coords
	output.uv = input.uv;
	
	
    return output;
}