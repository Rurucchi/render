#ifndef _TYPESH_
#define _TYPESH_

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

#endif // _TYPESH_