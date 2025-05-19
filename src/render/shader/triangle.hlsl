struct VS_INPUT {
	float3 pos   : POSITION;		// these names must match D3D11_INPUT_ELEMENT_DESC array
	float3 normal : NORMAL;
	float2 uv    : TEXCOORD;
};

struct PS_INPUT {
	float4 pos   : SV_POSITION; 	// these names do not matter, except SV_... ones
	float3 normal : NORMAL;
	float2 uv    : TEXCOORD;
};


// ------- frame buffer
// b0 = constant buffer bound to slot 0 
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
	
	// texture coords
	output.uv = input.uv;
	
    return output;
}

float4 ps(PS_INPUT input) : SV_TARGET {
	// ambient light
	float ambientStrength = 0.3;

	// texture
	float4 tex = texture0.Sample(sampler0, input.uv);

    return float4(1.0f, 1.0f, 1.0f, 1.0f) * tex * ambientStrength;
}

// todo: refactor shaders 