# GD Free NONGs

A community-powered alternative audio manager for Geometry Dash.

The in-game Jukebox connects to the GD Free NONGs API. Players can browse a searchable, paginated song catalogue, view approved alternative audio versions, download and verify a selected file using SHA-256, and restore the original song.

The browser can be paired with a game installation using a one-time code. Once paired, website selections are assigned to the game device. The Jukebox's **Sync website** action downloads the associated approved audio and applies local replacements after integrity verification.

This is an early development build. CI packages are built for Windows and Android64, but real in-game testing is still required. Direct in-game uploading, pagination of per-song NONG lists, hosted production infrastructure and full account authentication are not implemented. The API must be separately deployed and configured in the mod.
