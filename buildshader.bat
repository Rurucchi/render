:: https://learn.microsoft.com/en-us/windows/win32/direct3dtools/dx-graphics-tools-fxc-using

:: fxc /Od /Zi /T vs_5_1 /E vs /Fo triangle.vs.fxc triangle.hlsl
:: fxc /Od /Zi /T ps_5_1 /E ps /Fo triangle.ps.fxc triangle.hlsl


@set OUT_DIR=build

::ps
@set SOURCE=src/shaders/ps.hlsl
@set OUT_FXC=ps.fxc
@set ENTRY=ps
@set PROFILE=ps_5_0

fxc /T %PROFILE% /E %ENTRY% /Fo %OUT_DIR%/%OUT_FXC% %SOURCE%


::vs
@set SOURCE=src/shaders/vs.hlsl
@set OUT_FXC=vs.fxc
@set ENTRY=vs
@set PROFILE=vs_5_0

fxc /T %PROFILE% /E %ENTRY% /Fo %OUT_DIR%/%OUT_FXC% %SOURCE%