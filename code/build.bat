@ECHO OFF
SETLOCAL ENABLEEXTENSIONS
SETLOCAL ENABLEDELAYEDEXPANSION
cls

SET BuildMode="debug"
SET CompileShaders=0

IF "%1"=="d" SET BuildMode="debug"
IF "%1"=="f" SET BuildMode="fastdebug"
IF "%1"=="r" SET BuildMode="release"
IF "%2"=="s" SET CompileShaders=1

IF %CompileShaders%==1 (
  call shaders\build.bat nc
)

IF %errorlevel% NEQ 0 EXIT /b %errorlevel%



REM ----------------------------------------------------------------------------------
REM Compiler Options
REM https://docs.microsoft.com/en-us/cpp/build/reference/compiler-options-listed-alphabetically?view=vs-2017

SET IgnoredWarnings=/wd4100 /wd4201 /wd4505
SET GeneralCompilerOptions=/nologo /WL /fp:fast /fp:except- /EHsc /Gm- /Oi /FC /WX /W4 !IgnoredWarnings!

IF %BuildMode%=="release" (
  ECHO Release mode
  SET CompilerOptions=/O2 !GeneralCompilerOptions!
  SET CompilerOptions=/DRELEASE=1 /D_WIN32=1 /DOPTIMIZATION=2 !CompilerOptions!
) 
IF %BuildMode%=="fastdebug" (
  ECHO Fast debug mode
  SET CompilerOptions=/O2 !GeneralCompilerOptions! /Zi
  SET CompilerOptions=/D_WIN32=1 /DOPTIMIZATION=2 !CompilerOptions!
) 
IF  %BuildMode%=="debug" (
  ECHO Debug mode
  SET CompilerOptions=/Od !GeneralCompilerOptions! /Zi
  SET CompilerOptions=/DDEBUG=1 /D_WIN32=1 /DOPTIMIZATION=0 !CompilerOptions!
)


REM WL		 One line diagonostics
REM Ox		 Code generation x E [d = Debug, 1 = small code, 2 = fast code]
REM fp:fast    Fast floating point code generated
REM fp:except- No floating point exceptions
REM EHsc       Catches C++ exceptions only
REM GM-		Enables minimal rebuild (- disables it, we want all files compiled all the time)
REM Zi		 Produces separate PDB file that contains all the symbolic debugging information for use with the debugger
REM Zo		 Generate enhanced debugging information for optimized code in non-debug builds (enabled with Zi)
REM Oi		 Generates intrinsic functions.
REM WX		 Treats all warnings as errors
REM W4		 All warnings
REM wx		 Except...
REM			  4100 'identifier': unreferenced formal parameter
REM			  4201 nonstandard extension used: nameless struct/union
REM			  4505 unreferenced local function
REM FC		 Display full path of source code files passed to cl.exe in diagnostic text
REM GS		 Buffer security check
REM Gs		 Control stack checking calls



REM ----------------------------------------------------------------------------------
REM Linker Options
REM https://docs.microsoft.com/en-us/cpp/build/reference/linker-options?view=vs-2017

SET LinkerLibs=user32.lib d3d11.lib dxgi.lib dxguid.lib dwrite.lib d2d1.lib
REM Temp: gdi32.lib winmm.lib kernel32.lib

IF %BuildMode%=="release" (
  SET LinkerOptions=/MANIFEST:NO !LinkerLibs!  
) ELSE (
  SET LinkerOptions=/Profile /DEBUG:FULL /INCREMENTAL:NO /OPT:ICF /OPT:REF /MANIFEST:NO !LinkerLibs!
)

REM MANIFEST:NO    Prevents the linker from adding a manifest to the exe (this is done "manually" further down)
REM incremental:no Link Incrementally is not selected
REM opt:ref		eliminates functions and data that are never referenced



REM ----------------------------------------------------------------------------------
REM Build

IF NOT EXIST ..\build mkdir ..\build
PUSHD ..\build

ECHO Removing all old files...
del /Q *.*

ECHO Building...
cl %CompilerOptions% ../code/*.cpp /link /SUBSYSTEM:windows %LinkerOptions% /out:particles.exe

IF %errorlevel% NEQ 0 (
  popd
  EXIT /b %errorlevel%
)

ECHO Embedding manifest...
mt.exe -nologo -manifest "../data/a.manifest" -outputresource:"particles.exe;#1"

IF %errorlevel% NEQ 0 (
  popd
  EXIT /b %errorlevel%
)

ECHO All done.
POPD