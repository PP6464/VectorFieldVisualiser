#pragma once
#include <DirectXMath.h>

namespace vf::vertex
{
    /// Struct representing a vertex
    struct vertex
    {
        /// Position
        DirectX::XMFLOAT3 pos;
        /// RGBA Colour
        DirectX::XMFLOAT4 col;
    };
}
