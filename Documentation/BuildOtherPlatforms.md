# Building the Windows and Linux releases

Unreal Engine on macOS cannot compile Windows or Linux executables, so these two releases are built on a Windows PC. The project code is portable (no Mac-only APIs); the same source produces all three platforms.

## 1. One-time setup on the Windows PC

1. Install **Epic Games Launcher**, then **Unreal Engine 5.8** (Library → Engine Versions). In the engine's *Options*, tick **Linux** under *Target Platforms* if you will also build Linux.
2. Install **Visual Studio 2022** (Community is fine) with the workloads **Desktop development with C++** and **Game development with C++**.
3. For Linux only: install the **Linux cross-compile toolchain** that matches UE 5.8 (Epic's "Cross-Compiling for Linux" page lists the exact clang toolchain version). The installer sets the `LINUX_MULTIARCH_ROOT` environment variable; restart the PC afterwards.
4. Clone the repository: `git clone https://github.com/yhan3-ctrl/last-minute-courier.git`

## 2. Build

Open a Command Prompt in the repository folder and run:

```
Scripts\package_windows.bat "C:\Program Files\Epic Games\UE_5.8"
Scripts\package_linux.bat "C:\Program Files\Epic Games\UE_5.8"
```

Each script compiles the game, cooks the content, and writes a self-contained build to:

- `Builds\Windows\Windows\` → zip this folder as `LastMinuteCourier-0.10-Windows-x64.zip`
- `Builds\Linux\Linux\` → zip this folder as `LastMinuteCourier-0.10-Linux-x64.zip`

The first build takes a while because shaders are compiled once.

## 3. Check before uploading

- **Windows:** unzip on a second Windows PC if possible and run `LastMinuteCourier.exe`. Play from the first card to the parcel desk and accept the first order.
- **Linux (x86_64):** on Ubuntu 22.04 or newer, `chmod +x LastMinuteCourier.sh` then run `./LastMinuteCourier.sh`.
- The regression suite can also run on Windows: `LastMinuteCourier.exe -unattended -nullrhi -ExecCmds="Automation RunTests Courier.Regression;Quit"` in a Development build.

## 4. Submit

Upload the Windows zip, the Linux zip and the Mac DMG (from the GitHub release) to the course Box folder. The rubric asks for x86 Windows and Linux; these builds are 64-bit x86 (x64), the only x86 target Unreal Engine 5 supports.
