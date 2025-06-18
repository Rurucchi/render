/*
	main, entry point
*/

#define internal static
#define local_persist static
#define global_variable static

#define STR2(x) #x
#define STR(x) STR2(x)

// compiler options
#define COBJMACROS
#define WIN32_LEAN_AND_MEAN
#define _USE_MATH_DEFINES

#pragma comment (lib, "gdi32")
#pragma comment (lib, "user32")
#pragma comment (lib, "dxguid")
#pragma comment (lib, "dxgi")
#pragma comment (lib, "d3d11")
#pragma comment (lib, "d3dcompiler")

// std
#include <math.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>

// os
#include <windows.h>
#include <combaseapi.h>
#include <d3d11.h>
#include <dxgi1_3.h>
#include <d3dcompiler.h>
#include <dxgidebug.h>

// raylib 
// #define RL_VECTOR2_TYPE
// #define RL_VECTOR3_TYPE
// #define RL_MATRIX_TYPE
#define RAYMATH_IMPLEMENTATION
#include "raylib/raymath.h"
// #include "raylib/rcamera_ex.h"
#define RCAMERA_IMPLEMENTATION
#define RCAMERA_STANDALONE
#include "raylib/rcamera.h"

// stb
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

// imgui
#define IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_IMPLEMENTATION
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.cpp"
#include "imgui_impl_dx11.cpp"
#include "imgui.cpp"
#include "imgui_draw.cpp"
#include "imgui_tables.cpp"
#include "imgui_widgets.cpp"

// custom
#include "types.h"
#include "platform/platform.h"
#include "platform/io.h"
#include "asset/parser.h"
#include "render/render.h"
#include "render/ui.h"

// const f32 DEG_TO_RAD = PI / 180.0f;
	
enum mouse_mode { FREE, CAMERA };

struct mouse_settings {
	f32 sensitivity;
	mouse_mode current_mouse_mode;
	POINT mouse_pos;
};
	
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

int WINAPI WinMain(HINSTANCE instance, HINSTANCE previnstance, LPSTR cmdline, int cmdshow)
{
	HRESULT hr;
	
	// window instance
	ui32 width = CW_USEDEFAULT;
    ui32 height = CW_USEDEFAULT;
	HWND window = platform_create_window(instance, width, height);
	
	ui32 platformClockSpeed = platform_get_clock_speed();
	LARGE_INTEGER freq, c1;
	QueryPerformanceCounter(&c1);
	
	f64 currentTime = platform_get_time(platformClockSpeed);
    
	// camera
	Camera camera = { 0 };
    camera.position = { 0.0f, 0.0f, -20.0f };  // Camera position
    camera.target = { 0.0f, 0.0f, 0.0f };      // Camera looking at point
    camera.up = { 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 90.0f;                       // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;    // Camera mode type
	
	camera_settings cam_settings = {
		.yaw = 0.0f,
		.pitch = 0.0f,
		.dist = -20.0f,
	};
	
	// light
	light_source light = {
		.intensity = 1.0f,
		.pos = {20, 20, 20},
		.color = {1.0f, 1.0f, 1.0f, 1.0f}
	};
	
	// render context
	render_context rContext = {0};
	ui_context uiContext = {
		.fps = 0,
		.fps_display_delay = 0.1f, // in seconds
	};
	
	// init renderer
	hr = render_init_d3d11(window, &rContext);


    // show window
    ShowWindow(window, SW_SHOWDEFAULT);

	imgui_init(window, rContext);
	
	viewport_size window_size = {
		.width = 0,
		.height = 0,
	};

	mouse_settings current_mouse_settings = {
		.sensitivity = 0.003f,
		.current_mouse_mode = FREE,
		.mouse_pos = {0, 0},
	};
	
	vertex vertices_data[] =
	{
		// Front face
		{{-0.5f, -0.5f, -0.5f}, {0.0f,  0.0f, -1.0f}, {0.0f, 1.0f}}, // 0
		{{ 0.5f, -0.5f, -0.5f}, {0.0f,  0.0f, -1.0f}, {1.0f, 1.0f}}, // 1
		{{ 0.5f,  0.5f, -0.5f}, {0.0f,  0.0f, -1.0f}, {1.0f, 0.0f}}, // 2
		{{-0.5f,  0.5f, -0.5f}, {0.0f,  0.0f, -1.0f}, {0.0f, 0.0f}}, // 3

		// Back face
		{{-0.5f, -0.5f,  0.5f}, {0.0f,  0.0f, 1.0f}, {0.0f, 1.0f}}, // 4
		{{ 0.5f, -0.5f,  0.5f}, {0.0f,  0.0f, 1.0f}, {1.0f, 1.0f}}, // 5
		{{ 0.5f,  0.5f,  0.5f}, {0.0f,  0.0f, 1.0f}, {1.0f, 0.0f}}, // 6
		{{-0.5f,  0.5f,  0.5f}, {0.0f,  0.0f, 1.0f}, {0.0f, 0.0f}}, // 7
		
		
		// Right face
		{{-0.5f, -0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}, {0.0f, 1.0f}}, // 8
		{{-0.5f, -0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}, {1.0f, 1.0f}}, // 9
		{{-0.5f,  0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}, {1.0f, 0.0f}}, // 10
		{{-0.5f,  0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}, {0.0f, 0.0f}}, // 11
		
		// Left face
		{{ 0.5f,  0.5f,  0.5f}, {1.0f,  0.0f,  0.0f}, {0.0f, 0.0f}}, // 15
		{{ 0.5f,  0.5f, -0.5f}, {1.0f,  0.0f,  0.0f}, {1.0f, 0.0f}}, // 14
		{{ 0.5f, -0.5f, -0.5f}, {1.0f,  0.0f,  0.0f}, {1.0f, 1.0f}}, // 13
		{{ 0.5f, -0.5f,  0.5f}, {1.0f,  0.0f,  0.0f}, {0.0f, 1.0f}}, // 12
		
		// Top face
		{{-0.5f,  0.5f, -0.5f}, {0.0f,  1.0f,  0.0f}, {0.0f, 1.0f}}, // 16
		{{ 0.5f,  0.5f, -0.5f}, {0.0f,  1.0f,  0.0f}, {1.0f, 1.0f}}, // 17
		{{ 0.5f,  0.5f,  0.5f}, {0.0f,  1.0f,  0.0f}, {1.0f, 0.0f}}, // 18
		{{-0.5f,  0.5f,  0.5f}, {0.0f,  1.0f,  0.0f}, {0.0f, 0.0f}}, // 19
		
		// Bottom face
		{{-0.5f,  -0.5f,  0.5f}, {0.0f, -1.0f,  0.0f}, {0.0f, 0.0f}}, // 20
		{{ 0.5f,  -0.5f,  0.5f}, {0.0f, -1.0f,  0.0f}, {1.0f, 0.0f}}, // 21
		{{ 0.5f,  -0.5f, -0.5f}, {0.0f, -1.0f,  0.0f}, {1.0f, 1.0f}}, // 22
		{{-0.5f,  -0.5f, -0.5f}, {0.0f, -1.0f,  0.0f}, {0.0f, 1.0f}}, // 23
	};
	
	
	// vertices order are defined by culling mode.
	ui16 indices[] =
	{
		// Front face
		0, 2, 1,  2, 0, 3,

		// Back face 	
		5, 7, 4,  7, 5, 6,

		// Right face
		8, 10, 9,  10, 8, 11,

		// Left face
		12, 14, 13,  14, 12, 15,

		// Top face
		16, 18, 17,  18, 16, 19,

		// Bottom face
		20, 22, 21,  22, 20, 23
	};
	
	mesh mesh_data = {
		.vertex_count = 24,
		.vertices = vertices_data,
		.index_count = 36,
		.indices = indices,
	};
	
	//  ------------------------------------------- frame loop
	
	
	
	for (;;)
    {
		// update states here
		f64 new_time = platform_get_time(platformClockSpeed);
		// window_size = platform_get_window_size(window);
	
        // windows api (todo: process this on another thread later)
        MSG msg;
        if (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                break;
            }
			
			if (msg.message == WM_KEYDOWN) {
				if (msg.wParam == VK_ESCAPE) {
					current_mouse_settings.current_mouse_mode = FREE;
					ShowCursor(TRUE);
				}
			}
			
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
            continue;
        }
		
		
		// handle camera movement with mouse input 
		{
			GetCursorPos(&current_mouse_settings.mouse_pos);
			ScreenToClient(window, &current_mouse_settings.mouse_pos);
			
			viewport_size window_vp = platform_get_window_size(window);
			
			POINT middle = {window_vp.width/2, window_vp.height/2};
			
			if(current_mouse_settings.current_mouse_mode == CAMERA) {
				POINT mouse_offset = {
					.x = current_mouse_settings.mouse_pos.x - middle.x,
					.y = current_mouse_settings.mouse_pos.y - middle.y
				};
				
				camera_movement(&camera, &cam_settings, mouse_offset, current_mouse_settings.sensitivity);
				
				ClientToScreen(window, &middle);
				SetCursorPos(middle.x, middle.y);
				
			};
		}
		
		
		// states
		
		    LARGE_INTEGER c2;
            QueryPerformanceCounter(&c2);
            f32 delta = (f32)((f64)(c2.QuadPart - c1.QuadPart) / platformClockSpeed);
            c1 = c2;
			
			// update fps
			update_ui_context(&uiContext, 1.0f/delta, new_time);
		
		// render

        // reset frame and rendering data
		render_reset_frame(&rContext);
		
		// resize swap chain if needed + updates window_size
		render_resize_swapchain(window, &window_size, &rContext);

        // can render only if window size is non-zero - we must have backbuffer & RenderTarget view created
        if (rContext.rtView)
        {
            // reset all pipeline states and input assembler
			render_pipeline_states(&rContext, &window_size);
			
			// clear screen
			f32 color[4] = { 0.2f, 0.2f, 0.2f, 1.f }; // black
			render_clear_screen(&rContext, color);
			
			// upload to gpu before rendering
			
			// resize the camera and upload it
			render_upload_frame_buffer(&rContext, &camera, window_size);
			
			// draw
			render_draw_mesh(&rContext, mesh_data);
			
			// imgui
			imgui_render();
			
			if(ImGui::Begin("test")){
				ImGui::Text("FPS : %f", uiContext.fps);
				ImGui::Text("[Mouse coords] X: %d Y: %d", current_mouse_settings.mouse_pos.x, current_mouse_settings.mouse_pos.y);
				ImGui::Text("[Camera target] X: %f Y: %f", camera.target.x, camera.target.y);
				
				if(ImGui::Button("camera mode")){
					if(current_mouse_settings.current_mouse_mode == FREE) {
						current_mouse_settings.current_mouse_mode = CAMERA;
						ShowCursor(FALSE);
					} else {
						current_mouse_settings.current_mouse_mode = FREE;
						ShowCursor(TRUE);
					};
				};
				
				if(current_mouse_settings.current_mouse_mode == FREE) {
					ImGui::Text("Mouse mode: FREE");
				} else {
					ImGui::Text("Mouse mode: CAMERA");
				};
				
			} ImGui::End();

			ImGui::Render();
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        }

        // change to FALSE to disable vsync
        BOOL vsync = FALSE;
        hr = rContext.swapChain->Present(vsync ? 1 : 0, 0);
		
		// debug code
		hr = rContext.device->GetDeviceRemovedReason();
		
        if (hr == DXGI_STATUS_OCCLUDED)
        {
            // window is minimized, cannot vsync - instead sleep a bit
            if (vsync)
            {
                Sleep(10);
            }
        }	
        else if (FAILED(hr))
        {
            FatalError("Failed to present swap chain! Device lost?");
        }
		
		currentTime = new_time;
    }
}