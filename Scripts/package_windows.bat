@echo off
rem Builds the Windows x64 release of Last Minute Courier on a Windows PC.
rem Usage: Scripts\package_windows.bat "C:\Program Files\Epic Games\UE_5.8"
setlocal
set "ENGINE=%~1"
if "%ENGINE%"=="" set "ENGINE=C:\Program Files\Epic Games\UE_5.8"
set "PROJECT=%~dp0..\LastMinuteCourier.uproject"
set "OUT=%~dp0..\Builds\Windows"
if not exist "%ENGINE%\Engine\Build\BatchFiles\RunUAT.bat" (
  echo Unreal Engine not found at "%ENGINE%". Pass the engine folder as the first argument.
  exit /b 2
)
call "%ENGINE%\Engine\Build\BatchFiles\RunUAT.bat" BuildCookRun -project="%PROJECT%" -noP4 -unattended -utf8output ^
  -platform=Win64 -clientconfig=Development -build -cook -stage -pak -archive -archivedirectory="%OUT%" -nodebuginfo
if errorlevel 1 (
  echo Build failed. See the log above.
  exit /b 1
)
echo Done: "%OUT%"
endlocal
