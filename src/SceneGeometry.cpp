#include "SceneGeometry.h"

#include <cmath>
#include <stdexcept>

#include <DirectXMath.h>

#include "VectorField.h"

namespace
{
    /// Returns the RGBA colour for a given magnitude
    DirectX::XMFLOAT4 color_for_magnitude(const float magnitude)
    {
        switch (static_cast<int>(std::floor(magnitude / 5.0f)))
        {
        case 0: return {0.0f, 0.0f, 1.0f, 1.0f};
        case 1: return {0.0f, 1.0f, 1.0f, 1.0f};
        case 2: return {0.0f, 1.0f, 0.0f, 1.0f};
        case 3: return {1.0f, 1.0f, 0.0f, 1.0f};
        default: return {1.0f, 0.0f, 0.0f, 1.0f};
        }
    }

    /// Adds a vector field arrow to the given list of arrows
    void append_field_arrow(std::vector<vf::vertex::vertex>& arrows, const DirectX::XMFLOAT3& start,
                            const DirectX::XMFLOAT3& end, const DirectX::XMFLOAT4& color,
                            const float arrow_length)
    {
        using namespace DirectX;

        const XMVECTOR direction = XMVectorSubtract(XMLoadFloat3(&end), XMLoadFloat3(&start));
        const XMVECTOR start_vector = XMLoadFloat3(&start);
        const XMVECTOR head_base = XMVectorAdd(
            start_vector, XMVectorScale(direction, arrow_length * 0.75f));
        const XMVECTOR perpendicular = XMVectorGetX(XMVector3Dot(direction, XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f))) == 0.0f
            ? XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f)
            : XMVectorScale(
                XMVector3Normalize(XMVectorSet(
                    XMVectorGetY(direction), -XMVectorGetX(direction), 0.0f, 0.0f)),
                arrow_length * 0.25f);

        XMFLOAT3 tip1;
        XMFLOAT3 tip2;
        XMStoreFloat3(&tip1, XMVectorAdd(head_base, perpendicular));
        XMStoreFloat3(&tip2, XMVectorSubtract(head_base, perpendicular));

        arrows.emplace_back(vf::vertex::vertex{start, color});
        arrows.emplace_back(vf::vertex::vertex{end, color});
        arrows.emplace_back(vf::vertex::vertex{tip1, color});
        arrows.emplace_back(vf::vertex::vertex{end, color});
        arrows.emplace_back(vf::vertex::vertex{tip2, color});
        arrows.emplace_back(vf::vertex::vertex{end, color});
    }
}

vf::scene::geometry vf::scene::create_geometry(const int axes_size, const int stride, const float arrow_length)
{
    if (stride <= 0)
    {
        throw std::invalid_argument("Scene geometry stride must be positive.");
    }
    
    if (axes_size < 1)
    {
        throw std::invalid_argument("Scene geometry axes size must be positive.");
    }

    geometry result;
    const float axis_extent = static_cast<float>(axes_size);
    result.axes = {
        // X-axis
        {{-axis_extent, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
        {{axis_extent + 0.5f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
        // Y-axis
        {{0.0f, -axis_extent, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
        {{0.0f, axis_extent + 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
        // Z-axis
        {{0.0f, 0.0f, -axis_extent}, {0.0f, 0.0f, 1.0f, 1.0f}},
        {{0.0f, 0.0f, axis_extent + 0.5f}, {0.0f, 0.0f, 1.0f, 1.0f}},
    };

    for (int i = -axes_size; i <= axes_size; i += stride)
    {
        const float tick = static_cast<float>(i);
        // X-axis tick
        result.ticks.emplace_back(vertex::vertex{{tick, -0.25f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}});
        result.ticks.emplace_back(vertex::vertex{{tick, 0.25f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}});
        // Y-axis tick
        result.ticks.emplace_back(vertex::vertex{{-0.25f, tick, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}});
        result.ticks.emplace_back(vertex::vertex{{0.25f, tick, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}});
        // Z-axis tick
        result.ticks.emplace_back(vertex::vertex{{-0.25f, 0.0f, tick}, {0.0f, 0.0f, 1.0f, 1.0f}});
        result.ticks.emplace_back(vertex::vertex{{0.25f, 0.0f, tick}, {0.0f, 0.0f, 1.0f, 1.0f}});
    }

    result.axis_arrows = {
        // X-axis arrow
        {{axis_extent + 0.5f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
        {{axis_extent + 0.25f, -0.25f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
        {{axis_extent + 0.25f, 0.25f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
        // Y-axis arrow
        {{0.0f, axis_extent + 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
        {{-0.25f, axis_extent + 0.25f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
        {{0.25f, axis_extent + 0.25f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
        // Z-axis arrow
        {{0.0f, 0.0f, axis_extent + 0.5f}, {0.0f, 0.0f, 1.0f, 1.0f}},
        {{-0.25f, 0.0f, axis_extent + 0.25f}, {0.0f, 0.0f, 1.0f, 1.0f}},
        {{0.25f, 0.0f, axis_extent + 0.25f}, {0.0f, 0.0f, 1.0f, 1.0f}},
    };
    result.axis_arrow_indices = {0, 2, 1, 3, 4, 5, 6, 7, 8};

    for (int x = -axes_size; x <= axes_size; x += stride)
    {
        for (int y = -axes_size; y <= axes_size; y += stride)
        {
            for (int z = -axes_size; z <= axes_size; z += stride)
            {
                // Calculate the vector field at (x, y, z)
                const DirectX::XMFLOAT3 start{
                    static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)
                };
                const DirectX::XMFLOAT3 field = vector_field::vector_field(start);
                const DirectX::XMVECTOR field_vector = DirectX::XMLoadFloat3(&field);
                const float magnitude_squared = DirectX::XMVectorGetX(
                    DirectX::XMVector3LengthSq(field_vector));
                DirectX::XMFLOAT3 end;
                DirectX::XMStoreFloat3(&end, DirectX::XMVectorAdd(
                    DirectX::XMLoadFloat3(&start),
                    DirectX::XMVectorScale(DirectX::XMVector3Normalize(field_vector), arrow_length)));

                append_field_arrow(result.field_arrows, start, end,
                                   color_for_magnitude(magnitude_squared), arrow_length);
            }
        }
    }

    return result;
}
