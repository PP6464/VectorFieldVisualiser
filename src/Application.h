#pragma once

#include <windows.h>

#include "Camera.h"
#include "Renderer.h"
#include "SceneGeometry.h"
#include "Window.h"

namespace vf::app
{
    /// The application instance
    class application
    {
    public:
        explicit application(HINSTANCE instance);
        /// Runs the app and returns the exit code
        int run();

    private:
        /// Handles a scroll or key press
        void handle_input(const input_event& event);

        camera::camera camera_;
        scene::geometry geometry_;
        window window_;
        render::renderer renderer_;
    };
}
