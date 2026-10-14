#include "Renderer.h"

#include <d3dcompiler.h>

#include <iomanip>
#include <limits>
#include <sstream>
#include <stdexcept>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

using namespace DirectX;

namespace
{
    void check_hresult(const HRESULT result, const char* operation)
    {
        if (FAILED(result))
        {
            std::ostringstream message;
            message << operation << " failed with HRESULT 0x"
                << std::hex << std::uppercase << static_cast<unsigned long>(result);
            throw std::runtime_error(message.str());
        }
    }

    void create_buffer(
        ID3D11Device* device,
        const void* data,
        const std::size_t byte_size,
        const UINT bind_flags,
        ID3D11Buffer** buffer)
    {
        if (byte_size == 0 || byte_size > (std::numeric_limits<UINT>::max)())
        {
            throw std::runtime_error("Cannot create a Direct3D buffer with the requested size.");
        }

        D3D11_BUFFER_DESC description{};
        description.Usage = D3D11_USAGE_DEFAULT;
        description.ByteWidth = static_cast<UINT>(byte_size);
        description.BindFlags = bind_flags;

        D3D11_SUBRESOURCE_DATA initial_data{};
        initial_data.pSysMem = data;
        check_hresult(device->CreateBuffer(&description, &initial_data, buffer), "CreateBuffer");
    }
}

void vf::render::renderer::initialize(
    const HWND window_handle, const UINT width, const UINT height, const scene::geometry& geometry)
{
    create_device_and_swap_chain(window_handle, width, height);
    create_pipeline();
    create_geometry_buffers(geometry);
    create_render_targets(width, height);

    world_ = XMMatrixRotationY(XMConvertToRadians(-150.0f))
        * XMMatrixRotationX(XMConvertToRadians(15.0f));
    const XMMATRIX switch_y_z = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    world_ = switch_y_z * world_;
    projection_ = XMMatrixPerspectiveFovLH(
        XM_PIDIV4,
        static_cast<float>(width) / static_cast<float>(height),
        0.1f,
        100.0f);
}

void vf::render::renderer::create_device_and_swap_chain(
    const HWND window_handle, const UINT width, const UINT height)
{
    DXGI_SWAP_CHAIN_DESC swap_chain_description{};
    swap_chain_description.BufferCount = 1;
    swap_chain_description.BufferDesc.Width = width;
    swap_chain_description.BufferDesc.Height = height;
    swap_chain_description.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swap_chain_description.BufferDesc.RefreshRate.Numerator = 60;
    swap_chain_description.BufferDesc.RefreshRate.Denominator = 1;
    swap_chain_description.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swap_chain_description.OutputWindow = window_handle;
    swap_chain_description.SampleDesc.Count = 1;
    swap_chain_description.Windowed = TRUE;
    swap_chain_description.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    D3D_FEATURE_LEVEL feature_level{}; // NOLINT(bugprone-invalid-enum-default-initialization)
    check_hresult(D3D11CreateDeviceAndSwapChain(
                      nullptr,
                      D3D_DRIVER_TYPE_HARDWARE,
                      nullptr,
                      D3D11_CREATE_DEVICE_SINGLETHREADED,
                      nullptr,
                      0,
                      D3D11_SDK_VERSION,
                      &swap_chain_description,
                      swap_chain_.GetAddressOf(),
                      device_.GetAddressOf(),
                      &feature_level,
                      context_.GetAddressOf()), "D3D11CreateDeviceAndSwapChain");

    D3D11_RASTERIZER_DESC rasterizer_description{}; // NOLINT(bugprone-invalid-enum-default-initialization)
    rasterizer_description.CullMode = D3D11_CULL_NONE;
    rasterizer_description.FillMode = D3D11_FILL_SOLID;
    rasterizer_description.FrontCounterClockwise = TRUE;
    rasterizer_description.DepthClipEnable = TRUE;
    check_hresult(device_->CreateRasterizerState(
                      &rasterizer_description, rasterizer_state_.GetAddressOf()), "CreateRasterizerState");
    context_->RSSetState(rasterizer_state_.Get());
}

void vf::render::renderer::create_pipeline()
{
    Microsoft::WRL::ComPtr<ID3DBlob> vertex_shader_blob;
    Microsoft::WRL::ComPtr<ID3DBlob> pixel_shader_blob;
    check_hresult(D3DReadFileToBlob(
                      L"Shaders/Compiled/VertexShader.cso", vertex_shader_blob.GetAddressOf()), "Read vertex shader");
    check_hresult(D3DReadFileToBlob(
                      L"Shaders/Compiled/PixelShader.cso", pixel_shader_blob.GetAddressOf()), "Read pixel shader");
    check_hresult(device_->CreateVertexShader(
                      vertex_shader_blob->GetBufferPointer(),
                      vertex_shader_blob->GetBufferSize(),
                      nullptr,
                      vertex_shader_.GetAddressOf()), "CreateVertexShader");
    check_hresult(device_->CreatePixelShader(
                      pixel_shader_blob->GetBufferPointer(),
                      pixel_shader_blob->GetBufferSize(),
                      nullptr,
                      pixel_shader_.GetAddressOf()), "CreatePixelShader");

    constexpr D3D11_INPUT_ELEMENT_DESC layout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };
    check_hresult(device_->CreateInputLayout(
                      layout,
                      ARRAYSIZE(layout),
                      vertex_shader_blob->GetBufferPointer(),
                      vertex_shader_blob->GetBufferSize(),
                      input_layout_.GetAddressOf()), "CreateInputLayout");

    D3D11_BUFFER_DESC constant_buffer_description{};
    constant_buffer_description.Usage = D3D11_USAGE_DEFAULT;
    constant_buffer_description.ByteWidth = sizeof(constant_buffer_data);
    constant_buffer_description.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    check_hresult(device_->CreateBuffer(
                      &constant_buffer_description, nullptr, constant_buffer_.GetAddressOf()), "CreateBuffer");
}

void vf::render::renderer::create_geometry_buffers(const scene::geometry& geometry)
{
    create_buffer(device_.Get(), geometry.axes.data(), geometry.axes.size() * sizeof(vertex::vertex),
                  D3D11_BIND_VERTEX_BUFFER, axes_buffer_.GetAddressOf());
    create_buffer(device_.Get(), geometry.ticks.data(), geometry.ticks.size() * sizeof(vertex::vertex),
                  D3D11_BIND_VERTEX_BUFFER, ticks_buffer_.GetAddressOf());
    create_buffer(device_.Get(), geometry.field_arrows.data(),
                  geometry.field_arrows.size() * sizeof(vertex::vertex),
                  D3D11_BIND_VERTEX_BUFFER, field_arrows_buffer_.GetAddressOf());
    create_buffer(device_.Get(), geometry.axis_arrows.data(),
                  geometry.axis_arrows.size() * sizeof(vertex::vertex),
                  D3D11_BIND_VERTEX_BUFFER, axis_arrows_buffer_.GetAddressOf());
    create_buffer(device_.Get(), geometry.axis_arrow_indices.data(),
                  geometry.axis_arrow_indices.size() * sizeof(std::uint32_t),
                  D3D11_BIND_INDEX_BUFFER, axis_arrow_indices_buffer_.GetAddressOf());

    axes_vertex_count_ = static_cast<UINT>(geometry.axes.size());
    tick_vertex_count_ = static_cast<UINT>(geometry.ticks.size());
    field_arrow_vertex_count_ = static_cast<UINT>(geometry.field_arrows.size());
    axis_arrow_index_count_ = static_cast<UINT>(geometry.axis_arrow_indices.size());
}

void vf::render::renderer::create_render_targets(const UINT width, const UINT height)
{
    Microsoft::WRL::ComPtr<ID3D11Texture2D> back_buffer;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> depth_buffer;

    check_hresult(swap_chain_->GetBuffer(
                      0, IID_PPV_ARGS(back_buffer.GetAddressOf())), "GetBuffer");

    D3D11_TEXTURE2D_DESC depth_description{};
    depth_description.Width = width;
    depth_description.Height = height;
    depth_description.MipLevels = 1;
    depth_description.ArraySize = 1;
    depth_description.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depth_description.SampleDesc.Count = 1;
    depth_description.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    check_hresult(device_->CreateTexture2D(
                      &depth_description, nullptr, depth_buffer.GetAddressOf()), "CreateTexture2D");
    check_hresult(device_->CreateRenderTargetView(
                      back_buffer.Get(), nullptr, target_view_.GetAddressOf()), "CreateRenderTargetView");
    check_hresult(device_->CreateDepthStencilView(
                      depth_buffer.Get(), nullptr, depth_view_.GetAddressOf()), "CreateDepthStencilView");

    context_->OMSetRenderTargets(1, target_view_.GetAddressOf(), depth_view_.Get());

    D3D11_VIEWPORT viewport{};
    viewport.Width = static_cast<float>(width);
    viewport.Height = static_cast<float>(height);
    viewport.MaxDepth = 1.0f;
    context_->RSSetViewports(1, &viewport);
}

void vf::render::renderer::render(const FXMMATRIX& view)
{
    constexpr float background[] = {0.0f, 0.0f, 0.0f, 1.0f};
    context_->ClearRenderTargetView(target_view_.Get(), background);
    context_->ClearDepthStencilView(
        depth_view_.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    context_->IASetInputLayout(input_layout_.Get());

    const constant_buffer_data updated_constants{world_ * view * projection_};
    context_->UpdateSubresource(constant_buffer_.Get(), 0, nullptr, &updated_constants, 0, 0);
    context_->VSSetShader(vertex_shader_.Get(), nullptr, 0);
    context_->PSSetShader(pixel_shader_.Get(), nullptr, 0);
    context_->VSSetConstantBuffers(0, 1, constant_buffer_.GetAddressOf());

    constexpr UINT stride = sizeof(vertex::vertex);
    constexpr UINT offset = 0;
    context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
    context_->IASetVertexBuffers(0, 1, axes_buffer_.GetAddressOf(), &stride, &offset);
    context_->Draw(axes_vertex_count_, 0);
    context_->IASetVertexBuffers(0, 1, ticks_buffer_.GetAddressOf(), &stride, &offset);
    context_->Draw(tick_vertex_count_, 0);
    context_->IASetVertexBuffers(0, 1, field_arrows_buffer_.GetAddressOf(), &stride, &offset);
    context_->Draw(field_arrow_vertex_count_, 0);

    context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context_->IASetVertexBuffers(0, 1, axis_arrows_buffer_.GetAddressOf(), &stride, &offset);
    context_->IASetIndexBuffer(axis_arrow_indices_buffer_.Get(), DXGI_FORMAT_R32_UINT, 0);
    context_->DrawIndexed(axis_arrow_index_count_, 0, 0);
    check_hresult(swap_chain_->Present(1, 0), "Present");
}
