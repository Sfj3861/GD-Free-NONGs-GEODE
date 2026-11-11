#pragma once

#include <Geode/Geode.hpp>
#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <sstream>
#include <string>
#include <vector>

namespace gfn::local_library {
    inline constexpr char INDEX_KEY[] = "local-nong-song-ids";
    inline constexpr char DISABLED_INDEX_KEY[] = "local-nong-disabled-song-ids";

    inline std::string keyForSong(int songID) {
        return fmt::format("local-nong-path-{}", songID);
    }

    inline bool supportedAudioExtension(std::string const& extension) {
        return extension == ".mp3" || extension == ".ogg" || extension == ".wav" ||
            extension == ".flac" || extension == ".m4a";
    }

    inline std::vector<int> indexedSongIDs() {
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

    inline std::vector<int> disabledSongIDs() {
        auto mod = geode::Mod::get();
        if (!mod) return {};
        const auto raw = mod->getSavedValue<std::string>(DISABLED_INDEX_KEY, "");
        std::vector<int> ids;
        std::stringstream stream(raw);
        std::string token;
        while (std::getline(stream, token, ',')) {
            try {
                size_t consumed = 0;
                const int id = std::stoi(token, &consumed);
                if (consumed == token.size() && id > 0 &&
                    std::find(ids.begin(), ids.end(), id) == ids.end()) {
                    ids.push_back(id);
                }
            } catch (...) {
                // Ignore malformed disabled-index entries.
            }
        }
        return ids;
    }

    inline void saveDisabledSongIDs(std::vector<int> const& ids) {
        auto mod = geode::Mod::get();
        if (!mod) return;
        std::string raw;
        for (int id : ids) {
            if (id <= 0) continue;
            if (!raw.empty()) raw += ',';
            raw += std::to_string(id);
        }
        mod->setSavedValue(DISABLED_INDEX_KEY, raw);
    }

    inline std::vector<int> installedSongIDs() {
        auto mod = geode::Mod::get();
        if (!mod) return {};
        auto ids = indexedSongIDs();
        auto disabled = disabledSongIDs();
        ids.erase(std::remove_if(ids.begin(), ids.end(), [&](int id) {
            return std::find(disabled.begin(), disabled.end(), id) != disabled.end();
        }), ids.end());

        // Discover exported song-ID files copied into the mod's save directory.
        std::error_code scanError;
        const auto saveDir = mod->getSaveDir().string();
        for (std::filesystem::directory_iterator it(saveDir, scanError), end;
             !scanError && it != end; it.increment(scanError)) {
            std::error_code fileError;
            if (!it->is_regular_file(fileError) || fileError) continue;
            const auto filename = it->path().filename().string();
            if (filename.rfind("song-", 0) != 0) continue;
            const auto dot = filename.find('.', 5);
            if (dot == std::string::npos) continue;
            const auto extension = filename.substr(dot);
            if (!supportedAudioExtension(extension)) continue;

            try {
                const auto idText = filename.substr(5, dot - 5);
                size_t consumed = 0;
                const int id = std::stoi(idText, &consumed);
                if (consumed != idText.size() || id <= 0 ||
                    std::find(disabled.begin(), disabled.end(), id) != disabled.end()) continue;

                const auto candidate = it->path().string();
                const auto registered = mod->getSavedValue<std::string>(keyForSong(id), "");

                if (!registered.empty()) {
                    std::error_code registeredError;
                    const bool registeredExists =
                        std::filesystem::is_regular_file(registered, registeredError);

                    // Do not silently select an older alternate-format file when the
                    // previously active file has disappeared. A filesystem error is
                    // inconclusive, so preserve the registration and try again later.
                    if (registeredError) continue;
                    if (!registeredExists) {
                        if (std::find(disabled.begin(), disabled.end(), id) == disabled.end()) {
                            disabled.push_back(id);
                            saveDisabledSongIDs(disabled);
                        }
                        mod->setSavedValue(keyForSong(id), std::string{});
                        ids.erase(std::remove(ids.begin(), ids.end(), id), ids.end());
                        continue;
                    }
                } else {
                    mod->setSavedValue(keyForSong(id), candidate);
                }

                const auto activePath = mod->getSavedValue<std::string>(keyForSong(id), "");
                if (activePath == candidate &&
                    std::find(ids.begin(), ids.end(), id) == ids.end()) {
                    ids.push_back(id);
                }
            } catch (...) {
                // Ignore unrelated or malformed files in the mod save folder.
            }
        }
        std::sort(ids.begin(), ids.end());
        saveInstalledSongIDs(ids);
        return ids;
    }

    // An empty path means the game should use its normal song file.
    inline std::string pathForSong(int songID) {
        auto mod = geode::Mod::get();
        if (!mod || songID <= 0) return {};
        return mod->getSavedValue<std::string>(keyForSong(songID), "");
    }

    inline std::uintmax_t fileSizeForSong(int songID) {
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
        auto disabled = disabledSongIDs();
        disabled.erase(std::remove(disabled.begin(), disabled.end(), songID), disabled.end());
        saveDisabledSongIDs(disabled);
        auto ids = indexedSongIDs();
        if (!path.empty() && std::find(ids.begin(), ids.end(), songID) == ids.end()) {
            ids.push_back(songID);
            saveInstalledSongIDs(ids);
        }
    }

    inline void clearPathForSong(int songID) {
        auto mod = geode::Mod::get();
        if (!mod || songID <= 0) return;
        mod->setSavedValue(keyForSong(songID), std::string{});
        auto ids = indexedSongIDs();
        ids.erase(std::remove(ids.begin(), ids.end(), songID), ids.end());
        saveInstalledSongIDs(ids);
    }

    inline void disableSongForSong(int songID) {
        auto mod = geode::Mod::get();
        if (!mod || songID <= 0) return;

        // Keep stale alternate-format files from being rediscovered after the active file is gone.
        auto disabled = disabledSongIDs();
        if (std::find(disabled.begin(), disabled.end(), songID) == disabled.end()) {
            disabled.push_back(songID);
            saveDisabledSongIDs(disabled);
        }
        clearPathForSong(songID);
    }

    inline bool deleteManagedFileForSong(int songID) {
        auto mod = geode::Mod::get();
        if (!mod || songID <= 0) return false;
        const auto rawPath = pathForSong(songID);
        if (rawPath.empty()) return false;

        std::error_code error;
        const auto saveDir = std::filesystem::canonical(mod->getSaveDir().string(), error);
        if (error) return false;
        const auto file = std::filesystem::canonical(rawPath, error);
        if (error || file.parent_path() != saveDir) return false;

        const auto expectedPrefix = fmt::format("song-{}.", songID);
        const auto filename = file.filename().string();
        if (filename.rfind(expectedPrefix, 0) != 0 ||
            !supportedAudioExtension(file.extension().string())) return false;

        const bool removed = std::filesystem::remove(file, error);
        if (error || !removed) return false;
        disableSongForSong(songID);
        return true;
    }
}