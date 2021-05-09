@echo off

@rem Supported target types. These will determine which entry point source is 
@rem compiled with the appliction.
set TARG_WIN32=Win32


@rem Change this to build for different entry points
@rem 
set BUILD_TYPE=%TARG_WIN32%

echo %cd%
echo Building for %BUILD_TYPE%

set prjdir= %cd%
set includeDirs= /I %prjdir%\sacwlib\

@rem VisualStudio compiler flags
set compFlags= /Zi /EHsc /FeSACWeather /MD
set compFlags=%compFlags%%includeDirs%
set linkFlags= /link /subsystem:console


@rem If windows, then grab the windows libs.
if %BUILD_TYPE%==%TARG_WIN32% (
	set libs= user32.lib gdi32.lib urlmon.lib ws2_32.lib opengl32.lib %prjdir%\lib\libbz2.lib
)


@rem Set the entry point based on target.
if %BUILD_TYPE%==%TARG_WIN32% (
	set src=%cd%\windows\win32_main.cpp
)


@rem Add the main source files.
set src=%src% ^
%cd%\sacwlib\sacw_main.cpp ^
%cd%\sacwlib\tjd_ftp.cpp ^
%cd%\sacwlib\tjd_shapefile.cpp ^
%cd%\sacwlib\tjd_gl_render.cpp ^
%cd%\sacwlib\tjd_radar.cpp ^
%cd%\sacwlib\tjd_conversions.cpp ^
%cd%\sacwlib\nws_info.cpp


@rem Get into the build directory and build the application
if not exist %prjdir%\build mkdir %prjdir%\build
pushd %prjdir%\build

    @rem delete all contents for rebuild
    del /F /Q *.*

    echo Running "cl %compFlags% %src% %libs% %linkFLags%"
    cl %compFlags% %src% %libs% %linkFLags%

popd


