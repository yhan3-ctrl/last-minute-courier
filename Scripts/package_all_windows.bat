@echo off
rem One-step Windows + Linux release build for Last Minute Courier.
rem Run on a Windows PC that has Unreal Engine 5.8 and Visual Studio 2022
rem (workload "Game development with C++"). Double-click it, or run:
rem   Scripts\package_all_windows.bat ["C:\path\to\UE_5.8"]
rem Linux is built too when the UE Linux cross-compile toolchain is installed
rem (LINUX_MULTIARCH_ROOT is set); otherwise it is skipped with a message.
setlocal EnableExtensions EnableDelayedExpansion
cd /d "%~dp0.."
set "ROOT=%CD%"
set "VERSION=0.10"
set "ENGINE=%~1"
if "%ENGINE%"=="" for %%E in ("C:\Program Files\Epic Games\UE_5.8" "D:\Program Files\Epic Games\UE_5.8" "D:\Epic Games\UE_5.8" "E:\Epic Games\UE_5.8" "C:\Epic Games\UE_5.8") do if exist "%%~E\Engine\Build\BatchFiles\RunUAT.bat" if "!ENGINE!"=="" set "ENGINE=%%~E"
if not exist "%ENGINE%\Engine\Build\BatchFiles\RunUAT.bat" (
  echo Could not find Unreal Engine 5.8. Drag the UE_5.8 folder onto this window or pass it as the first argument.
  pause
  exit /b 2
)
echo Engine: %ENGINE%
set "OUT=%ROOT%\Builds"
if not exist "%OUT%" mkdir "%OUT%"

echo.
echo ===== 1/2 Windows x64 =====
call "%ENGINE%\Engine\Build\BatchFiles\RunUAT.bat" BuildCookRun -project="%ROOT%\LastMinuteCourier.uproject" -noP4 -unattended -utf8output -platform=Win64 -clientconfig=Development -build -cook -stage -pak -archive -archivedirectory="%OUT%\Windows" -nodebuginfo
if errorlevel 1 (
  echo Windows build failed. Scroll up for the first error.
  pause
  exit /b 1
)
powershell -NoProfile -Command "Compress-Archive -Force -Path '%OUT%\Windows\Windows\*' -DestinationPath '%OUT%\LastMinuteCourier-%VERSION%-Windows-x64.zip'; (Get-FileHash '%OUT%\LastMinuteCourier-%VERSION%-Windows-x64.zip' -Algorithm SHA256).Hash.ToLower() + '  LastMinuteCourier-%VERSION%-Windows-x64.zip' | Set-Content '%OUT%\LastMinuteCourier-%VERSION%-Windows-x64.zip.sha256'"

echo.
echo ===== 2/2 Linux x64 =====
if "%LINUX_MULTIARCH_ROOT%"=="" (
  echo Skipped: the Unreal Linux cross-compile toolchain is not installed.
  echo Install the toolchain listed for UE 5.8 on Epic's "Cross-Compiling for Linux" page, restart, then run this again.
  goto done
)
call "%ENGINE%\Engine\Build\BatchFiles\RunUAT.bat" BuildCookRun -project="%ROOT%\LastMinuteCourier.uproject" -noP4 -unattended -utf8output -platform=Linux -clientconfig=Development -build -cook -stage -pak -archive -archivedirectory="%OUT%\Linux" -nodebuginfo
if errorlevel 1 (
  echo Linux build failed. Scroll up for the first error.
  pause
  exit /b 1
)
powershell -NoProfile -Command "Compress-Archive -Force -Path '%OUT%\Linux\Linux\*' -DestinationPath '%OUT%\LastMinuteCourier-%VERSION%-Linux-x64.zip'; (Get-FileHash '%OUT%\LastMinuteCourier-%VERSION%-Linux-x64.zip' -Algorithm SHA256).Hash.ToLower() + '  LastMinuteCourier-%VERSION%-Linux-x64.zip' | Set-Content '%OUT%\LastMinuteCourier-%VERSION%-Linux-x64.zip.sha256'"

:done
echo.
echo Finished. Upload these from "%OUT%":
dir /b "%OUT%\*.zip" "%OUT%\*.sha256"
pause
endlocal
