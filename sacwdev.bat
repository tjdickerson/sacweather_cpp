@echo off
@REM This batch file sets up the compiler environment and launches sublime text for the target project

@REM Set up the visual studio compiler path
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x86_amd64


cd \code\sacweather\

@REM Add sublime text to the path
set PATH="C:\Program Files\Sublime Text 3";%PATH%

@REM Open sublime text with the project loaded.
subl.exe sacweather.sublime-project

