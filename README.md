# GD Free NONGs

Geometry Dash mod client for the GD Free NONGs community platform.

**Target:** Geometry Dash 2.2091 · Geode v6.0.0-alpha.1 (experimental target).

The in-game product name is **GD Free NONGs**. “GEODE” appears only in this repository name and build tooling.

## Current implementation

- Geode mod project and CMake packaging setup.
- Explicit game/loader target metadata for Windows and Android.
- Main-menu initialization hook.
- Master enable setting.
- Configurable universal Jukebox keybind. The default semicolon key code maps to the physical Ñ position on a Spanish keyboard; other layouts can rebind it.
- Keybind press listener with repeat suppression and master-switch check.

The hotkey currently logs a diagnostic event; it does not yet open the Jukebox. Music interception, the Jukebox UI, device authorization, API synchronization and local audio management remain under development. This is not yet a functional NONG replacement release.

## Build

Install the matching Geode SDK and CLI for v6.0.0-alpha.1, set `GEODE_SDK` to the SDK directory, then:

```bash
cmake -S . -B build
cmake --build build --config Release
```

The Geode CMake integration creates the package target. Do not package against a stable SDK targeting 2.2081 and relabel it as 2.2091.

## Compatibility warning

Geode v6.0.0-alpha.1 is an experimental loader target for GD 2.209x. This project has not yet been built or tested on Windows or Android. A successful compile is not proof that all platforms or the music hooks work.
