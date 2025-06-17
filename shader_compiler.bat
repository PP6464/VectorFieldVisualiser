@echo off
set /p file_name=Shader File Name (no extension): 
set /p entry_point=Shader Entry Point: 
set /p shader_type=Shader Version: 
set /p compile_name=Compile File Name (no extension): 
"C:\Program Files (x86)\Windows Kits\10\bin\10.0.26100.0\x64\fxc.exe" /O3 /T %shader_type% /E %entry_point% /Fo Shaders/Compiled/%compile_name%.cso Shaders/%file_name%.hlsl