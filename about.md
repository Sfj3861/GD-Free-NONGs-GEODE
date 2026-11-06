# GD Free NONGs

A community-powered alternative music manager for Geometry Dash.

The in-game Jukebox connects to the GD Free NONGs API. Players can browse songs in the server catalogue, inspect approved alternative audio versions, download a selected version, and restore the original song. Downloaded files are checked against the server-provided SHA-256 digest before being registered in the mod's local song library.

The current implementation is an early development build. CI packages have been built for Windows and Android64, but it has not yet been tested inside Geometry Dash on a real installation. The Jukebox includes song ID search, catalogue pagination, a clear-search control, and a touch button in the main menu alongside the keyboard shortcut. Secure device pairing, an in-mod upload UI, NONG-list pagination and production service hardening remain future work. A configured API server must be available; the repository does not provide a hosted public API.
