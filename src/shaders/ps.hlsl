// Pixel (Fragment) Shader

#include "./common/types.hlsl"
#include "./light/light_models.hlsl"

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

float4 ps(PS_INPUT input) : SV_TARGET {
	// ambient light
	float ambientStrength = 0.3;

	// texture
	float4 color = float4(1.0f, 1.0f, 1.0f, 1.0f);
	float4 tex = texture0.Sample(sampler0, input.uv);

    return phong(tex, color, ambientStrength);
}