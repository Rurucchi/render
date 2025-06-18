#include "render_types.h"
#include "dx11.h"
#include "../types.h"


// data processing

mx render_create_frame_buffer(Camera* camera, viewport_size vp) {
	
	float width = (float)vp.width;
	float height = (float)vp.height;
	
	// raylib camera
	mx matrix = GetCameraViewMatrix(camera);
	mx proj_matrix = GetCameraProjectionMatrix(camera, width / height);
	
	mx matrix_scaled = MatrixScale(10, 10, 10);
	matrix = MatrixMultiply(matrix_scaled, matrix);
	
	matrix = MatrixMultiply(matrix, proj_matrix);
	
	
};

// api calls