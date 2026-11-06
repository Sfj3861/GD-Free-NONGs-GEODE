import json
from pathlib import Path

root = Path(__file__).resolve().parents[1]
metadata = json.loads((root / 'mod.json').read_text(encoding='utf-8'))

assert metadata['name'] == 'GD Free NONGs', 'The in-game name must not include GEODE'
assert metadata['geode'] == '6.0.0-alpha.1', 'Use the canonical SDK version without a leading v'
assert metadata['gd']['win'] == '2.2091'
assert metadata['gd']['android'] == '2.2091'
assert metadata['id'] == 'sfj3861.gdfreenongs'
assert metadata['version'] == 'v0.2.0'
assert metadata['settings']['open-jukebox']['type'] == 'keybind'
assert metadata['settings']['open-jukebox']['default'] == ';'
assert metadata['settings']['enabled']['default'] is True
assert metadata['settings']['api-url']['type'] == 'string'
assert metadata['settings']['api-url']['default'] == ''

main = (root / 'src/main.cpp').read_text(encoding='utf-8')
jukebox = (root / 'src/ui/JukeboxLayer.hpp').read_text(encoding='utf-8')
local_library = (root / 'src/core/LocalLibrary.hpp').read_text(encoding='utf-8')
music_hook = (root / 'src/hooks/MusicDownloadManager.cpp').read_text(encoding='utf-8')

assert '#include "ui/JukeboxLayer.hpp"' in main
assert 'GFNJukeboxLayer::create()' in main
assert 'listenForKeybindSettingPresses' in main
assert 'class GFNJukeboxLayer' in jukebox
assert 'onClose' in jukebox
assert 'void loadSongs(int page = 1)' in jukebox
assert 'loadNongs(' in jukebox
assert '/api/v1/catalog/songs' in jukebox
assert 'limit=7' in jukebox
assert 'TextInput::create' in jukebox
assert 'onSearch' in jukebox
assert 'onClearSearch' in jukebox
assert '/api/v1/selections/save' in jukebox
assert '/api/v1/nongs/' in jukebox
assert 'onUseOriginal' in jukebox
assert 'geode::sha256' in jukebox
assert 'AsyncRelease hold' in jukebox
assert 'local_library::setPathForSong' in jukebox
assert 'local_library::clearPathForSong' in jukebox
assert 'WebRequest()' in jukebox
assert 'setPathForSong' in local_library
assert 'pathForSong(int songID)' in music_hook

print('GD Free NONGs metadata and source-wiring checks passed.')
print('Note: these checks do not compile or run the Geode mod.')
