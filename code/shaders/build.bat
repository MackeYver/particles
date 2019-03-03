@echo off
SETLOCAL ENABLEEXTENSIONS
SETLOCAL ENABLEDELAYEDEXPANSION

IF NOT "%2"=="nc" (
cls
)

IF NOT EXIST ..\..\build\shaders mkdir ..\..\build\shaders
PUSHD ..\..\build\shaders

IF EXIST vbasic.cso DEL /Q vbasic.cso
IF EXIST pbasic.cso DEL /Q pbasic.cso
IF EXIST vfullscreen_texture.cso DEL /Q vfullscreen_texture.cso
IF EXIST pfullscreen_texture.cso DEL /Q pfullscreen_texture.cso
IF EXIST gpoints_to_quads.cso    DEL /Q gpoints_to_quads.cso

IF "%1"=="r" ( 
  ECHO Compiling shaders, release mode...

  SET Options=/nologo /O2 /WX /Zpr

  REM Basic Shader
  REM ------------
  FXC !Options! /T vs_5_0 /E "vMain" /Fo vbasic.cso ..\..\code\shaders\basic.hlsl
  IF !errorlevel! NEQ 0 EXIT /b !errorlevel!

  FXC !Options! /T ps_5_0 /E "pMain" /Fo pbasic.cso ..\..\code\shaders\basic.hlsl
  IF !errorlevel! NEQ 0 EXIT /b !errorlevel!


  REM Texture shader
  REM --------------
  FXC !Options! /T vs_5_0 /E "vMain" /Fo vfullscreen_texture.cso ..\..\code\shaders\fullscreen_texture.hlsl
  IF !errorlevel! NEQ 0 EXIT /b !errorlevel!

  FXC !Options! /T ps_5_0 /E "pMain" /Fo pfullscreen_texture.cso ..\..\code\shaders\fullscreen_texture.hlsl
  IF !errorlevel! NEQ 0 EXIT /b !errorlevel!


  REM Geometry Shader
  REM ---------------
  FXC !Options! /T gs_5_0 /E "gMain" /Fo gpoints_to_quads.cso ..\..\code\shaders\points_to_quads.hlsl
  IF !errorlevel! NEQ 0 EXIT /b !errorlevel!

) ELSE (
  ECHO Compiling shaders, debug mode...

  SET Options=/nologo /Od /Zi /WX /Zpr

  REM Basic shaders
  REM -------------
  FXC !Options! /T vs_5_0 /E "vMain" /Fo vbasic.cso ..\..\code\shaders\basic.hlsl
  IF !errorlevel! NEQ 0 EXIT /b !errorlevel!

  FXC !Options! /T ps_5_0 /E "pMain" /Fo pbasic.cso ..\..\code\shaders\basic.hlsl
  IF !errorlevel! NEQ 0 EXIT /b !errorlevel!


  REM Texture shaders
  REM ---------------
  FXC !Options! /T vs_5_0 /E "vMain" /Fo vfullscreen_texture.cso ..\..\code\shaders\fullscreen_texture.hlsl
  IF !errorlevel! NEQ 0 EXIT /b !errorlevel!

  FXC !Options! /T ps_5_0 /E "pMain" /Fo pfullscreen_texture.cso ..\..\code\shaders\fullscreen_texture.hlsl
  IF !errorlevel! NEQ 0 EXIT /b !errorlevel!


  REM Geometry Shader
  REM ---------------
  FXC !Options! /T gs_5_0 /E "gMain" /Fo gpoints_to_quads.cso ..\..\code\shaders\points_to_quads.hlsl
  IF !errorlevel! NEQ 0 EXIT /b !errorlevel!
)

POPD