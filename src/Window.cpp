#include "Window.h"

#include <stdexcept>
#include <utility>

#include "../Resource.h"

namespace
{
    constexpr wchar_t class_name[] = L"VectorFieldVisualiser";
    constexpr wchar_t window_title[] = L"Vector Field Visualiser";
}

vf::app::window::window(const HINSTANCE instance, input_handler on_input)
    : instance_(instance),
      on_input_(std::move(on_input))
{
}

vf::app::window::~window()
{
    if (handle_ != nullptr)
    {
        DestroyWindow(handle_);
    }
}

void vf::app::window::create()
{
    WNDCLASSEXW window_class{};
    window_class.cbSize = sizeof(window_class);
    window_class.style = CS_HREDRAW | CS_VREDRAW;
    window_class.lpfnWndProc = window_proc;
    window_class.hInstance = instance_;
    window_class.hIcon = LoadIconW(instance_, MAKEINTRESOURCEW(IDI_VECTORFIELDVISUALISER));
    window_class.hIconSm = LoadIconW(instance_, MAKEINTRESOURCEW(IDI_SMALL));
    window_class.lpszClassName = class_name;
    window_class.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);  // NOLINT(performance-no-int-to-ptr)
    window_class.hCursor = LoadCursorW(nullptr, IDC_ARROW);

    if (RegisterClassExW(&window_class) == 0 && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
    {
        throw std::runtime_error("Failed to register the application window class.");
    }

    handle_ = CreateWindowExW(
        0,
        class_name,
        window_title,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        800,
        600,
        nullptr,
        nullptr,
        instance_,
        this);

    if (handle_ == nullptr)
    {
        throw std::runtime_error("Failed to create the application window.");
    }

    ShowWindow(handle_, SW_SHOW);
}

HWND vf::app::window::handle() const
{
    return handle_;
}

bool vf::app::window::process_messages()
{
    MSG message{};
    while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE))
    {
        if (message.message == WM_QUIT)
        {
            return false;
        }

        TranslateMessage(&message);
        DispatchMessageW(&message);
    }
    return true;
}

void vf::app::window::close() const
{
    if (handle_ != nullptr)
    {
        PostMessageW(handle_, WM_CLOSE, 0, 0);
    }
}

LRESULT CALLBACK vf::app::window::window_proc(
    const HWND handle, const UINT message, const WPARAM w_param, const LPARAM l_param)
{
    auto self = reinterpret_cast<window*>(GetWindowLongPtrW(handle, GWLP_USERDATA));  // NOLINT(performance-no-int-to-ptr)
    if (message == WM_NCCREATE)
    {
        const auto* create_data = reinterpret_cast<const CREATESTRUCTW*>(l_param);  // NOLINT(performance-no-int-to-ptr)
        self = static_cast<window*>(create_data->lpCreateParams);
        SetWindowLongPtrW(handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
        self->handle_ = handle;
    }

    if (self != nullptr)
    {
        return self->handle_message(handle, message, w_param, l_param);
    }
    return DefWindowProcW(handle, message, w_param, l_param);
}

LRESULT vf::app::window::handle_message(
    const HWND handle, const UINT message, const WPARAM w_param, const LPARAM l_param) const
{
    switch (message)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_KEYDOWN:
        if (on_input_)
        {
            on_input_({input_event::type::key_down, static_cast<int>(w_param)});
        }
        return 0;
    case WM_MOUSEWHEEL:
        if (on_input_)
        {
            on_input_({input_event::type::mouse_wheel, GET_WHEEL_DELTA_WPARAM(w_param)});
        }
        return 0;
    default:
        return DefWindowProcW(handle, message, w_param, l_param);
    }
}
