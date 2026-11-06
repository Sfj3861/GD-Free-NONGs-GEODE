# GD Free NONGs

Geometry Dash mod client for the GD Free NONGs community platform.

**Target:** Geometry Dash 2.2091 · Geode v6.0.0-alpha.1 (experimental target).

The in-game product name is **GD Free NONGs**. “GEODE” appears only in this repository name and build tooling.

## Current implementation

- Geode mod project and CMake packaging setup.
- Explicit game/loader target metadata for Windows and Android.
- Main-menu initialization hook, master enable setting and a visible touch button in the main menu.
- Configurable universal Jukebox keybind (default `;`, corresponding to the physical Ñ position on a Spanish keyboard; rebind it on other layouts).
- In-game Jukebox catalogue: loads community songs and approved NONG versions from the backend.
- Paginated in-game catalogue (seven songs per page) backed by the searchable `/api/v1/catalog/songs` endpoint, with an in-game Geometry Dash song ID search field and a one-tap search reset.
- Browser library uses server-side search and pagination (18 songs per page).
- NONG selection downloads the audio into the mod's private save directory, checks its SHA-256 digest, saves the chosen version for the installation, and registers the local file for the game's song resolver.
- A “Use original song” action clears the local replacement and saves the original selection.
- Configurable API base URL in mod settings; loading the catalogue makes a live API request and reports connection or server errors. Remote servers must use HTTPS; HTTP is accepted only for localhost development.
- Both the main-menu touch button and keybind open the Jukebox layer in the running scene, suppress duplicate windows, and check the master switch.
- Persistent per-song local path registry.
- `MusicDownloadManager::pathForSong(int)` redirection hook for locally installed NONG files, with missing-file fallback to the original song.

## What is not implemented yet

The Jukebox supports a paginated song catalogue (seven songs per page), search by Geometry Dash song ID, a clear-search action, approved NONG listing and download/selection. The NONG-version list currently shows up to seven entries and does not yet have its own pagination. The mod has no upload UI or secure device pairing. The device identifier is a local installation identifier, not authentication. CI builds have succeeded for Windows and Android64, but the mod has not yet been tested inside Geometry Dash; successful compilation does not prove that the audio hook works correctly on a real device.

## Build

Install the matching Geode SDK and CLI for v6.0.0-alpha.1, set `GEODE_SDK` to the SDK directory, then:

```bash
cmake -S . -B build
cmake --build build --config Release
```

The Geode CMake integration creates the package target. Do not package against a stable SDK targeting 2.2081 and relabel it as 2.2091.

## Compatibility warning

Geode v6.0.0-alpha.1 is an experimental loader target for GD 2.209x. Windows and Android64 CI packages have been built successfully. Runtime compatibility with the installed game, especially the audio replacement hook, still needs to be checked on Windows and a real Android device.
