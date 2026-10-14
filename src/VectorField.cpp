// ReSharper disable once CppInconsistentNaming
# define _USE_MATH_DEFINES  // NOLINT(clang-diagnostic-reserved-macro-identifier, bugprone-reserved-identifier)

#include <DirectXMath.h>
#include <cmath>
#include "VectorField.h"

using namespace DirectX;

XMFLOAT3 vf::vector_field::vector_field(const XMFLOAT3 pos)
{
    const float x = pos.x;
    const float y = pos.y;
    const float z = pos.z;
    return {
        (-10 * y + std::sin(z)) / (x * x + y * y + 1),
        (10 * x + std::cos(z)) / (x * x + y * y + 1),
        -5 * std::sin((z + 0.1f * x * y + 0.25f) * static_cast<float>(M_PI))
    };
}
