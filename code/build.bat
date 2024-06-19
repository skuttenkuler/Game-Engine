@echo off

mkdir ..\build
pushd ..\build
pwd
cl -Zi ..\code\main.cpp user32.lib Gdi32.lib
popd
