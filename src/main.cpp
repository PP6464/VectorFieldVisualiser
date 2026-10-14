#include "Application.h"

#include <exception>

int WINAPI WinMain(const HINSTANCE instance, HINSTANCE, LPSTR, int)
{
    try
    {
        vf::app::application application(instance);
        return application.run();
    }
    catch (const std::exception& error)
    {
        MessageBoxA(nullptr, error.what(), "Vector Field Visualiser error", MB_OK | MB_ICONERROR);
        return 1;
    }
}
