# GD Free NONGs

Geometry Dash mod client for the GD Free NONGs community platform.

**Target:** Geometry Dash 2.2091 · Geode v6.0.0-alpha.1 (experimental target).

The in-game product name is **GD Free NONGs**. “GEODE” appears only in this repository name and build tooling.

## Current implementation

- Geode mod project and CMake packaging setup.
- Explicit game/loader target metadata for Windows and Android.
- Main-menu initialization hook.
- Master enable setting.
- Configurable universal Jukebox keybind.
- Persistent per-song local path registry.
- `MusicDownloadManager::pathForSong(int)` redirection hook for locally installed NONG files, with missing-file fallback to the original song. The default semicolon key code maps to the physical Ñ position on a Spanish keyboard; other layouts can rebind it.
- Keybind press listener with repeat suppression and master-switch check.

The Ñ hotkey currently logs a diagnostic event; it does not yet open the Jukebox. The path resolver is wired to Geode's 2.2091 `MusicDownloadManager::pathForSong` binding and reads per-song local paths from Geode save data. The downloader and selection-sync pipeline that populate those paths, the Jukebox UI, and device authorization remain under development. The hook has not yet been compiled or runtime-tested, so this is not yet a release-ready NONG replacement.

## Build

Install the matching Geode SDK and CLI for v6.0.0-alpha.1, set `GEODE_SDK` to the SDK directory, then:

```bash
cmake -S . -B build
cmake --build build --config Release
```

The Geode CMake integration creates the package target. Do not package against a stable SDK targeting 2.2081 and relabel it as 2.2091.

## Compatibility warning

Geode v6.0.0-alpha.1 is an experimental loader target for GD 2.209x. This project has not yet been built or tested on Windows or Android. A successful compile is not proof that all platforms or the music hooks work.
