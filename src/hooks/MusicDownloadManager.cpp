#include <Geode/Geode.hpp>
#include <Geode/binding/MusicDownloadManager.hpp>
#include <Geode/modify/MusicDownloadManager.hpp>

#include <filesystem>
#include <system_error>

#include "../core/LocalLibrary.hpp"

using namespace geode::prelude;

class $modify(GFNMusicDownloadManager, MusicDownloadManager) {
    gd::string pathForSong(int songID) {
        if (!Mod::get()->getSettingValue<bool>("enabled")) {
            return MusicDownloadManager::pathForSong(songID);
        }

        auto replacement = gfn::local_library::pathForSong(songID);
        if (replacement.empty()) {
            return MusicDownloadManager::pathForSong(songID);
        }

        std::error_code error;
        const auto exists = std::filesystem::is_regular_file(replacement, error);
        if (error || !exists) {
            log::warn(
                "Selected NONG for song {} is unavailable; using the original song.",
                songID
            );
            gfn::local_library::clearPathForSong(songID);
            return MusicDownloadManager::pathForSong(songID);
        }

        log::debug("Resolving Geometry Dash song {} to its local NONG.", songID);
        return gd::string(replacement);
    }
};
