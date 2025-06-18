#ifndef _RENDERTYPESH_
#define _RENDERTYPESH_

struct vertex
{
    v3 pos;
	v3 normal;
    v2 uv;
};

struct mesh
{
	v3 pos;
	ui32 vertex_count;
	vertex* vertices;
	ui16 index_count;
	ui16* indices;
};

struct light {
	f32 intensity;
	v3 pos;
	v4 color;
};

struct render_frame_buffer {
	Camera* camera;
};


// constant buffer
struct shader_frame_buffer {
	mx matrix;
	light* light_sources;
};

#endif /* _RENDERTYPESH_ */