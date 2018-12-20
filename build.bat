@echo off
if not defined DevEnvDir (
    SET VSCMD_START_DIR="%CD%"
    "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
)
pushd build
cl -Zi ..\src\SNGPSort.c
REM gcc -o SNGPSort ..\src\SNGPSort.c
popd