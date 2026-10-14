#pragma once

#include <functional>

#include <windows.h>

namespace vf::app
{
    /// Represents a key press or scroll
    struct input_event
    {
        enum class type
        {
            key_down,
            mouse_wheel
        };

        type event_type;
        int value;
    };

    /// Represents the window
    class window
    {
    public:
        using input_handler = std::function<void(const input_event&)>;

        explicit window(HINSTANCE instance, input_handler on_input);
        ~window();

        window(const window&) = delete;
        window& operator=(const window&) = delete;

        void create();
        [[nodiscard]] HWND handle() const;
        [[nodiscard]] static bool process_messages();
        void close() const;

    private:
        /// Handles window callbacks
        static LRESULT CALLBACK window_proc(HWND handle, UINT message, WPARAM w_param, LPARAM l_param);
        /// Helper for window_proc to handle callbacks
        LRESULT handle_message(HWND handle, UINT message, WPARAM w_param, LPARAM l_param) const;

        HINSTANCE instance_;
        HWND handle_ = nullptr;
        input_handler on_input_;
    };
}
