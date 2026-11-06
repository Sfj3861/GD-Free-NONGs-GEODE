# Changelog

## v0.2.0
- Added the in-game Jukebox catalogue backed by the community API.
- Added approved NONG listing per Geometry Dash song.
- Added searchable paginated song catalogue in the backend, browser UI and in-game Jukebox, including in-game song ID search and a clear-search control.
- Added selection saving, audio download, SHA-256 verification, and local file registration.
- Added an action to restore the original song.
- Added asynchronous callback lifetime guards and clearer failure messages.
- Added a touch button to the main menu while retaining keyboard access.
- Added one-time browser/game pairing codes and a Jukebox action to sync website selections.
- Website downloads are checked with SHA-256 before local registration; original/unavailable selections clear local replacements.
- CI builds have succeeded for Windows and Android64; in-game runtime testing is still required.

## v0.1.0
- Initial Geode project foundation for Geometry Dash 2.2091.
- Added master enable setting.
- Added safe main-menu initialization smoke test.
