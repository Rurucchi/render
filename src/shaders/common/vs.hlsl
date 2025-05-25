#ifndef _PSH_
#define _PSH_

PS_INPUT vs(VS_INPUT input) {
	PS_INPUT output;
	
	// rotation + pos transform
    output.pos = mul(float4(input.pos, 1), view_projection);
	
	// texture coords
	output.uv = input.uv;
	
    return output;
}

#endif // _PSH_