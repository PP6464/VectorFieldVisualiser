#pragma once
#include <DirectXMath.h>

namespace vf::camera
{
    /// Class representing the camera
    class camera
    {
        DirectX::XMVECTOR pos_;
        DirectX::XMVECTOR target_;
        DirectX::XMVECTOR up_;

    public:
        /// Create a new camera with position, target and up direction
        camera(DirectX::XMVECTOR pos, DirectX::XMVECTOR target, DirectX::XMVECTOR up);
        /// Get the view matrix for this camera
        [[nodiscard]] DirectX::XMMATRIX get_view_matrix() const;
        /// Pan the camera
        void pan(float dx, float dy, float dz);
        /// Zoom by `scale`
        void zoom(float scale);
        /// Yaw the camera by `dx` radians
        void yaw(float dx);
        /// Pitch the camera by `dy` radians
        void pitch(float dy);
    };
}
