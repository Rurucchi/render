#ifndef _CAMERAH_
#define _CAMERAH_

struct camera_settings {
	// in radians
	f32 yaw; // clamped between 0 and 360 (circular movement)
	f32 pitch; // clamped between 0 and 179 (prevent flipping)
	
	// game units
	f32 dist; // distance from mesh
};
	
void camera_movement(Camera* camera, camera_settings* settings, POINT mouse_offset, f32 sensitivity){
	
	v3 pos = {0};
	
	// adjust camera settings
	settings->yaw = fmod(settings->yaw + (mouse_offset.x * sensitivity), 2 * M_PI);
	settings->pitch = clamp(settings->pitch + (mouse_offset.y * sensitivity), -1.55334, 1.553344);

	// get position vector
	pos.x = sinf(settings->yaw);
	pos.z = cosf(settings->yaw);
	pos.y = tanf(settings->pitch);
	
	// normalize it
	v3 cam_pos = Vector3Scale(Vector3Normalize(pos), settings->dist);
	
	// apply
	camera->position.x = cam_pos.x;
	camera->position.y = cam_pos.y;
	camera->position.z = cam_pos.z;
};

#endif /* _CAMERAH_ */