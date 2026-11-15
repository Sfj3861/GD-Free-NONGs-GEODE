# Changelog

## v0.3.0
- Replaced the remote catalogue and website-pairing flow with a fully offline local NONG workflow.
- Added system file-picker import from the exported `song-<GD_ID>.<extension>` format.
- Added validation for positive song IDs, supported audio extensions, audio signatures and the 30 MiB file-size limit.
- Added local save-directory storage, library scanning, pagination, refresh and managed-file deletion.
- Added original-song fallback and explicit disable markers to prevent stale alternate-format files from being reactivated after deletion or file loss.
- Added a main-menu touch button for devices without a physical keyboard and retained a configurable keyboard shortcut.
- Targets Geometry Dash 2.2091 with Geode SDK 6.0.0-alpha.1 (Windows and Android64).

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

## v0.1.0
- Initial Geode project foundation for Geometry Dash 2.2091.
- Added master enable setting.
- Added safe main-menu initialization smoke test.
