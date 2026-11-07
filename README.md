# GD Free NONGs

Geometry Dash mod client for the GD Free NONGs community platform.

**Targets:** Geometry Dash 2.2091 · Geode 6.0.0-alpha.1 (experimental).

The in-game product name is **GD Free NONGs**. “GEODE” is used only for the repository and build tooling.

## Current implementation

- Geode project metadata and CMake packaging for Windows and Android64.
- Master enable setting and configurable API base URL.
- Main-menu touch button for opening the Jukebox, plus the configurable keyboard shortcut.
- Searchable catalogue of Geometry Dash song IDs, with seven songs per page and previous/next controls.
- One-tap clearing of the in-game song ID search.
- Approved NONG versions listed for each Geometry Dash song, with seven entries per page and previous/next navigation.
- Download of approved audio into the mod's private save directory.
- SHA-256 verification before any downloaded audio is registered.
- Local song resolver hook with fallback to the original game path when no valid local file is registered.
- Restore-original action for a song.
- One-time browser/game pairing: the Jukebox requests a 16-character code that expires after 10 minutes and can only be claimed once from the web PAIR tab.
- Pairing requests and claims are rate-limited by the API-observed client IP; production deployments behind a reverse proxy must also apply edge rate limiting.
- **Sync website** fetches selections associated with the paired game device, downloads approved NONGs, validates their SHA-256 values, and installs local replacements. Original or unavailable selections clear the corresponding local replacement.
- Remote API URLs must use HTTPS; HTTP is accepted only for exact localhost/127.0.0.1 development hosts.

## Not implemented yet

- Uploading NONG files directly from inside the game.
- Search within the list of NONG versions for an individual song.
- A full user-account system or multi-factor authentication; pairing is a one-time device-linking flow, not account identity verification.
- Production hosting, persistent managed database/object storage, backups, monitoring, abuse reporting and a full rights/takedown process.

The API repository must be deployed and configured in the mod's **API URL** setting before online functionality can be used. No public API server is provided by this repository.

## Build

Install the matching Geode SDK and CLI for v6.0.0-alpha.1, set `GEODE_SDK` to the SDK directory, then:

```bash
cmake -S . -B build
cmake --build build --config Release
```

The Geode CMake integration creates the package target. Do not build against a stable SDK targeting another Geometry Dash version and relabel the result as 2.2091.

## Compatibility and testing

GitHub Actions has produced Windows and Android64 packages for this project. A successful CI compile verifies that the code builds against the declared SDK; it does **not** replace testing the mod inside Geometry Dash on real devices. In particular, confirm that the main-menu button is positioned correctly and that the music resolver replaces audio on Android and Windows.
