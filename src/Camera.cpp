#include "Camera.h"

#include <algorithm>
#include <cmath>

using namespace DirectX;

vf::camera::camera::camera(const XMVECTOR pos, const XMVECTOR target, const XMVECTOR up)
{
    pos_ = pos;
    target_ = target;
    up_ = up;
}

XMMATRIX vf::camera::camera::get_view_matrix() const
{
    return XMMatrixLookAtLH(pos_, target_, up_);
}

void vf::camera::camera::pan(const float dx, const float dy, const float dz)
{
    const XMVECTOR forward = XMVector3Normalize(
        XMVectorSubtract(target_, pos_));
    const XMVECTOR right = XMVector3Normalize(
        XMVector3Cross(up_, forward));
    const XMVECTOR world_up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    const XMVECTOR delta = XMVectorAdd(
        XMVectorAdd(
            XMVectorScale(right, dx),
            XMVectorScale(world_up, dy)),
        XMVectorScale(forward, dz));
    pos_ = XMVectorAdd(pos_, delta);
    target_ = XMVectorAdd(target_, delta);
}

void vf::camera::camera::zoom(const float scale)
{
    const XMVECTOR offset = XMVectorSubtract(pos_, target_);
    const float distance = XMVectorGetX(XMVector3Length(offset));
    constexpr float min_distance = 0.5f;
    constexpr float max_distance = 80.0f;
    const float new_distance = (std::max)(min_distance, (std::min)(max_distance, distance * scale));
    pos_ = XMVectorAdd(
        target_,
        XMVectorScale(offset, new_distance / distance));
}

void vf::camera::camera::yaw(const float dx)
{
    const XMVECTOR world_up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    const XMVECTOR offset = XMVectorSubtract(pos_, target_);
    const XMVECTOR rotation = XMQuaternionRotationAxis(world_up, dx);
    pos_ = XMVectorAdd(target_, XMVector3Rotate(offset, rotation));
    up_ = world_up;
}

void vf::camera::camera::pitch(const float dy)
{
    const XMVECTOR world_up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    const XMVECTOR offset = XMVectorSubtract(pos_, target_);
    const XMVECTOR direction = XMVector3Normalize(offset);
    const XMVECTOR right = XMVector3Normalize(
        XMVector3Cross(direction, world_up));
    const float elevation = std::asin(XMVectorGetX(XMVector3Dot(direction, world_up)));
    constexpr float max_elevation = XM_PIDIV2 - 0.01f;
    float pitch_angle = dy;
    if (elevation + pitch_angle > max_elevation) pitch_angle = max_elevation - elevation;
    if (elevation + pitch_angle < -max_elevation) pitch_angle = -max_elevation - elevation;
    const XMVECTOR rotation = XMQuaternionRotationAxis(right, pitch_angle);
    pos_ = XMVectorAdd(target_, XMVector3Rotate(offset, rotation));
    up_ = world_up;
}
