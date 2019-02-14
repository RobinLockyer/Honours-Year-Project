@echo off
if NOT EXIST "build" mkdir build
if not defined DevEnvDir (
    SET VSCMD_START_DIR="%CD%"
    if EXIST "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" (
	
		"C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
	
	) else (
	
		"C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" x64
	
	)
)
pushd build
cl -Zi -ZI ..\src\SNGPSort.c
REM gcc -o SNGPSort ..\src\SNGPSort.c
popd