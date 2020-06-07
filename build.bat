@echo off

set comp_flags="-Zi"
set libs=""
set src="../src/main.cpp"

if not exist .\build mkdir .\build
pushd .\build

cl %compFlags% %src%

popd

.\build\main.exe

