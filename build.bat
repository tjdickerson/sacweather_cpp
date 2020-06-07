@echo off

set TARG_WIN32=Win32
set TARG_CLI=cli

@rem
set BUILD_TYPE=%TARG_WIN32%
@rem set BUILD_TYPE=%TARG_CLI%

echo Building for %BUILD_TYPE%

@rem VisualStudio compiler flags
set compFlags= -Zi

@rem If windows, then grab the windows libs.
if %BUILD_TYPE%==%TARG_WIN32% (
	set libs= user32.lib gdi32.lib	
) else (
	set libs=
)

if %BUILD_TYPE%==%TARG_WIN32% (
	set src=..\src\win32_main.cpp
) else (
	set src=..\src\main.cpp
)


if not exist .\build mkdir .\build
pushd .\build

echo Running "cl %compFlags% %src% %libs%"
cl %compFlags% %src% %libs%

popd


if %BUILD_TYPE%==%TARG_WIN32% (
	.\build\win32_main.exe
) else (
	.\build\main.exe
)


