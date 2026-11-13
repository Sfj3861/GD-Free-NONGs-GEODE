import json
from pathlib import Path

root = Path(__file__).resolve().parents[1]
metadata = json.loads((root / "mod.json").read_text(encoding="utf-8"))

assert metadata["name"] == "GD Free NONGs"
assert metadata["geode"] == "6.0.0-alpha.1"
assert metadata["gd"]["win"] == "2.2091"
assert metadata["gd"]["android"] == "2.2091"
assert metadata["id"] == "sfj3861.gdfreenongs"
assert metadata["version"] == "v0.3.0"
assert metadata["settings"]["open-jukebox"]["type"] == "keybind"
assert metadata["settings"]["enabled"]["default"] is True
assert "api-url" not in metadata["settings"]

main = (root / "src/main.cpp").read_text(encoding="utf-8")
jukebox = (root / "src/ui/JukeboxLayer.hpp").read_text(encoding="utf-8")
local_library = (root / "src/core/LocalLibrary.hpp").read_text(encoding="utf-8")
music_hook = (root / "src/hooks/MusicDownloadManager.cpp").read_text(encoding="utf-8")

assert '#include "ui/JukeboxLayer.hpp"' in main
assert "GFNJukeboxLayer::create()" in main
assert "onOpenGFNJukebox" in main
assert "fallbackMenu" in main
assert "gfn-open-jukebox" in main
assert "listenForKeybindSettingPresses" in main
assert "class GFNJukeboxLayer" in jukebox
assert "onClose" in jukebox
assert "showLibrary()" in jukebox
assert "onRefresh" in jukebox
assert "onDelete" in jukebox
assert "onImportAudio" in jukebox
assert "geode::utils::file::pick" in jukebox
assert "geode::utils::file::readBinary" in jukebox
assert "MAX_AUDIO_BYTES = 30u * 1024u * 1024u" in jukebox
assert "std::filesystem::file_size(sourcePath, sourceError)" in jukebox
assert jukebox.index("std::filesystem::file_size(sourcePath, sourceError)") < jukebox.index("geode::utils::file::readBinary(source)")
assert "geode::utils::file::writeBinarySafe" in jukebox
assert "validAudioPayload" in jukebox
assert "song-" in jukebox
assert "installedSongIDs()" in jukebox
assert "std::filesystem::directory_iterator" in local_library
assert "supportedAudioExtension" in local_library
assert "getSaveDir()" in local_library
assert "deleteManagedFileForSong" in local_library
assert "pathForSong(int songID)" in music_hook
assert "local_library::pathForSong" in music_hook

for source in (jukebox, local_library, music_hook):
    for forbidden in ("WebRequest", "/api/v1/", "https://", "api-url", "Cloudflare"):
        assert forbidden not in source, f"Remote dependency found in Geode source: {forbidden}"

print("GD Free NONGs offline metadata and source-wiring checks passed.")
print("Note: these checks do not compile or run the Geode mod.")
