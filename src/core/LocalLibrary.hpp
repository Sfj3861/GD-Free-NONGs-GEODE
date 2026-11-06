#pragma once

#include <Geode/Geode.hpp>
#include <string>

namespace gfn::local_library {
    inline std::string keyForSong(int songID) {
        return fmt::format("local-nong-path-{}", songID);
    }

    // An empty path means the game should use its normal song file.
    inline std::string pathForSong(int songID) {
        auto mod = geode::Mod::get();
        if (!mod || songID <= 0) {
            return {};
        }
        return mod->getSavedValue<std::string>(keyForSong(songID), "");
    }

    inline void setPathForSong(int songID, std::string const& path) {
        auto mod = geode::Mod::get();
        if (mod && songID > 0) {
            mod->setSavedValue(keyForSong(songID), path);
        }
    }

    inline void clearPathForSong(int songID) {
        setPathForSong(songID, "");
    }
}
