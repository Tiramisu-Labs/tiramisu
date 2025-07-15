@echo off
REM This script translates the bash commands to batch for setting up Emscripten SDK.

REM Get the emsdk repo
git clone https://github.com/emscripten-core/emsdk.git

REM Enter that directory
cd emsdk

REM Fetch the latest version of the emsdk (not needed the first time you clone)
git pull

REM Download and install the latest SDK tools.
emsdk.bat install latest

REM Make the "latest" SDK "active" for the current user. (writes .emscripten file)
emsdk.bat activate latest

REM Activate PATH and other environment variables in the current terminal
REM Note: emsdk_env.bat or emsdk_env.cmd will be created by the emsdk activate command.
call emsdk_env.bat

echo Emscripten SDK setup complete.
pause
