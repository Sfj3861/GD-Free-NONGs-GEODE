import json
from pathlib import Path

metadata = json.loads(Path("mod.json").read_text(encoding="utf-8"))
assert metadata["name"] == "GD Free NONGs", "The in-game name must not include GEODE"
assert metadata["geode"] == "v6.0.0-alpha.1", "Unexpected Geode target"
assert metadata["gd"]["win"] == "2.2091"
assert metadata["gd"]["android"] == "2.2091"
assert metadata["id"] == "sfj3861.gdfreenongs"
assert metadata["settings"]["open-jukebox"]["type"] == "keybind"
assert metadata["settings"]["open-jukebox"]["default"] == ";"
assert metadata["settings"]["enabled"]["default"] is True
print("mod.json metadata checks passed")
