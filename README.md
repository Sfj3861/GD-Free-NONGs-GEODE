# GD Free NONGs — Offline Geode Mod

A fully local Geometry Dash mod for managing alternative song files. The mod does not contact a remote API, use Cloudflare, require FastAPI, or depend on a cloud service.

**Targets:** Geometry Dash 2.2091 · Geode SDK 6.0.0-alpha.1.

## How the offline workflow works

1. Use the companion browser app in the [GD Free NONGs repository](https://github.com/Sfj3861/GD-Free-NONGs) to import an audio file into the browser's local IndexedDB library.
2. Select the version and choose **EXPORT FOR GEODE**. The browser saves it as song-&lt;GD_ID&gt;.&lt;extension&gt;.
3. Copy that file into this mod's Geode save directory.
4. Open the in-game Jukebox using the main-menu NONGs button or configured keyboard shortcut.
5. The Jukebox scans its save directory and automatically registers matching files.
6. The song resolver redirects the matching Geometry Dash song ID to the local audio file.
7. Use **Delete** in My NONGs to remove a managed file and return to the original game audio.

Supported extensions: MP3, OGG, WAV, FLAC and M4A.

## Local library

- Detects files named song-&lt;positive song ID&gt;.&lt;supported extension&gt; in the mod save directory.
- Stores the resolved path and a local song-ID index in Geode saved values.
- Displays filename and file size, with seven entries per page.
- Re-scans the folder on refresh.
- Deletes only a matching managed audio file whose canonical parent directory is the mod's own save directory.
- Falls back to the original Geometry Dash audio if no local replacement is registered or the file is unavailable.

The browser and the game do not share storage. Exporting and copying the file is the deliberate offline bridge; no pairing or network connection is used.

## Compatibility and build

- Geometry Dash 2.2091 (Windows and Android)
- Geode SDK 6.0.0-alpha.1
- C++20

~~~bash
cmake -S . -B build
cmake --build build --config Release
~~~

A successful build does not replace in-game testing on both target platforms.

## Rights

Only use audio you own or are authorized to use. A Geometry Dash or Newgrounds ID is an identifier, not permission to redistribute a track.
