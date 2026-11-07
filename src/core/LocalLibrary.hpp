#pragma once

#include <Geode/Geode.hpp>
#include <algorithm>
#include <filesystem>
#include <sstream>
#include <string>
#include <vector>

namespace gfn::local_library {
    inline constexpr char INDEX_KEY[] = "local-nong-song-ids";

    inline std::string keyForSong(int songID) {
        return fmt::format("local-nong-path-{}", songID);
    }

    inline std::vector<int> installedSongIDs() {
        auto mod = geode::Mod::get();
        if (!mod) return {};
        auto raw = mod->getSavedValue<std::string>(INDEX_KEY, "");
        std::vector<int> ids;
        std::stringstream stream(raw);
        std::string token;
        while (std::getline(stream, token, ',')) {
            try {
                size_t consumed = 0;
                int id = std::stoi(token, &consumed);
                if (consumed == token.size() && id > 0 &&
                    std::find(ids.begin(), ids.end(), id) == ids.end()) {
                    ids.push_back(id);
                }
            } catch (...) {
                // Ignore malformed legacy index entries.
            }
        }
        return ids;
    }

    inline void saveInstalledSongIDs(std::vector<int> const& ids) {
        auto mod = geode::Mod::get();
        if (!mod) return;
        std::string raw;
        for (int id : ids) {
            if (id <= 0) continue;
            if (!raw.empty()) raw += ',';
            raw += std::to_string(id);
        }
        mod->setSavedValue(INDEX_KEY, raw);
    }

    // An empty path means the game should use its normal song file.
    inline std::string pathForSong(int songID) {
        auto mod = geode::Mod::get();
        if (!mod || songID <= 0) return {};
        auto path = mod->getSavedValue<std::string>(keyForSong(songID), "");
        // Migrate replacements saved by older mod versions into the index.
        if (!path.empty()) {
            auto ids = installedSongIDs();
            if (std::find(ids.begin(), ids.end(), songID) == ids.end()) {
                ids.push_back(songID);
                saveInstalledSongIDs(ids);
            }
        }
        return path;
    }

    inline uintmax_t fileSizeForSong(int songID) {
        auto path = pathForSong(songID);
        if (path.empty()) return 0;
        std::error_code error;
        auto size = std::filesystem::file_size(path, error);
        return error ? 0 : size;
    }

    inline void setPathForSong(int songID, std::string const& path) {
        auto mod = geode::Mod::get();
        if (!mod || songID <= 0) return;
        mod->setSavedValue(keyForSong(songID), path);
        auto ids = installedSongIDs();
        if (!path.empty() && std::find(ids.begin(), ids.end(), songID) == ids.end()) {
            ids.push_back(songID);
            saveInstalledSongIDs(ids);
        }
    }

    inline bool deleteManagedFileForSong(int songID) {
        auto mod = geode::Mod::get();
        if (!mod || songID <= 0) return false;
        const auto rawPath = pathForSong(songID);
        if (rawPath.empty()) return false;

        std::error_code error;
        const auto saveDir = std::filesystem::canonical(mod->getSaveDir(), error);
        if (error) return false;
        const auto file = std::filesystem::canonical(rawPath, error);
        if (error || file.parent_path() != saveDir) return false;

        const auto expectedPrefix = fmt::format("song-{}.", songID);
        const auto filename = file.filename().string();
        if (filename.rfind(expectedPrefix, 0) != 0) return false;
        const auto extension = file.extension().string();
        if (extension != ".mp3" && extension != ".ogg" && extension != ".wav" &&
            extension != ".flac" && extension != ".m4a") return false;

        const bool removed = std::filesystem::remove(file, error);
        if (error || !removed) return false;
        clearPathForSong(songID);
        return true;
    }

    inline void clearPathForSong(int songID) {
        auto mod = geode::Mod::get();
        if (!mod || songID <= 0) return;
        mod->setSavedValue(keyForSong(songID), "");
        auto ids = installedSongIDs();
        ids.erase(std::remove(ids.begin(), ids.end(), songID), ids.end());
        saveInstalledSongIDs(ids);
    }
}
