// entry point

#include "./types.hlsl"

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

#include "./vs.hlsl"
#include "./ps.hlsl"



