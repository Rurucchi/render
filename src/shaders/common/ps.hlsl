#ifndef _PSH_
#define _PSH_

float4 ps(PS_INPUT input) : SV_TARGET {
	// ambient light
	float ambientStrength = 0.3;

	// texture
	float4 tex = texture0.Sample(sampler0, input.uv);

    return float4(1.0f, 1.0f, 1.0f, 1.0f) * tex * ambientStrength;
}

#endif // _PSH_