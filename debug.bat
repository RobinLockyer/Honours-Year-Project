@echo off
if not defined DevEnvDir (
    SET VSCMD_START_DIR="%CD%"
    "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
)
devenv build\SNGPSort.exe