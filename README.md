# Vector Field Visualiser

A DirectX application that lets you visualise a vector field (defined within the code).

## Code organization

Application startup and input coordination are in `src/Application.*`; Win32 window handling is in `src/Window.*`;
camera controls are in `src/Camera.*`; CPU-side axes and vector-field geometry are built in `src/SceneGeometry.*`;
and Direct3D resource management and frame rendering are in `src/Renderer.*`. The vector field itself is defined in
`src/VectorField.cpp`, with the shared vertex layout in `src/Vertex.h`.

## Running the program

This works on Windows only, as it uses DirectX. It can be run inside Visual Studio, or by running the following commands (you must have the Visual Studio Developer Command prompt installed):

- `devenv VectorFieldVisualiser.sln /build` to build
- `devenv VectorFieldVisualiser.sln /run` to run

## Interacting with the visualizer
The visualizer allows for rotation, panning and zooming. This can be achieved with the following bindings:
- Arrows: Rotation
- WASD: Panning
- Mouse scroll: Zoom

## Example output

!["See assets/example.png"](./assets/example.png)


### Note on `shader_compiler.bat`

This is a bat file to generate shader cso files, and when run will request the following:

1. Shader file name (without the extension): The name of the shader file to be compiled (within the `Shaders` directory)
2. Shader entry point: The function to be compiled
3. Compiled name (without the extension): The name of the `.cso` file the shader will be compiled to. This will be
   in the `Shaders/Compiled` directory 