/*
	imgui + ui system
*/

#ifndef _UIH_
#define _UIH_


// ----------------------- STATES
struct ui_context {
	f32 fps;
	f32 fps_display_delay; // this should be in seconds
	f64 last_update; // last timestamp fps was updated (in seconds)
};

void update_ui_context(ui_context* uiContext, f32 fps, f64 time) {
	// here we will periodically update the FPS number.
	// if the conditions doesn't match we just keep the old value as we need it for display_fps
	
	if(time - uiContext->last_update > uiContext->fps_display_delay){
		uiContext->fps = fps;
		uiContext->last_update = time;
	};
};



// ----------------------- IMGUI STUFF

void imgui_init(HWND window, render_context rContext) {
	// IMGUI Init

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui::StyleColorsDark();
    
    ImGui_ImplWin32_Init(window);
    ImGui_ImplDX11_Init(rContext.device, rContext.context);
}

void imgui_render() {
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

#endif /* _UIH_ */