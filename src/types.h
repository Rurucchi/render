/* 
	types & math
*/

#ifndef _TYPESH_
#define _TYPESH_

#define Assert(expression)                                                     \
  if(!(expression)) {                                                          \
    *(int *)0 = 0;                                                             \
}

#define AssertHR(hr) Assert(SUCCEEDED(hr))

typedef unsigned int uint;

typedef uint8_t ui8;
typedef uint16_t ui16;
typedef uint32_t ui32;
typedef uint64_t ui64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef float f32;
typedef double f64;

// math types

typedef Vector2 v2;
typedef Vector3 v3;
typedef Vector4 v4;
typedef Matrix mx;

ui32 SafeTruncateUInt64(ui64 value){
  Assert(value <= 0xFFFFFFFF);
  ui32 result = (ui32)value;
  return result;
}

ui32 int_to_ui32(int x) {
	return (ui32)x << 0;
};

// math
float slope(float min, float max, float cap) {
	return (max - min) / cap;
};

float lerp(float v0, float v1, float t) {
  return v0 + t * (v1 - v0);
}

float clamp(float value, float min, float max) {
    if (value < min) {
        return min;
    } else if (value > max) {
        return max;
    } else {
        return value;
    }
}

// todo: operator overloading

#endif /* _TYPESH_ */