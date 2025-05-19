/*
    windows api
*/

#ifndef _PLATFORMH_
#define _PLATFORMH_

struct viewport_size {
	i32 width;
	i32 height;
};

// functions

static void FatalError(const char* message)
{
	/*
	https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-messageboxa
	https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-exitprocess */
	
    MessageBoxA(NULL, message, "Error", MB_ICONEXCLAMATION);
    ExitProcess(0);
}

static LRESULT CALLBACK WindowProc(HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	/*
	https://learn.microsoft.com/en-us/windows/win32/api/winuser/nc-winuser-wndproc */
	
    if(ImGui_ImplWin32_WndProcHandler(wnd, msg, wparam, lparam))
    {
        return true;
    }
    switch (msg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    
    return DefWindowProcW(wnd, msg, wparam, lparam);
}

ui32 platform_get_clock_speed() {
	/*
	https://learn.microsoft.com/en-us/windows/win32/api/profileapi/nf-profileapi-queryperformancefrequency */
	
	LARGE_INTEGER queryClock;
	QueryPerformanceFrequency(&queryClock);
	ui32 clockFrequency = SafeTruncateUInt64(queryClock.QuadPart);
	return clockFrequency;
};

i64 platform_get_tick(){
	/*
	https://learn.microsoft.com/en-us/windows/win32/api/profileapi/nf-profileapi-queryperformancecounter */
	
	LARGE_INTEGER ticks;
    if (!QueryPerformanceCounter(&ticks))
    {
        FatalError("QueryPerformanceCounter failed!");
    }
    return ticks.QuadPart;
}

f64 platform_get_time(i32 clock){
	i64 tick = platform_get_tick();
	f64 time = tick / (f64)clock;
    return time; // time in seconds (f64 cuz we need precision)
}


HWND platform_create_window(HINSTANCE instance, int width, int height) {
	/*
	https://learn.microsoft.com/en-us/windows/apps/develop/ui-input/retrieve-hwnd
	https://learn.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-wndclassexw
	https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createwindowexa
	https://learn.microsoft.com/en-us/windows/win32/dataxchg/about-atom-tables */
	
    // register window class to have custom WindowProc callback
    WNDCLASSEXW wc =
    {
        .cbSize = sizeof(wc),
        .lpfnWndProc = WindowProc,
        .hInstance = instance,
        .hIcon = LoadIcon(NULL, IDI_APPLICATION),
        .hCursor = LoadCursor(NULL, IDC_ARROW),
        .lpszClassName = L"d3d11_window_class",
    };
    ATOM atom = RegisterClassExW(&wc);
    Assert(atom && "Failed to register window class");

    // window properties - width, height and style

    // WS_EX_NOREDIRECTIONBITMAP flag here is needed to fix ugly bug with Windows 10
    // when window is resized and DXGI swap chain uses FLIP presentation model
    // DO NOT use it if you choose to use non-FLIP presentation model
    // read about the bug here: https://stackoverflow.com/q/63096226 and here: https://stackoverflow.com/q/53000291
    DWORD exstyle = WS_EX_APPWINDOW | WS_EX_NOREDIRECTIONBITMAP;
    DWORD style = WS_OVERLAPPEDWINDOW;

    // uncomment in case you want fixed size window
    // style &= ~WS_THICKFRAME & ~WS_MAXIMIZEBOX;
    // RECT rect = { 0, 0, 1280, 720 };
    // AdjustWindowRectEx(&rect, style, FALSE, exstyle);
    // width = rect.right - rect.left;
    // height = rect.bottom - rect.top;

    // create window
    HWND window = CreateWindowExW(
        exstyle, wc.lpszClassName, L"3d renderer", style,
        CW_USEDEFAULT, CW_USEDEFAULT, width, height,
        NULL, NULL, wc.hInstance, NULL);
    Assert(window && "Failed to create window");
	
    return window;
}

viewport_size platform_get_window_size(HWND window) {
	/*
	https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getclientrect */
	
	RECT rect;
	GetClientRect(window, &rect);
		
	viewport_size newWindowSize = {
	(i32)rect.right - rect.left << 0,
	(i32)rect.bottom - rect.top << 0,
	};
		
	return newWindowSize;
};

#endif /* _PLATFORMH_ */