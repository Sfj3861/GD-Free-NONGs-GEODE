# GD Free NONGs

Geometry Dash mod client for the GD Free NONGs community platform.

**Target:** Geometry Dash 2.2091 · Geode v6.0.0-alpha.1 (experimental target).

The in-game product name is **GD Free NONGs**. “GEODE” appears only in this repository name and build tooling.

## Current implementation

- Geode mod project and CMake packaging setup.
- Explicit game/loader target metadata for Windows and Android.
- Main-menu initialization hook and master enable setting.
- Configurable universal Jukebox keybind (default `;`, corresponding to the physical Ñ position on a Spanish keyboard; rebind it on other layouts).
- In-game Jukebox catalogue: loads community songs and approved NONG versions from the backend.
- Paginated in-game catalogue (seven songs per page) backed by the searchable `/api/v1/catalog/songs` endpoint, with an in-game Geometry Dash song ID search field.
- Browser library uses server-side search and pagination (18 songs per page).
- NONG selection downloads the audio into the mod's private save directory, checks its SHA-256 digest, saves the chosen version for the installation, and registers the local file for the game's song resolver.
- A “Use original song” action clears the local replacement and saves the original selection.
- Configurable API base URL in mod settings; loading the catalogue makes a live API request and reports connection or server errors. Remote servers must use HTTPS; HTTP is accepted only for localhost development.
- Keybind listener opens the Jukebox layer in the running scene, suppresses key-repeat, and checks the master switch.
- Persistent per-song local path registry.
- `MusicDownloadManager::pathForSong(int)` redirection hook for locally installed NONG files, with missing-file fallback to the original song.

## What is not implemented yet

The Jukebox now has an initial catalogue → approved NONG list → selection/download flow. It currently shows up to seven entries per list and has no pagination, search, upload UI, or secure device pairing. The device identifier is a local installation identifier, not authentication. The C++ mod has not yet been compiled or runtime-tested; successful download and checksum code do not prove that the Geode hook replaces audio correctly.

## Build

Install the matching Geode SDK and CLI for v6.0.0-alpha.1, set `GEODE_SDK` to the SDK directory, then:

```bash
cmake -S . -B build
cmake --build build --config Release
```

The Geode CMake integration creates the package target. Do not package against a stable SDK targeting 2.2081 and relabel it as 2.2091.

## Compatibility warning

Geode v6.0.0-alpha.1 is an experimental loader target for GD 2.209x. This project has not yet been built or tested on Windows or Android. A successful metadata check is not a build, and a successful compile is not proof that all platforms or the music hooks work.
