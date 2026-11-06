# GD Free NONGs

Geometry Dash mod client for the GD Free NONGs community platform.

**Target:** Geometry Dash 2.2091 · Geode v6.0.0-alpha.1 (experimental target).

The in-game product name is **GD Free NONGs**. “GEODE” appears only in this repository name and build tooling.

## Current implementation

- Geode mod project and CMake packaging setup.
- Explicit game/loader target metadata for Windows and Android.
- Main-menu initialization hook and master enable setting.
- Configurable universal Jukebox keybind (default `;`, corresponding to the physical Ñ position on a Spanish keyboard; rebind it on other layouts).
- Initial in-game Jukebox layer with title, API connection test, and close control.
- Configurable API base URL in mod settings; the Jukebox can asynchronously test the server `/health` endpoint. Remote servers must use HTTPS; HTTP is accepted only for localhost development.
- Keybind listener opens the Jukebox layer in the running scene, suppresses key-repeat, and checks the master switch.
- Persistent per-song local path registry.
- `MusicDownloadManager::pathForSong(int)` redirection hook for locally installed NONG files, with missing-file fallback to the original song.

## What is not implemented yet

The current Jukebox is an early interface. It can test connectivity to a configured API server, but it does not yet list community songs, search, upload, select or download NONGs. The mod is not yet connected to the backend, and there is no device pairing or authorization flow. The local path resolver has not been compiled or runtime-tested, so actual song replacement is not verified.

## Build

Install the matching Geode SDK and CLI for v6.0.0-alpha.1, set `GEODE_SDK` to the SDK directory, then:

```bash
cmake -S . -B build
cmake --build build --config Release
```

The Geode CMake integration creates the package target. Do not package against a stable SDK targeting 2.2081 and relabel it as 2.2091.

## Compatibility warning

Geode v6.0.0-alpha.1 is an experimental loader target for GD 2.209x. This project has not yet been built or tested on Windows or Android. A successful metadata check is not a build, and a successful compile is not proof that all platforms or the music hooks work.
