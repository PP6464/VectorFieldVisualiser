// ReSharper disable CppClangTidyClangDiagnosticLanguageExtensionToken
// ReSharper disable CppFunctionResultShouldBeUsed
// ReSharper disable CppClangTidyConcurrencyMtUnsafe
// ReSharper disable CppDeclaratorNeverUsed
#include <windows.h>
#include <windowsx.h>
// DirectX 11
#include <dxgi1_6.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

#include <iostream>
#include <stdexcept>
#include <vector>

#include "resource.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

namespace
{
	// Global variables
	HWND window_handle;
	UINT width = 800;
	UINT height = 600;
	int axes_size = 3;

	float rotation_speed = 1.0f;

	// Rendering variables
	struct vertex
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT4 col;
	};

	struct camera
	{
		DirectX::XMVECTOR pos;
		DirectX::XMVECTOR target;
		DirectX::XMVECTOR up;

		DirectX::XMMATRIX get_view_matrix() const
		{
			return DirectX::XMMatrixLookAtLH(pos, target, up);
		}

		void yaw(const float dx)
		{
			const DirectX::XMVECTOR dist = DirectX::XMVector3Length(DirectX::XMVectorSubtract(target, pos));
			const DirectX::XMVECTOR right = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(up, DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(target, pos))));
			pos = DirectX::XMVectorScale(DirectX::XMVector3Normalize(DirectX::XMVectorAdd(pos, DirectX::XMVectorScale(right, dx))), dist.m128_f32[0]);
			const DirectX::XMVECTOR direction = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(target, pos));
			up = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(
				DirectX::XMVectorScale(DirectX::XMVectorSubtract(pos, target), DirectX::XMVectorSubtract(pos, target).m128_f32[0]),
				{ 0.0, DirectX::XMVector3Length(DirectX::XMVectorSubtract(pos, target)).m128_f32[0], 0.0 }
			));
		}
	};

	camera cam
	{
		{ 0.0, 1.0, 10.0, 1.0 },
		{ 0.0, 0.0, 0.0, 1.0 },
		DirectX::XMVector3Normalize({ 0.0, 10.0, 1.0, 0.0 }),
	};

	DirectX::XMMATRIX init_world_orientation = DirectX::XMMatrixRotationY(DirectX::XMConvertToRadians(-150)) * DirectX::XMMatrixRotationX(DirectX::XMConvertToRadians(15));
	DirectX::XMMATRIX switch_y_z = {
		1.0, 0.0, 0.0, 0.0,
		0.0, 0.0, 1.0, 0.0,
		0.0, 1.0, 0.0, 0.0,
		0.0, 0.0, 0.0, 1.0
	};
	DirectX::XMMATRIX init_view = DirectX::XMMatrixLookAtLH(
		{ 0.0, 1.0, 10.0,  1.0 },
		{ 0.0, 0.0, 0.0, 1.0 },
		DirectX::XMVector3Normalize({ 0.0, 10.0, 1.0, 0.0 })
	);
	DirectX::XMMATRIX init_world = switch_y_z * init_world_orientation;
	DirectX::XMMATRIX init_projection = DirectX::XMMatrixPerspectiveFovLH(
		DirectX::XM_PIDIV4,
		static_cast<float>(width) / static_cast<float>(height),
		0.1f, 100.0f
	);

	DirectX::XMMATRIX wvp = init_world * cam.get_view_matrix() * init_projection;

	struct cb_data
	{
		DirectX::XMMATRIX wvp;
	};

	cb_data cb = {wvp};

	// Axes
	std::vector<vertex> axes = {
		// X-axis (red)
		{{-static_cast<float>(axes_size), 0.0f, 0.0f}, {1, 0, 0, 1}},
		{{0.5f + static_cast<float>(axes_size), 0.0f, 0.0f}, {1, 0, 0, 1}},

		// Y-axis (green)
		{{0.0f, -static_cast<float>(axes_size), 0.0f}, {0, 1, 0, 1}},
		{{0.0f, 0.5f + static_cast<float>(axes_size), 0.0f}, {0, 1, 0, 1}},

		// Z-axis (blue)
		{{0.0f, 0.0f, -static_cast<float>(axes_size)}, {0, 0, 1, 1}},
		{{0.0f, 0.0f, 0.5f + static_cast<float>(axes_size)}, {0, 0, 1, 1}},
	};
	std::vector<vertex> axes_ticks = {};
	std::vector<vertex> axes_arrows = {
		// x-axis
		{ { static_cast<float>(axes_size) + 0.5f, 0.0, 0.0 }, { 1.0, 0.0, 0.0, 1.0 }},
		{ { static_cast<float>(axes_size) + 0.25f, -0.25, 0.0 }, { 1.0, 0.0, 0.0, 1.0 }},
		{ { static_cast<float>(axes_size) + 0.25f, 0.25, 0.0 }, { 1.0, 0.0, 0.0, 1.0 }},
		// y-axis
		{ { 0.0, static_cast<float>(axes_size) + 0.5f, 0.0 }, { 0.0, 1.0, 0.0, 1.0 }},
		{ { -0.25, static_cast<float>(axes_size) + 0.25f, 0.0 }, { 0.0, 1.0, 0.0, 1.0 }},
		{ { 0.25, static_cast<float>(axes_size) + 0.25f, 0.0 }, { 0.0, 1.0, 0.0, 1.0 }},
		// z-axis
		{ { 0.0, 0.0, static_cast<float>(axes_size) + 0.5f }, { 0.0, 0.0, 1.0, 1.0 }},
		{ { -0.25, 0.0, static_cast<float>(axes_size) + 0.25f }, { 0.0, 0.0, 1.0, 1.0 }},
		{ { 0.25, 0.0, static_cast<float>(axes_size) + 0.25f }, { 0.0, 0.0, 1.0, 1.0 }},
	};
	std::vector<UINT> axes_arrows_indices = {
		// x-axis
		0, 2, 1,
		// y-axis
		3, 4, 5,
		// z-axis
		6, 7, 8,
	};

	// Buffers
	ID3D11Buffer* axes_vertex_buffer; // A buffer for axes vertex data
	ID3D11Buffer* axes_ticks_buffer; // A buffer for axes ticks data
	ID3D11Buffer* axes_arrows_buffer; // A buffer for axes arrows data
	ID3D11Buffer* axes_arrows_index_buffer; // A buffer for axes arrows index data
	ID3D11Buffer* cb_buffer; // A buffer for constant data

	ID3D11VertexShader* vertex_shader; // A vertex shader
	ID3D11PixelShader* pixel_shader; // A pixel shader
	ID3D11InputLayout* input_layout; // A layout for vertex data

	ID3D11Device* device; // An interface to communicate with device GPU
	ID3D11DeviceContext* context; // The context surrounding rendering to the device's screen
	IDXGISwapChain* swap_chain; // A swap chain determines how to change frames
	ID3D11RenderTargetView* target_view; // What is rendered on
	ID3D11DepthStencilView* depth_view; // Manages the depth and visibility of objects

	ID3D11RasterizerState* rasterizer_state = nullptr;

	// Window Procedure: handles events/messages like close, resize, etc.
	LRESULT CALLBACK window_proc(const HWND h_wnd, const UINT u_msg, const WPARAM w_param, const LPARAM l_param)
	{
		switch (u_msg)
		{
		case WM_DESTROY:
			PostQuitMessage(0); // Tells the app to quit
			return 0;
		case WM_KEYDOWN:
			switch (w_param)
			{
				case VK_ESCAPE:
					PostQuitMessage(0);
					return 0;
				case VK_LEFT:
					std::cout << "Left" << std::endl ;
					cam.yaw(-0.01f);
					return 0;
				case VK_RIGHT:
					std::cout << "Right" << std::endl;
					cam.yaw(0.01f);
					return 0;
				default: break;
			}
		default: break;
		}

		return DefWindowProc(h_wnd, u_msg, w_param, l_param);
	}

	// Initialise the window
	void init_window(const HINSTANCE h_instance)
	{
		// 1. Register the window class
		const auto class_name = L"VectorFieldVisualiser";
		const auto title = L"Vector Field Visualiser";

		WNDCLASSEX wc = {};
		wc.cbSize = sizeof(WNDCLASSEX);
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = window_proc; // callback for messages
		wc.hInstance = h_instance;
		wc.hIcon = LoadIcon(h_instance, MAKEINTRESOURCE(IDI_VECTORFIELDVISUALISER));
		wc.hIconSm = LoadIcon(h_instance, MAKEINTRESOURCE(IDI_SMALL));
		wc.lpszClassName = class_name;
		wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW);
		wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

		RegisterClassEx(&wc);

		// 2. Create the window
		window_handle = CreateWindowEx(
			0, // Optional window styles
			class_name, // Window class
			title, // Window text (title)
			WS_OVERLAPPEDWINDOW, // Window style

			// Size and position
			CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,

			nullptr, // Parent window
			nullptr, // Menu
			h_instance, // Instance handle
			nullptr // Additional application data
		);

		if (window_handle == nullptr)
		{
			throw std::runtime_error("Failed to create a window.");
		}

		ShowWindow(window_handle, SW_SHOW);
	}

	// ReSharper disable once CppInconsistentNaming
	void init_D3D()
	{
		// Swap chain configuration
		DXGI_SWAP_CHAIN_DESC scd = {};
		scd.BufferCount = 1;
		scd.BufferDesc.Width = width;
		scd.BufferDesc.Height = height;
		scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		scd.BufferDesc.RefreshRate.Numerator = 60;
		scd.BufferDesc.RefreshRate.Denominator = 1;
		scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		scd.OutputWindow = window_handle;
		scd.SampleDesc.Count = 1;
		scd.SampleDesc.Quality = 0;
		scd.Windowed = true;
		scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

		// Depth view configuration
		CD3D11_TEXTURE2D_DESC dvd = {};
		dvd.Width = width;
		dvd.Height = height;
		dvd.MipLevels = 1;
		dvd.ArraySize = 1;
		dvd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		dvd.SampleDesc.Count = 1;
		dvd.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		
		// Axes vertex buffer configuration
		D3D11_BUFFER_DESC axes_vertex_buffer_desc = {};
		axes_vertex_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
		axes_vertex_buffer_desc.ByteWidth = sizeof(vertex) * axes.size();
		axes_vertex_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		axes_vertex_buffer_desc.CPUAccessFlags = 0;
		D3D11_SUBRESOURCE_DATA init_axes_vertex_data = {};
		init_axes_vertex_data.pSysMem = axes.data();

		// Axes ticks buffer configuration
		for (int i = -axes_size; i <= axes_size; ++i)
		{
			// x-axis tick
			axes_ticks.emplace_back(vertex {{static_cast<float>(i), -0.25, 0.0}, { 1.0, 0.0, 0.0, 1.0 }});
			axes_ticks.emplace_back(vertex {{static_cast<float>(i), 0.25, 0.0}, { 1.0, 0.0, 0.0, 1.0 }});
			// y-axis tick
			axes_ticks.emplace_back(vertex {{-0.25, static_cast<float>(i), 0.0}, { 0.0, 1.0, 0.0, 1.0 }});
			axes_ticks.emplace_back(vertex {{0.25, static_cast<float>(i), 0.0}, { 0.0, 1.0, 0.0, 1.0 }});
			// z-axis tick
			axes_ticks.emplace_back(vertex {{-0.25, 0.0, static_cast<float>(i)}, { 0.0, 0.0, 1.0, 1.0 }});
			axes_ticks.emplace_back(vertex {{0.25, 0.0, static_cast<float>(i)}, { 0.0, 0.0, 1.0, 1.0 }});
		}
		D3D11_BUFFER_DESC axes_ticks_buffer_desc = {};
		axes_ticks_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
		axes_ticks_buffer_desc.ByteWidth = sizeof(vertex) * axes_ticks.size();
		axes_ticks_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		axes_ticks_buffer_desc.CPUAccessFlags = 0;
		D3D11_SUBRESOURCE_DATA init_axes_ticks_data = {};
		init_axes_ticks_data.pSysMem = axes_ticks.data();

		// Axes arrows buffer configuration
		D3D11_BUFFER_DESC axes_arrows_buffer_desc = {};
		axes_arrows_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
		axes_arrows_buffer_desc.ByteWidth = sizeof(vertex) * axes_arrows.size();
		axes_arrows_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		axes_arrows_buffer_desc.CPUAccessFlags = 0;
		D3D11_SUBRESOURCE_DATA init_axes_arrows_data = {};
		init_axes_arrows_data.pSysMem = axes_arrows.data();

		// Axes arrows index buffer configuration
		D3D11_BUFFER_DESC axes_arrows_index_buffer_desc = {};
		axes_arrows_index_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
		axes_arrows_index_buffer_desc.ByteWidth = sizeof(UINT) * axes_arrows_indices.size();
		axes_arrows_index_buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		axes_arrows_index_buffer_desc.CPUAccessFlags = 0;
		D3D11_SUBRESOURCE_DATA init_axes_arrows_index_data = {};
		init_axes_arrows_index_data.pSysMem = axes_arrows_indices.data();

		// Constant buffer configuration
		D3D11_BUFFER_DESC constant_buffer_desc = {};
		constant_buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
		constant_buffer_desc.ByteWidth = sizeof(cb_data);
		constant_buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		constant_buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		constant_buffer_desc.MiscFlags = 0;
		constant_buffer_desc.StructureByteStride = 0;
		D3D11_SUBRESOURCE_DATA init_cb_data = {};
		init_cb_data.pSysMem = &cb;
		init_cb_data.SysMemPitch = 0;
		init_cb_data.SysMemSlicePitch = 0;

		// TEMPORARY: Disable culling
		D3D11_RASTERIZER_DESC rasterizer_desc = {};
		rasterizer_desc.CullMode = D3D11_CULL_NONE;
		rasterizer_desc.FillMode = D3D11_FILL_SOLID;
		rasterizer_desc.FrontCounterClockwise = true;
		rasterizer_desc.DepthBias = 0;
		rasterizer_desc.DepthBiasClamp = 0.0f;
		rasterizer_desc.SlopeScaledDepthBias = 0.0f;
		rasterizer_desc.DepthClipEnable = true;
		rasterizer_desc.ScissorEnable = false;
		rasterizer_desc.MultisampleEnable = false;
		rasterizer_desc.AntialiasedLineEnable = false;

		D3D_FEATURE_LEVEL feature_level;

		const HRESULT hr_device_swap_chain = D3D11CreateDeviceAndSwapChain(
			nullptr,
			D3D_DRIVER_TYPE_HARDWARE,
			nullptr,
			D3D11_CREATE_DEVICE_SINGLETHREADED,
			nullptr,
			0,
			D3D11_SDK_VERSION,
			&scd,
			&swap_chain,
			&device,
			&feature_level,
			&context
		);

		if (FAILED(hr_device_swap_chain))
		{
			char msg[128];
			sprintf_s(msg, "D3D11CreateDeviceAndSwapChain failed with HRESULT: 0x%08X", hr_device_swap_chain);
			std::cout << msg;
			MessageBoxA(nullptr, msg, "Error", MB_OK | MB_ICONERROR);
			exit(1);
		}

		device->CreateRasterizerState(&rasterizer_desc, &rasterizer_state);
		
		// Create the axes vertex buffer
		device->CreateBuffer(&axes_vertex_buffer_desc, &init_axes_vertex_data, &axes_vertex_buffer);
		// Create the axes ticks buffer
		device->CreateBuffer(&axes_ticks_buffer_desc, &init_axes_ticks_data, &axes_ticks_buffer);
		// Create the axes' arrows buffer
		device->CreateBuffer(&axes_arrows_buffer_desc, &init_axes_arrows_data, &axes_arrows_buffer);
		// Create the axes' arrows index buffer
		device->CreateBuffer(&axes_arrows_index_buffer_desc, &init_axes_arrows_index_data, &axes_arrows_index_buffer);
		// Create the constant buffer
		device->CreateBuffer(&constant_buffer_desc, &init_cb_data, &cb_buffer);

		// Load compiled shaders
		ID3DBlob* vertex_shader_blob = nullptr;
		ID3DBlob* pixel_shader_blob = nullptr;

		const HRESULT hr_vertex_shader = D3DReadFileToBlob(L"Shaders/Compiled/VertexShader.cso", &vertex_shader_blob);

		if (FAILED(hr_vertex_shader))
		{
			char msg[128];
			sprintf_s(msg, "D3DCompileFromFile failed with HRESULT: 0x%08X", hr_vertex_shader);
			std::cout << msg;
			MessageBoxA(nullptr, msg, "Error", MB_OK | MB_ICONERROR);
			exit(1);
		}

		device->CreateVertexShader(
			vertex_shader_blob->GetBufferPointer(),
			vertex_shader_blob->GetBufferSize(),
			nullptr,
			&vertex_shader
		);

		const HRESULT hr_pixel_shader = D3DReadFileToBlob(L"Shaders/Compiled/PixelShader.cso", &pixel_shader_blob);

		if (FAILED(hr_pixel_shader))
		{
			char msg[128];
			sprintf_s(msg, "D3DCompileFromFile failed with HRESULT: 0x%08X", hr_pixel_shader);
			std::cout << msg;
			MessageBoxA(nullptr, msg, "Error", MB_OK | MB_ICONERROR);
			exit(1);
		}

		device->CreatePixelShader(
			pixel_shader_blob->GetBufferPointer(),
			pixel_shader_blob->GetBufferSize(),
			nullptr,
			&pixel_shader
		);

		// Create the input layout
		D3D11_INPUT_ELEMENT_DESC layout[] = {
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12 /*D3D11_APPEND_ALIGNED_ELEMENT*/, D3D11_INPUT_PER_VERTEX_DATA, 0},
		};
		UINT num_elements = ARRAYSIZE(layout);
		device->CreateInputLayout(
			layout,
			num_elements,
			vertex_shader_blob->GetBufferPointer(),
			vertex_shader_blob->GetBufferSize(),
			&input_layout
		);

		ID3D11Texture2D* back_buffer = nullptr; // The back buffer of the swap chain
		ID3D11Texture2D* depth_buffer = nullptr; // The depth buffer of the swap chain

		swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&back_buffer));
		device->CreateTexture2D(&dvd, nullptr, &depth_buffer);
		device->CreateRenderTargetView(back_buffer, nullptr, &target_view);
		device->CreateDepthStencilView(depth_buffer, nullptr, &depth_view);
		back_buffer->Release();
		depth_buffer->Release();

		context->OMSetRenderTargets(1, &target_view, depth_view);

		// Viewport configuration
		D3D11_VIEWPORT viewport = {};
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = static_cast<float>(width);
		viewport.Height = static_cast<float>(height);
		context->RSSetViewports(1, &viewport);
		context->RSSetState(rasterizer_state);
	}

	void render()
	{
		// Clear the screen for rendering
		constexpr float background[4] = {0.0f, 0.0f, 0.0f, 1.0f}; // RGBA from 0 to 1
		context->ClearRenderTargetView(target_view, background);
		context->ClearDepthStencilView(depth_view, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		context->IASetInputLayout(input_layout);

		// "Recalculate"	the WVP matrix - currently no need to change it
		const DirectX::XMMATRIX world = init_world;
		const DirectX::XMMATRIX view = cam.get_view_matrix();
		const DirectX::XMMATRIX projection = init_projection;
		const DirectX::XMMATRIX wvp = world * view * projection;
		const cb_data new_cb = {DirectX::XMMatrixTranspose(wvp)}; // Transpose the matrix for the shader
		context->UpdateSubresource(cb_buffer, 0, nullptr, &new_cb, 0, 0);

		// Configure shaders for rendering
		context->VSSetShader(vertex_shader, nullptr, 0);
		context->PSSetShader(pixel_shader, nullptr, 0);
		context->VSSetConstantBuffers(0, 1, &cb_buffer);

		// The following are useful for vertices in general
		constexpr UINT stride = sizeof(vertex);
		constexpr UINT offset = 0;

		// Render the axes
		context->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_LINELIST);
		context->IASetVertexBuffers(0, 1, &axes_vertex_buffer, &stride, &offset);
		context->Draw(axes.size(), 0);

		// Render the axes ticks
		context->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_LINELIST);
		context->IASetVertexBuffers(0, 1, &axes_ticks_buffer, &stride, &offset);
		context->Draw(axes_ticks.size(), 0);

		// Render the axes' arrows
		context->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		context->IASetVertexBuffers(0, 1, &axes_arrows_buffer, &stride, &offset);
		context->IASetIndexBuffer(axes_arrows_index_buffer, DXGI_FORMAT_R32_UINT, 0);
		context->DrawIndexed(axes_arrows_indices.size(), 0, 0);
		
		swap_chain->Present(1, 0);
	}

	void clean_up()
	{
		if (target_view != nullptr) target_view->Release();
		if (depth_view != nullptr) depth_view->Release();
		if (swap_chain != nullptr) swap_chain->Release();
		if (context != nullptr) context->Release();
		if (device != nullptr) device->Release();
		if (vertex_shader != nullptr) vertex_shader->Release();
		if (pixel_shader != nullptr) pixel_shader->Release();
		if (input_layout != nullptr) input_layout->Release();
		if (cb_buffer != nullptr) cb_buffer->Release();
		if (axes_ticks_buffer != nullptr) axes_ticks_buffer->Release();
		if (rasterizer_state != nullptr) rasterizer_state->Release();
		if (axes_arrows_buffer != nullptr) axes_arrows_buffer->Release();
		if (axes_vertex_buffer != nullptr) axes_vertex_buffer->Release();
		if (window_handle != nullptr) DestroyWindow(window_handle);
	}
}

// WinMain: the entry point for a Windows desktop application
// ReSharper disable once CppInconsistentNaming
int WINAPI WinMain(const HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
	init_window(hInstance);
	init_D3D();

	MSG msg = {};
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			render();
		}
	}

	clean_up();
	return static_cast<int>(msg.wParam);
}
