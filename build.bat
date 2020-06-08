@echo off

@rem Supported target types. These will determine which entry point source is 
@rem compiled with the appliction.
set TARG_WIN32=Win32
set TARG_CLI=cli


@rem Change this to build for different entry points
@rem set BUILD_TYPE=%TARG_WIN32%
@rem 
set BUILD_TYPE=%TARG_CLI%


echo Building for %BUILD_TYPE%



@rem VisualStudio compiler flags
set compFlags= -Zi


@rem If windows, then grab the windows libs.
if %BUILD_TYPE%==%TARG_WIN32% (
	set libs= user32.lib gdi32.lib	
) else (
	set libs= ws2_32.lib
)

@rem Set the entry point based on target.
if %BUILD_TYPE%==%TARG_WIN32% (
	set src=..\src\win32_main.cpp
) else (
	set src=..\src\main.cpp
)


@rem Add the main source files.
set src=%src% ..\src\sacw_main.cpp ..\src\tjd_ftp.cpp


@rem Get into the build directory and build the application
if not exist .\build mkdir .\build
pushd .\build

echo Running "cl %compFlags% %src% %libs%"
cl %compFlags% %src% %libs%

popd


@rem Execute the application after build.
if %BUILD_TYPE%==%TARG_WIN32% (
	.\build\win32_main.exe
) else (
	.\build\main.exe
)


