# GD Free NONGs

GD Free NONGs is an offline Geometry Dash mod for managing local replacement audio. It does not connect to a website, remote API, cloud storage or backend.

## Local workflow

1. Import an audio file into the companion browser library in the [GD Free NONGs repository](https://github.com/Sfj3861/GD-Free-NONGs).
2. Export the selected version. The browser downloads it as `song-<GD_ID>.<extension>`.
3. Open the in-game Jukebox and choose **Import audio**.
4. Select the exported file with the system file picker. The mod validates the filename, supported format, audio signature and 30 MiB size limit.
5. The mod copies the file into its private save directory and applies it to the matching Geometry Dash song ID.
6. Use **Delete** in My NONGs to remove the managed replacement and restore the original game audio.

Supported formats: MP3, OGG, WAV, FLAC and M4A.

## Compatibility

- Geometry Dash 2.2091 (Windows and Android64)
- Geode SDK 6.0.0-alpha.1
- C++20

Windows and Android64 CI builds are automated. A successful build verifies compilation and packaging, but does not replace runtime testing inside Geometry Dash on each platform.

## Audio rights

Only import and use audio you own or are authorized to use. A Geometry Dash or Newgrounds ID is an identifier, not permission to redistribute a track.
