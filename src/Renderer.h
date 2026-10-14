#pragma once

#include <d3d11.h>
#include <dxgi.h>
#include <wrl/client.h>

#include <DirectXMath.h>

#include "SceneGeometry.h"

namespace vf::render
{
    class renderer
    {
    public:
        void initialize(HWND window_handle, UINT width, UINT height, const scene::geometry& geometry);
        void render(DirectX::FXMMATRIX& view);

        renderer()
            : world_(DirectX::XMMatrixIdentity()),
              projection_(DirectX::XMMatrixIdentity())
        {
        }

    private:
        struct constant_buffer_data
        {
            DirectX::XMMATRIX wvp;
        };

        void create_device_and_swap_chain(HWND window_handle, UINT width, UINT height);
        void create_pipeline();
        void create_geometry_buffers(const scene::geometry& geometry);
        void create_render_targets(UINT width, UINT height);

        Microsoft::WRL::ComPtr<ID3D11Device> device_;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext> context_;
        Microsoft::WRL::ComPtr<IDXGISwapChain> swap_chain_;
        Microsoft::WRL::ComPtr<ID3D11RenderTargetView> target_view_;
        Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depth_view_;
        Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizer_state_;
        Microsoft::WRL::ComPtr<ID3D11Buffer> axes_buffer_;
        Microsoft::WRL::ComPtr<ID3D11Buffer> ticks_buffer_;
        Microsoft::WRL::ComPtr<ID3D11Buffer> field_arrows_buffer_;
        Microsoft::WRL::ComPtr<ID3D11Buffer> axis_arrows_buffer_;
        Microsoft::WRL::ComPtr<ID3D11Buffer> axis_arrow_indices_buffer_;
        Microsoft::WRL::ComPtr<ID3D11Buffer> constant_buffer_;
        Microsoft::WRL::ComPtr<ID3D11VertexShader> vertex_shader_;
        Microsoft::WRL::ComPtr<ID3D11PixelShader> pixel_shader_;
        Microsoft::WRL::ComPtr<ID3D11InputLayout> input_layout_;
        DirectX::XMMATRIX world_;
        DirectX::XMMATRIX projection_;
        UINT axes_vertex_count_ = 0;
        UINT tick_vertex_count_ = 0;
        UINT field_arrow_vertex_count_ = 0;
        UINT axis_arrow_index_count_ = 0;
    };
}
