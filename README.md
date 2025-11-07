# torrent-client

A lightweight desktop BitTorrent client written in modern C++ with a Qt-based GUI. This project uses C++20, Qt Widgets + Qt Charts and `libtorrent-rasterbar` as the torrent backend. The codebase is organized to separate core torrent/session logic from GUI code (models, delegates, widgets, dialogs), making it easier to extend and maintain.

This README provides:
- Project overview and goals
- Key features
- Prerequisites and dependencies
- Build and install instructions (Linux / macOS / Windows)
- Runtime notes and usage
- Project layout and key components
- Development tips (formatting, debugging)
- Contributing guidelines
- Known issues and TODOs
- License pointer

---

## Overview

The goal of this project is to provide a clean, modular torrent client that demonstrates:
- Good separation of concerns between networking/session code and UI
- Modern C++ practices (C++20)
- Qt-based UI built using Qt Designer `.ui` files
- Integration with `libtorrent-rasterbar` (via `pkg-config`) for robust BitTorrent support
- Extensible models, delegates, and widgets for a responsive UI

The project uses CMake for configuration and supports both Qt5 and Qt6 (Widgets and Charts).

---

## Key features

- Add torrents via `.torrent` files or magnet links
- Torrent list with sortable model and custom delegates
- File tree view with per-file priority controls
- Peers and trackers views (tables)
- Speed graph (Qt Charts)
- Dialogs for: adding peers, saving torrents, managing peers, torrent settings, application settings
- Light and dark themes via `.qss` files (bundled in resources)
- Modular design: `src/core` (logic) and `src/gui` (UI)

---

## Prerequisites

You will need:
- C++ compiler supporting C++20 (GCC, Clang, MSVC)
- CMake >= 3.16
- pkg-config
- Qt (5.15+ or 6.x) with Widgets and Charts modules
- libtorrent-rasterbar development files
- Optional: libmaxminddb for GeoIP lookups

Common package names:
- Debian/Ubuntu: `build-essential cmake pkg-config libtorrent-rasterbar-dev qtbase5-dev qttools5-dev-tools libqt5charts5-dev` (adjust for Qt6)
- Fedora: corresponding `-devel` packages
- macOS (Homebrew): `cmake pkg-config qt libtorrent-rasterbar`
- Windows: Qt (official installer) and libtorrent prebuilt or built from source

---

## Build & Install

General approach: out-of-source CMake build.

Basic (cross-platform)
1. Create a build directory:
   - mkdir -p build && cd build
2. Configure:
   - cmake .. -DCMAKE_BUILD_TYPE=Release
   - If CMake cannot find Qt or libtorrent, set `CMAKE_PREFIX_PATH` (for Qt) and `PKG_CONFIG_PATH` (for libtorrent) accordingly.
3. Build:
   - cmake --build . --config Release
4. Install (optional):
   - cmake --install . --prefix /usr/local

Platform-specific hints:

Linux
- Install dependencies through package manager.
- Example:
  - sudo apt install build-essential cmake pkg-config libtorrent-rasterbar-dev qtbase5-dev libqt5charts5-dev
- Build as above.

macOS (Homebrew)
- brew install cmake pkg-config qt libtorrent-rasterbar
- You may need to point CMake to Qt:
  - export CMAKE_PREFIX_PATH="$(brew --prefix qt)/lib/cmake"
- Then run the general CMake steps.

Windows
- Use the Qt online installer and Visual Studio (MSVC).
- Either obtain libtorrent binaries or build libtorrent from source.
- Use `cmake -G "Visual Studio 16 2019" -A x64 -DCMAKE_PREFIX_PATH="C:\Qt\<version>\lib\cmake" ..`
- Build with Visual Studio or `cmake --build . --config Release`.
- Use `windeployqt` to gather Qt DLLs into the build output folder.

CMake options that may be useful:
- `-DCMAKE_BUILD_TYPE=Debug|Release`
- `-DCMAKE_PREFIX_PATH=/path/to/Qt` (help CMake find Qt)
- `PKG_CONFIG_PATH` environment variable for `libtorrent-rasterbar` if not found in standard locations

Notes about Qt6 vs Qt5
- The codebase is set up to work with Qt6 or Qt5; CMake searches for either. Ensure the corresponding Charts and Widgets modules are present for the version you use.

---

## Running the application

After a successful build the executable is named `torrent-client`. From the build directory:

- Linux/macOS:
  - ./torrent-client
- Windows:
  - run `torrent-client.exe` from the appropriate build configuration folder (Debug/Release)

Runtime requirements:
- Ensure Qt runtime libraries and `libtorrent` shared libraries are discoverable (PATH on Windows, LD_LIBRARY_PATH on Linux if using non-standard install paths).
- The app loads UI resources (compiled via `resources.qrc`) and theme `.qss` files bundled into resources. Consider copying themes to user directories for runtime customization if you implement a runtime search path.

---

## Project layout

Top-level:
- `CMakeLists.txt` — project build configuration
- `LICENSE` — license file
- `src/` — source code
  - `main.cpp` — application entrypoint
  - `core/` — core logic and utilities
    - `controllers/` — session lifecycle management (e.g. `sessionmanager.*`)
    - `models/` — Qt data models (`torrentstablemodel`, `peertablemodel`, `filetreemodel`, `trackertablemodel`)
    - `delegates/` — painting/edit delegates for views
    - `filters/` — model filters such as category sorting
    - `net/` — network helpers (e.g. `metadatafetcher`)
    - `utils/` — data types and helpers (`torrent.h`, `torrenthandle.*`, `peer.h`, `tracker.h`, `dirs.h`, `category.h`)
  - `gui/` — UI code and dialogs
    - `widgets/` — reusable widgets and settings pages
    - `dialogs/` — modal dialogs (add peers, manage peers, save torrent, settings)
    - `mainwindow.*` — main window UI and logic
  - `resources/` — `themes.qrc` referencing `dark.qss`, `light.qss`

Naming & design conventions:
- Keep GUI-only logic under `src/gui` and core/session interactions under `src/core`.
- Use models to expose torrent/peer/tracker data to views and delegates to handle custom rendering and editing.

---

## Contributing

Guidelines:
- Open an issue describing feature requests or bugs before major PRs.
- Use small focused PRs; write descriptive commit messages.
- Maintain API stability for public-facing controllers/models where possible.
- Run formatting and static checks before submitting review.
- Add tests for new features and bug fixes where reasonable.

Code review checklist:
- Is the change limited in scope?
- Are new public APIs documented?
- Are edge cases handled (I/O errors, missing metadata, network failures)?
- Are UI changes accessible and localizable?

---

## Known issues & TODO

- Packaging/installer scripts are not provided yet.
- No automated tests are included yet.
- Some libtorrent interactions may require additional hardening around disk allocation and resume data handling
- Add more robust reconnection/peer management policies and improved tracker failure handling.
- Improve error reporting and user-facing messages for common failure modes (permission errors, missing dependencies).

---

## License

See the `LICENSE` file at the project root for licensing details.

---

## Honorable mentions

- [qBitTorrent](https://www.qbittorrent.org/)
- [Icons](https://www.flaticon.com)
