# GD Free NONGs

Geometry Dash mod client for the GD Free NONGs community platform.

**Target:** Geometry Dash 2.2091 · Geode v6.0.0-alpha.1 (experimental target).

The in-game product name is **GD Free NONGs**. “GEODE” appears only in this repository name and build tooling.

## Current implementation

- Geode mod project and CMake packaging setup.
- Explicit game/loader target metadata.
- Main-menu load hook used as a safe initialization smoke test.
- Master enable setting.
- Architecture notes for the upcoming local library, API client, downloader and music resolver.

The first commit is a buildable foundation, not a completed NONG replacement engine. Music interception, hotkey registration (Ñ default), Jukebox UI, device authorization and API synchronization must be implemented and tested against the exact 2.2091 executable and Geode alpha SDK before a release can be called functional.

## Build

Install the matching Geode SDK and CLI for v6.0.0-alpha.1, set `GEODE_SDK` to the SDK directory, then:

```bash
cmake -S . -B build
cmake --build build --config Release
```

The Geode CMake integration creates the package target. Do not package against a stable SDK targeting 2.2081 and relabel it as 2.2091.

## Compatibility warning

Geode v6.0.0-alpha.1 is an experimental loader target for GD 2.209x. This project has not yet been built or tested on Windows or Android. A successful compile is not proof that all platforms or the music hooks work.
