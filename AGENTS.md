# AGENTS.md

This file provides guidance to WARP (warp.dev) when working with code in this repository.

## Project Overview

HomeAssistantDesktop is a Windows desktop application that provides a system tray interface for controlling Home Assistant entities. Built with Qt 6 and C++17, it uses WebSocket for real-time communication with Home Assistant and includes a local HTTP server for receiving notifications. The application is packaged as a Windows MSIX package.

## Build Commands

### Prerequisites
- Visual Studio 2022 (v143 toolset) with C++ Desktop Development
- Qt 6.8.3+ with modules: qtactiveqt, qthttpserver, qtwebsockets
- NuGet (for Microsoft.Windows.CppWinRT package)
- Git (for submodules)

### Initial Setup
```pwsh
# Clone with submodules (spdlog is a git submodule)
git clone --recursive <repo-url>

# Or initialize submodules if already cloned
git submodule update --init --recursive

# Restore NuGet packages
nuget restore src\HomeAssistantDesktop.sln
```

### Build via MSBuild
```pwsh
# Debug build (x64)
msbuild src\HomeAssistantDesktop.sln /p:Configuration=Debug /p:Platform=x64

# Release build (x64)
msbuild src\HomeAssistantDesktop.sln /p:Configuration=Release /p:Platform=x64

# Debug build (ARM64)
msbuild src\HomeAssistantDesktop.sln /p:Configuration=Debug /p:Platform=ARM64

# Release build (ARM64)
msbuild src\HomeAssistantDesktop.sln /p:Configuration=Release /p:Platform=ARM64
```

Build outputs are placed in `bin\{Platform}\{Configuration}\` directories.

### Build via Visual Studio
Open `src\HomeAssistantDesktop.sln` in Visual Studio and build using the IDE. Select platform (x64/ARM64) and configuration (Debug/Release) from the toolbar.

### Deploy Qt Dependencies
After building, copy Qt runtime dependencies to the output directory:
```pwsh
cd bin\x64\Release
windeployqt --release --no-translations --no-system-d3d-compiler --no-system-dxc-compiler --no-compiler-runtime .\HomeAssistantDesktop.exe
```

For Debug builds, use `--debug` instead of `--release`.

### Create Debug Identity
Debug builds automatically run a post-build event to create a debug identity using the `winapp` CLI:
```pwsh
winapp create-debug-identity --manifest <path-to-appxmanifest.xml> <executable-path>
```
This enables debugging the packaged application with Windows Store identity capabilities.

## Architecture

### Project Structure
- **HomeAssistantDesktop** (Qt application project): Main executable with GUI and business logic
- **WinApi** (Static library): Windows Runtime interop layer for credential storage and startup tasks
- **spdlog** (Git submodule): Header-only logging library

### Core Components

**Main.cpp**
Entry point that initializes logging, configuration, and the Qt application. Sets up the system tray view and connects error handlers.

**ConfigurationService**
Manages application configuration including Home Assistant authentication tokens (stored via WinApi/PasswordVault) and startup preferences.

**HomeAssistantService**
WebSocket client that connects to Home Assistant. Handles authentication, ping/pong, reconnection logic, service calls, and event subscriptions. Uses Qt's QWebSocket.

**NotificationServer**
Local QHttpServer listening for notification POST requests from Home Assistant, typically sent to localhost:8080.

**TrayViewModel**
Business logic layer between the UI and services. Maintains entity states (lights, switches, sensors), processes Home Assistant events, and exposes commands to the view.

**TrayView**
Qt-based system tray UI with context menus. Uses DualMenuSystemTrayIcon to switch between connected/disconnected states and show CO2 sensor values.

**WinApi Library**
C++/WinRT wrapper providing:
- `WinApi_StoreAuthToken` / `WinApi_ReadAuthToken`: PasswordVault integration for secure token storage
- `WinApi_SetStartupEnabled` / `WinApi_GetStartupEnabled`: Windows StartupTask registration

### Data Flow
1. Application starts → ConfigurationService retrieves auth token from Windows PasswordVault
2. HomeAssistantService connects via WebSocket → authenticates with token
3. ViewModel subscribes to entity state changes and fetches initial states
4. User interacts with system tray → TrayView calls TrayViewModel methods → calls HomeAssistantService.CallService
5. Home Assistant sends state change events → HomeAssistantService emits EventReceived → TrayViewModel updates state → TrayView updates UI
6. Home Assistant sends notifications via HTTP POST → NotificationServer emits signal → displayed as Windows toast

### Qt Modules Used
- **QtCore/QtGui**: Core application framework
- **QtWidgets**: System tray and menus
- **QtNetwork**: Base networking
- **QtWebSockets**: WebSocket client for Home Assistant
- **QtHttpServer**: Local HTTP server for notifications

### Language Standard
C++17 (`/std:c++17` in vcxproj)

## Testing

This repository does not contain unit tests. The spdlog submodule contains its own tests but they are not built as part of this solution.

Manual testing is done by running the executable and verifying Home Assistant connectivity and entity control.

## Packaging

The application uses Windows MSIX packaging with manifest `appxmanifest.xml`. Key capabilities:
- `runFullTrust`: Required for full Win32 API access
- StartupTask extension: Enables "Start with Windows" functionality

Package identity is `HomeAssistantDesktop` by `CN=Hernan`.

Debug identity generation via `winapp create-debug-identity` allows debugging the packaged app with Windows Runtime APIs without full store signing.

## Code Style Conventions

- Use Qt naming conventions for Qt classes: `CamelCase` for types, signals/slots use `OnEventName` prefix
- Use `_memberVariable` naming for private members
- Use `QPointer<T>` for all Qt object pointers that could become invalid
- Use `spdlog::info`, `spdlog::error`, etc. for logging (header-only mode)
- Precompiled headers: WinApi uses `pch.h`, HomeAssistantDesktop does not

## Common Entity IDs (Hardcoded)
The application is currently hardcoded to specific Home Assistant entity IDs:
- `switch.humidifier_switch`
- `switch.test_plug`
- `light.bedroom_light`
- `light.kitchen_light`
- `light.office_light`
- `sensor.air_quality_co2` (for CO2 sensor display)
- `automation.humidifier_on` (automation toggle)

When modifying entity handling, these are the entity IDs currently referenced in TrayViewModel.cpp.
