#ifndef _LIGHTH_
#define _LIGHTH_

float4 phong(float4 tex, float4 color, float ambient) {
	// ambient
	return color * tex * ambient;
}

#endif // _LIGHTH_