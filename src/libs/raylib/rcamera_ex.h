	#ifndef _RCAMERAEXH_
	#define _RCAMERAEXH_
	
	// Camera type, defines a camera position/orientation in 3d space
    struct Camera {
        Vector3 position;       // Camera position
        Vector3 target;         // Camera target it looks-at
        Vector3 up;             // Camera up vector (rotation over its axis)
        float fovy;             // Camera field-of-view apperture in Y (degrees) in perspective, used as near plane width in orthographic
        int projection;         // Camera projection type: CAMERA_PERSPECTIVE or CAMERA_ORTHOGRAPHIC
    };

    // Camera3D Camera;    // Camera type fallback, defaults to Camera3D

    // Camera projection
    enum {
        CAMERA_PERSPECTIVE = 0, // Perspective projection
        CAMERA_ORTHOGRAPHIC     // Orthographic projection
    } CameraProjection;

    // Camera system modes
    enum {
        CAMERA_CUSTOM = 0,      // Camera custom, controlled by user (UpdateCamera() does nothing)
        CAMERA_FREE,            // Camera free mode
        CAMERA_ORBITAL,         // Camera orbital, around target, zoom supported
        CAMERA_FIRST_PERSON,    // Camera first person
        CAMERA_THIRD_PERSON     // Camera third person
    } CameraMode;
	
#endif /* _RCAMERAEXH_ */