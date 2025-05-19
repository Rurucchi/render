/*
	rendering with D3D11.
*/

#pragma comment (lib, "gdi32")
#pragma comment (lib, "user32")
#pragma comment (lib, "dxguid")
#pragma comment (lib, "dxgi")
#pragma comment (lib, "d3d11")
#pragma comment (lib, "d3dcompiler")

#ifndef _RENDERH_
#define _RENDERH_

#include <windows.h>
#include <d3d11.h>
#include <dxgi1_3.h>
#include <d3dcompiler.h>
#include <dxgidebug.h>

// structs

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

struct light_source {
	f32 intensity;
	v3 pos;
	v4 color;
};

// prone to change.
struct render_context {
	// device
	ID3D11Device* device;
	ID3D11DeviceContext* context;
	IDXGISwapChain1* swapChain;
	
	// states
	ID3D11RenderTargetView* rtView;
	ID3D11DepthStencilView* dsView; 
	ID3D11InputLayout* layout;
	ID3D11RasterizerState* rasterizerState;
	

	ID3D11ShaderResourceView* textureView;
	ID3D11DepthStencilState* depthState;
	ID3D11BlendState* blendState;
	ID3D11SamplerState* sampler;
	
	// shaders
	ID3D11VertexShader* vshader;
    ID3D11PixelShader* pshader;
	
	// buffers
	int vCount;
	int indexCount;
	vertex vQueue[16384];
	ui32 indexQueue[16384];
	ID3D11Buffer* vertex_buffer;
	ID3D11Buffer* index_buffer;
	ID3D11Buffer* frame_buffer; // buffer static to the frame
	ID3D11Buffer* object_buffer; // updated for each object drawn (inefficient)
	
};

// pipeline
void render_queue_vertex(render_context* rContext, mesh mesh_data) {
	for(int i=0; i < mesh_data.index_count; i++) {
		rContext->indexQueue[rContext->indexCount] = mesh_data.indices[i];
		rContext->indexCount++;
	};
};

void render_queue_index(render_context* rContext, mesh mesh_data) {
	for(int i=0; i < mesh_data.vertex_count; i++) {
		rContext->vQueue[rContext->vCount] = mesh_data.vertices[i];
		rContext->vCount++;
	};
};


void render_resize_swapchain(HWND window, viewport_size* window_size, render_context* rContext) {
	
		HRESULT hr;
	
        // get current size for window client area
		viewport_size new_window_size = platform_get_window_size(window);
		
		
	    if (rContext->rtView == NULL || new_window_size.width != window_size->width || new_window_size.height != window_size->height) {
            if (rContext->rtView)
            {
                // release old swap chain buffers
                rContext->context->ClearState();
                rContext->rtView->Release();
                rContext->dsView->Release();
                rContext->rtView = NULL;
            }

            // resize to new size for non-zero size
            if (new_window_size.width != 0 && new_window_size.height != 0)
            {
                hr = rContext->swapChain->ResizeBuffers(0, new_window_size.width, new_window_size.height, DXGI_FORMAT_UNKNOWN, 0);
                if (FAILED(hr))
                {
                    FatalError("Failed to resize swap chain!");
                }

                // create RenderTarget view for new backbuffer texture
                ID3D11Texture2D* backbuffer;
                hr = rContext->swapChain->GetBuffer(0, IID_ID3D11Texture2D, (void**)&backbuffer);
				// rContext->swapChain->GetBuffer(0, IID_PPV_ARGS(&backbuffer));
                hr = rContext->device->CreateRenderTargetView(backbuffer, NULL, &rContext->rtView);
                backbuffer->Release();

                D3D11_TEXTURE2D_DESC depthDesc = 
                {
                    .Width = int_to_ui32(new_window_size.width),
                    .Height = int_to_ui32(new_window_size.height),
                    .MipLevels = 1,
                    .ArraySize = 1,
                    .Format = DXGI_FORMAT_D24_UNORM_S8_UINT, // or use DXGI_FORMAT_D32_FLOAT_S8X24_UINT if you need stencil
                    .SampleDesc = { 1, 0 },
                    .Usage = D3D11_USAGE_DEFAULT,
                    .BindFlags = D3D11_BIND_DEPTH_STENCIL,
                };

                // create new depth stencil texture & DepthStencil view
				D3D11_DEPTH_STENCIL_VIEW_DESC dvd = {};
				
					dvd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
					dvd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
					dvd.Texture2D.MipSlice = 0;
				
                ID3D11Texture2D* depth;
                rContext->device->CreateTexture2D(&depthDesc, NULL, &depth);
                rContext->device->CreateDepthStencilView(depth, &dvd, &rContext->dsView);
                depth->Release();
            }

			window_size->width = new_window_size.width;
			window_size->height = new_window_size.height;
        }
};

void render_pipeline_states(render_context* rContext, viewport_size* vpSize){

	D3D11_VIEWPORT viewport =
	{
		.TopLeftX = 0,
		.TopLeftY = 0,
		.Width = (FLOAT)vpSize->width,
		.Height = (FLOAT)vpSize->height,
		.MinDepth = 0,
		.MaxDepth = 1,
	};

	{
		// Input Assembler
		rContext->context->IASetInputLayout(rContext->layout);
		rContext->context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		UINT stride = sizeof(struct vertex);
		UINT offset = 0;
		
		// Vertex Buffer
		rContext->context->IASetVertexBuffers(0, 1, &rContext->vertex_buffer, &stride, &offset);

		// Index Buffer
		rContext->context->IASetIndexBuffer(rContext->index_buffer, DXGI_FORMAT_R32_UINT, 0);

		// Vertex Shader 
		rContext->context->VSSetShader(rContext->vshader, NULL, 0);
		
		// Bind buffers
		ID3D11Buffer* constant_buffers[] = {rContext->frame_buffer, rContext->object_buffer};
		rContext->context->VSSetConstantBuffers(0, 2, constant_buffers); // 2 buffers for now

		// Rasterizer Stage
		rContext->context->RSSetViewports(1, &viewport);
		rContext->context->RSSetState(rContext->rasterizerState);

		// Pixel Shader
		rContext->context->PSSetSamplers(0, 1, &rContext->sampler);
		rContext->context->PSSetShaderResources(0, 1, &rContext->textureView);
		rContext->context->PSSetShader(rContext->pshader, NULL, 0);

		// Output Merger
		rContext->context->OMSetBlendState(rContext->blendState, NULL, 0xffffffff);
		rContext->context->OMSetDepthStencilState(rContext->depthState, 0);
		rContext->context->OMSetRenderTargets(1, &rContext->rtView, rContext->dsView);
	};
};

void render_clear_screen(render_context* rContext, f32 color[4]){
        rContext->context->ClearRenderTargetView(rContext->rtView, color);
        rContext->context->ClearDepthStencilView(rContext->dsView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);
};

void render_reset_frame(render_context* rContext){
	rContext->vCount = 0;
	rContext->indexCount = 0;
};

void render_upload_vertex_buffer(render_context* rContext){
	D3D11_MAPPED_SUBRESOURCE mapped;
	
	rContext->context->Map((ID3D11Resource*)rContext->vertex_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	memcpy(mapped.pData, rContext->vQueue, sizeof(vertex)*rContext->vCount);
	rContext->context->Unmap((ID3D11Resource*)rContext->vertex_buffer, 0);
};


void render_upload_index_buffer(render_context* rContext){
	D3D11_MAPPED_SUBRESOURCE mapped;
	
	rContext->context->Map((ID3D11Resource*)rContext->index_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	memcpy(mapped.pData, rContext->indexQueue, sizeof(ui32)*rContext->indexCount);
	rContext->context->Unmap((ID3D11Resource*)rContext->index_buffer, 0);
};

void render_upload_frame_buffer(render_context *rContext, Camera* camera, viewport_size vp){	
	
	float width = (float)vp.width;
	float height = (float)vp.height;
	
	// raylib camera
	mx matrix = GetCameraViewMatrix(camera);
	mx proj_matrix = GetCameraProjectionMatrix(camera, width / height);
	
	mx matrix_scaled = MatrixScale(10, 10, 10);
	matrix = MatrixMultiply(matrix_scaled, matrix);
	
	matrix = MatrixMultiply(matrix, proj_matrix);
	
	D3D11_MAPPED_SUBRESOURCE mapped;
	
	rContext->context->Map(rContext->frame_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	memcpy(mapped.pData, &matrix, sizeof(matrix));
	rContext->context->Unmap(rContext->frame_buffer, 0);
};

void render_draw_mesh(render_context* rContext,mesh mesh_data){
	render_queue_vertex(rContext, mesh_data);
	render_queue_index(rContext, mesh_data);
	render_upload_index_buffer(rContext);
	render_upload_vertex_buffer(rContext);
	rContext->context->DrawIndexed(rContext->indexCount, 0, 0);
};

// ----------- init stuff

// --- buffers

HRESULT render_create_mesh_buffer(render_context *rContext, int vertex_buffer_size, int index_buffer_size) {
	HRESULT hr;

	D3D11_BUFFER_DESC vertex_desc =
    {
        .ByteWidth = sizeof(vertex) * vertex_buffer_size,
		.Usage = D3D11_USAGE_DYNAMIC,
		.BindFlags = D3D11_BIND_VERTEX_BUFFER,
		.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
	};
	
	
	D3D11_BUFFER_DESC index_desc =
    {
        .ByteWidth = sizeof(ui16) * index_buffer_size,
		.Usage = D3D11_USAGE_DYNAMIC,
		.BindFlags = D3D11_BIND_INDEX_BUFFER,
		.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
	};

	D3D11_SUBRESOURCE_DATA initial = { .pSysMem = rContext->vQueue };
    hr = rContext->device->CreateBuffer(&vertex_desc, NULL, &rContext->vertex_buffer);
	hr = rContext->device->CreateBuffer(&index_desc, NULL, &rContext->index_buffer);
	
	return hr;
}

HRESULT render_create_frame_buffer(render_context* rContext) {
	HRESULT hr;
	
	// todo: fix this
	D3D11_BUFFER_DESC desc =
    {
        .ByteWidth = sizeof(mx) * 2,
		.Usage = D3D11_USAGE_DYNAMIC,
		.BindFlags = D3D11_BIND_CONSTANT_BUFFER,
		.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
	};

    hr = rContext->device->CreateBuffer(&desc, NULL, &rContext->frame_buffer); 
	
	return hr;
};

HRESULT render_create_object_buffer(render_context* rContext) {
	HRESULT hr;
	
	// todo: fix this
	D3D11_BUFFER_DESC desc =
    {
        .ByteWidth = sizeof(mx) * 2,
		.Usage = D3D11_USAGE_DYNAMIC,
		.BindFlags = D3D11_BIND_CONSTANT_BUFFER,
		.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
	};

    hr = rContext->device->CreateBuffer(&desc, NULL, &rContext->object_buffer); 
	
	return hr;
};

HRESULT render_load_shaders(render_context* rContext) {
	HRESULT hr;
	
	
	// IA decs
	// these must match vertex shader input layout (VS_INPUT in vertex shader source below)
	D3D11_INPUT_ELEMENT_DESC desc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, offsetof(vertex, pos), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, offsetof(vertex, normal),       D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",    0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(vertex, uv),    D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	
	char vsLocation[] = "triangle.vs.fxc";
	char psLocation[] = "triangle.ps.fxc";
	
	complete_file vblob = {0};
	complete_file pblob = {0};
	
	io_file_fullread(vsLocation, &vblob);
	io_file_fullread(psLocation, &pblob);
	
	
	hr = rContext->device->CreateVertexShader(vblob.memory, vblob.size, NULL, &rContext->vshader);
	hr = rContext->device->CreatePixelShader(pblob.memory, pblob.size, NULL, &rContext->pshader);
	hr = rContext->device->CreateInputLayout(desc, ARRAYSIZE(desc), vblob.memory, vblob.size, &rContext->layout);

	io_file_fullfree(&vblob);
	io_file_fullfree(&pblob);
	
	return hr;
};

HRESULT render_init_textures(render_context* rContext) {
	HRESULT hr;
	
	// todo: rewrite all of this
		
	// open and decode the texture
	char location[] = "texture.png";
	complete_file texture_file = {0};
	complete_img tex = parse_decode_img(location, &texture_file);

    D3D11_TEXTURE2D_DESC desc =
    {
        .Width = tex.x,
        .Height = tex.y,
        .MipLevels = 1,
        .ArraySize = 1,
        .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
        .SampleDesc = { 1, 0 },
        .Usage = D3D11_USAGE_IMMUTABLE,
        .BindFlags = D3D11_BIND_SHADER_RESOURCE,
    };

    D3D11_SUBRESOURCE_DATA data =
    {
		.pSysMem = tex.memory,
		.SysMemPitch = tex.x * tex.channels_in_file,
    };

    ID3D11Texture2D* texture;
    hr = rContext->device->CreateTexture2D(&desc, &data, &texture);
    hr = rContext->device->CreateShaderResourceView((ID3D11Resource*)texture, NULL, &rContext->textureView);
    texture->Release();
	io_file_fullfree(&texture_file);
	
	return hr;
};

HRESULT render_init_sampler(render_context* rContext) {
	HRESULT hr;
	
	/*
	https://learn.microsoft.com/en-us/windows/win32/api/d3d11/nf-d3d11-id3d11device-createsamplerstate
	https://learn.microsoft.com/en-us/windows/win32/api/d3d11/ns-d3d11-d3d11_sampler_desc */
	
    D3D11_SAMPLER_DESC desc =
    {
		.Filter = D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR,
		.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP,
		.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP,
		.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP,
		.MipLODBias = 0,
		.MaxAnisotropy = 1,
		.MinLOD = 0,
		.MaxLOD = D3D11_FLOAT32_MAX,
	};

    hr = rContext->device->CreateSamplerState(&desc, &rContext->sampler);
	
	return hr;
};

HRESULT render_init_ds(render_context* rContext){
	HRESULT hr;
	
	D3D11_DEPTH_STENCIL_DESC desc =
	{
		.DepthEnable = TRUE,
		.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL,
		.DepthFunc = D3D11_COMPARISON_LESS,
		.StencilEnable = TRUE,
		.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK,
		.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK,
    };
	desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
    desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	
	desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
    desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    desc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	
	
	
	hr = rContext->device->CreateDepthStencilState(&desc, &rContext->depthState);
	
	return hr;
};

HRESULT render_init_rasterizer(render_context* rContext){
	HRESULT hr;
	 
	/*
	https://github.com/ssloy/tinyrenderer/wiki/Lesson-2:-Triangle-rasterization-and-back-face-culling
	https://learn.microsoft.com/en-us/windows/win32/api/d3d11/ns-d3d11-d3d11_rasterizer_desc
	https://www.khronos.org/opengl/wiki/Face_Culling */
		
	D3D11_RASTERIZER_DESC desc =
    {
		.FillMode = D3D11_FILL_SOLID,
		.CullMode = D3D11_CULL_BACK,
		.FrontCounterClockwise = TRUE,
		.DepthClipEnable = TRUE,
	};
    hr = rContext->device->CreateRasterizerState(&desc, &rContext->rasterizerState);
	
	return hr;
};

HRESULT render_init_device(render_context* rContext) {
	HRESULT hr;
	
	{
	UINT flags = 0;
		#ifdef NDEBUG 
			// debug messages
			flags |= D3D11_CREATE_DEVICE_DEBUG;
		#endif 
	
	
		D3D_FEATURE_LEVEL levels[] = { D3D_FEATURE_LEVEL_11_0 };
        hr = D3D11CreateDevice(
            NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, flags, levels, ARRAYSIZE(levels),
            D3D11_SDK_VERSION, &rContext->device, NULL, &rContext->context);

        // D3D_DRIVER_TYPE_WARP driver type which enables software rendering (broken driver/remote desktop situations)
		
        AssertHR(hr);
    }
	
	#ifdef NDEBUG
		{
			// enable debug break on API errors
			ID3D11InfoQueue* info;
			rContext->device->QueryInterface(IID_ID3D11InfoQueue, (void**)&info);
			info->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, TRUE);
			info->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, TRUE);
			info->Release();
		}

		{
			// enable debug break for DXGI
			IDXGIInfoQueue* dxgiInfo;
			hr = DXGIGetDebugInterface1(0, IID_IDXGIInfoQueue, (void**)&dxgiInfo);
			AssertHR(hr);
			dxgiInfo->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, TRUE);
			dxgiInfo->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, TRUE);
			dxgiInfo->Release();
		}
		// debugger breaks on errors
	#endif
	
	return hr;
};

HRESULT render_init_swapchain(render_context* rContext, HWND window) {
	HRESULT hr;
	
	/*
	https://learn.microsoft.com/en-us/windows/win32/api/d3d11/nn-d3d11-id3d11query
	https://learn.microsoft.com/en-us/windows/win32/api/dxgi/nn-dxgi-idxgidevice 
	https://learn.microsoft.com/en-us/windows/win32/api/_direct3ddxgi/ 
	https://learn.microsoft.com/en-us/windows/win32/api/dxgi/ 
	https://learn.microsoft.com/en-us/windows/win32/api/dxgi/nn-dxgi-idxgiswapchain 
	https://learn.microsoft.com/en-us/windows/win32/api/dxgi/nn-dxgi-idxgifactory 
	https://learn.microsoft.com/en-us/windows/win32/api/dxgi/nn-dxgi-idxgiadapter */ 
	
	// get DXGI device from D3D11 device
	IDXGIDevice* dxgiDevice;
    hr = rContext->device->QueryInterface(IID_IDXGIDevice, (void**)&dxgiDevice);
    AssertHR(hr);
	
	// get DXGI adapter from DXGI device
    IDXGIAdapter* dxgiAdapter;
    hr = dxgiDevice->GetAdapter(&dxgiAdapter);
    AssertHR(hr);

    // get DXGI factory from DXGI adapter
    IDXGIFactory2* factory;
    hr = dxgiAdapter->GetParent(IID_IDXGIFactory2, (void**)&factory);
    AssertHR(hr);

    DXGI_SWAP_CHAIN_DESC1 desc =
    {
        // default 0 value for width & height means to get it from HWND automatically
        //.Width = 0,
        //.Height = 0,

        // or use DXGI_FORMAT_R8G8B8A8_UNORM_SRGB for storing sRGB
        .Format = DXGI_FORMAT_R8G8B8A8_UNORM,

        // FLIP presentation model does not allow MSAA framebuffer
        // if you want MSAA then you'll need to render offscreen and manually
        // resolve to non-MSAA framebuffer
		// more info: https://all500234765.github.io/graphics/2019/10/28/msaa-dx11/
        .SampleDesc = { 1, 0 },

        .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
        .BufferCount = 2,

        // we don't want any automatic scaling of window content
        // this is supported only on FLIP presentation model
        .Scaling = DXGI_SCALING_NONE,

        // use more efficient FLIP presentation model
        // Windows 10 allows to use DXGI_SWAP_EFFECT_FLIP_DISCARD
        // for Windows 8 compatibility use DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL
        // for Windows 7 compatibility use DXGI_SWAP_EFFECT_DISCARD
        .SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
    };
	
	// create swapchain
    hr = factory->CreateSwapChainForHwnd((IUnknown*)rContext->device, window, &desc, NULL, NULL, &rContext->swapChain);
    AssertHR(hr);

    // disable silly Alt+Enter changing monitor resolution to match window size
    factory->MakeWindowAssociation(window, DXGI_MWA_NO_ALT_ENTER);

	// release init stuff
    factory->Release();
    dxgiAdapter->Release();
    dxgiDevice->Release();
	
	return hr;
};


// main call for init
HRESULT render_init_d3d11(HWND window, render_context* rContext) {
	// https://learn.microsoft.com/en-us/windows/win32/api/d3d11/nf-d3d11-d3d11createdevice
	
	HRESULT hr;
	
	// create D3D11 device & context
	render_init_device(rContext);

	// create DXGI swap chain
	hr = render_init_swapchain(rContext, window);
	
	// init needed buffers 
	hr = render_create_mesh_buffer(rContext, 16384, 16384);
	hr = render_create_frame_buffer(rContext);
	hr = render_create_object_buffer(rContext);
	
	// sampler
	hr = render_init_sampler(rContext);
	
	// rasterizer 
	hr = render_init_rasterizer(rContext);
	
	// depth stencil
	hr = render_init_ds(rContext);
	
	// init shaders
	hr = render_load_shaders(rContext);
	
	// textures
	hr = render_init_textures(rContext);
	
	// set rt/ds view on rcontext (to zero)
	// https://learn.microsoft.com/en-us/windows/win32/api/d3d11/nn-d3d11-id3d11view
    rContext->rtView = NULL;
    rContext->dsView = NULL;

	return hr;
}



#endif /* _RENDERH_ */
