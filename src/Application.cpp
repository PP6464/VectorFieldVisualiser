#include "Application.h"

#include <cmath>

namespace
{
    constexpr float rotation_speed = 0.1f;
    constexpr float pan_speed = 0.5f;
    constexpr float zoom_per_wheel_step = 0.9f;
    constexpr UINT window_width = 800;
    constexpr UINT window_height = 600;
}

vf::app::application::application(const HINSTANCE instance)
    : camera_(
          {0.0f, 5.0f, 25.0f, 1.0f},
          {0.0f, 0.0f, 0.0f, 1.0f},
          DirectX::XMVector3Normalize({0.0f, 25.0f, 5.0f, 0.0f})),
      geometry_(scene::create_geometry()),
      window_(instance, [this](const input_event& event) { handle_input(event); })
{
}

int vf::app::application::run()
{
    window_.create();
    renderer_.initialize(window_.handle(), window_width, window_height, geometry_);

    while (window_.process_messages())
    {
        renderer_.render(camera_.get_view_matrix());
    }
    return 0;
}

void vf::app::application::handle_input(const input_event& event)
{
    if (event.event_type == input_event::type::mouse_wheel)
    {
        camera_.zoom(std::pow(
            zoom_per_wheel_step,
            static_cast<float>(event.value) / static_cast<float>(WHEEL_DELTA)));
        return;
    }

    switch (event.value)
    {
    case VK_ESCAPE:
        window_.close();
        break;
    case VK_LEFT:
        camera_.yaw(-rotation_speed);
        break;
    case VK_RIGHT:
        camera_.yaw(rotation_speed);
        break;
    case VK_UP:
        camera_.pitch(rotation_speed);
        break;
    case VK_DOWN:
        camera_.pitch(-rotation_speed);
        break;
    case 'A':
        camera_.pan(-pan_speed, 0.0f, 0.0f);
        break;
    case 'D':
        camera_.pan(pan_speed, 0.0f, 0.0f);
        break;
    case 'W':
        camera_.pan(0.0f, pan_speed, 0.0f);
        break;
    case 'S':
        camera_.pan(0.0f, -pan_speed, 0.0f);
        break;
    default:
        break;
    }
}
