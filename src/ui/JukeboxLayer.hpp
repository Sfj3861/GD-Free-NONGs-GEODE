#pragma once

#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>
#include <Geode/utils/crypto.hpp>
#include <Geode/ui/TextInput.hpp>
#include "../core/LocalLibrary.hpp"
#include <chrono>
#include <cstring>
#include <algorithm>
#include <memory>
#include <utility>
#include <random>
#include <span>
#include <string>
#include <string_view>
#include <vector>

class GFNJukeboxLayer final : public cocos2d::CCLayerColor {
    struct SongEntry {
        std::string id;
        std::string name;
        int nongs = 0;
    };
    struct AsyncRelease { GFNJukeboxLayer* layer; ~AsyncRelease() { layer->release(); } };

    struct NongEntry {
        int id = 0;
        std::string name;
        std::string format;
        std::string sha256;
    };

    struct RemoteSelection {
        int songId = 0;
        int nongId = 0;
        bool original = false;
        bool unavailable = false;
        std::string format;
        std::string sha256;
    };

    std::string m_baseUrl;
    std::string m_deviceId;
    std::string m_currentSongId;
    std::vector<SongEntry> m_songs;
    std::vector<NongEntry> m_nongs;
    int m_catalogPage = 1;
    int m_catalogPages = 0;
    int m_nongPage = 1;
    int m_nongPages = 0;
    std::vector<int> m_librarySongIds;
    int m_libraryPage = 1;
    static constexpr size_t LIBRARY_PAGE_SIZE = 7;
    std::string m_searchQuery;
    geode::TextInput* m_searchInput = nullptr;
    cocos2d::CCLabelTTF* m_status = nullptr;
    cocos2d::CCMenu* m_menu = nullptr;

    static std::string urlEncode(std::string_view value) {
        std::string encoded;
        encoded.reserve(value.size());
        for (unsigned char byte : value) {
            const bool unreserved =
                (byte >= 'A' && byte <= 'Z') ||
                (byte >= 'a' && byte <= 'z') ||
                (byte >= '0' && byte <= '9') ||
                byte == '-' || byte == '_' || byte == '.' || byte == '~';
            if (unreserved) {
                encoded.push_back(static_cast<char>(byte));
            } else {
                encoded += fmt::format("%{:02X}", static_cast<unsigned int>(byte));
            }
        }
        return encoded;
    }

    static bool validAudioPayload(std::string_view format, std::span<uint8_t const> bytes) {
        constexpr size_t MAX_AUDIO_BYTES = 30u * 1024u * 1024u;
        if (bytes.empty() || bytes.size() > MAX_AUDIO_BYTES) return false;

        if (format == "mp3") {
            const bool id3 = bytes.size() >= 3 && std::memcmp(bytes.data(), "ID3", 3) == 0;
            const bool frame = bytes.size() >= 2 && bytes[0] == 0xFF &&
                (bytes[1] & 0xE0) == 0xE0;
            return id3 || frame;
        }
        if (format == "ogg") {
            return bytes.size() >= 4 && std::memcmp(bytes.data(), "OggS", 4) == 0;
        }
        if (format == "wav") {
            return bytes.size() >= 12 &&
                std::memcmp(bytes.data(), "RIFF", 4) == 0 &&
                std::memcmp(bytes.data() + 8, "WAVE", 4) == 0;
        }
        if (format == "flac") {
            return bytes.size() >= 4 && std::memcmp(bytes.data(), "fLaC", 4) == 0;
        }
        if (format == "m4a") {
            return bytes.size() >= 8 && std::memcmp(bytes.data() + 4, "ftyp", 4) == 0;
        }
        return false;
    }

    void setStatus(std::string const& value) {
        if (m_status) {
            m_status->setString(value.c_str());
        }
    }

    bool configureServer() {
        auto mod = geode::Mod::get();
        m_baseUrl = mod ? mod->getSettingValue<std::string>("api-url") : "";
        while (!m_baseUrl.empty() && m_baseUrl.back() == '/') m_baseUrl.pop_back();
        if (m_baseUrl.empty()) {
            setStatus("Set the API URL in mod settings.");
            return false;
        }
        auto isLocalHttp = [](std::string const& url, std::string_view host) {
            if (url.rfind(host, 0) != 0) return false;
            if (url.size() == host.size()) return true;
            const char boundary = url[host.size()];
            return boundary == ':' || boundary == '/';
        };
        const bool secureRemote = m_baseUrl.rfind("https://", 0) == 0;
        const bool localDevelopment =
            isLocalHttp(m_baseUrl, "http://localhost") ||
            isLocalHttp(m_baseUrl, "http://127.0.0.1");
        if (!secureRemote && !localDevelopment) {
            setStatus("Remote API URLs must use HTTPS.");
            return false;
        }
        if (!mod) return false;
        m_deviceId = mod->getSavedValue<std::string>("device-id", "");
        if (m_deviceId.empty()) {
            // A persistent random identifier distinguishes this installation.
            m_deviceId = fmt::format("{:016x}{:016x}",
                static_cast<uint64_t>(std::chrono::high_resolution_clock::now().time_since_epoch().count()),
                static_cast<uint64_t>(std::random_device{}()) << 32 | std::random_device{}());
            mod->setSavedValue("device-id", m_deviceId);
        }
        return true;
    }

    void clearMenu() {
        if (m_menu) {
            m_menu->removeFromParentAndCleanup(true);
            m_menu = nullptr;
        }
    }

    void showMenu() {
        clearMenu();
        m_menu = cocos2d::CCMenu::create();
        if (!m_menu) return;
        auto size = cocos2d::CCDirector::sharedDirector()->getWinSize();
        m_menu->setPosition({ size.width / 2, size.height * 0.47f });
        this->addChild(m_menu, 2);
    }

    void showSongs() {
        showMenu();
        setStatus(fmt::format("Community song catalogue · Page {} / {}",
            m_catalogPage, std::max(m_catalogPages, 1)));
        if (m_songs.empty()) {
            setStatus("No songs found in the community catalogue.");
        }

        auto count = std::min<size_t>(m_songs.size(), 7);
        for (size_t i = 0; i < count; ++i) {
            auto label = cocos2d::CCLabelTTF::create(
                fmt::format("{}  ({} NONGs)", m_songs[i].name, m_songs[i].nongs).c_str(),
                "bigFont.fnt", 0.42f
            );
            if (!label) continue;
            auto item = cocos2d::CCMenuItemLabel::create(
                label, this, menu_selector(GFNJukeboxLayer::onSongSelected)
            );
            if (!item) continue;
            item->setTag(static_cast<int>(i));
            item->setPosition({ 0.f, 105.f - static_cast<float>(i) * thirtyFive() });
            m_menu->addChild(item);
        }

        if (m_catalogPage > 1) {
            auto label = cocos2d::CCLabelTTF::create("← Previous", "bigFont.fnt", 0.34f);
            auto button = label ? cocos2d::CCMenuItemLabel::create(
                label, this, menu_selector(GFNJukeboxLayer::onPreviousPage)
            ) : nullptr;
            if (button) {
                button->setPosition({ -85.f, -150.f });
                m_menu->addChild(button);
            }
        }

        if (m_catalogPage < m_catalogPages) {
            auto label = cocos2d::CCLabelTTF::create("Next →", "bigFont.fnt", 0.34f);
            auto button = label ? cocos2d::CCMenuItemLabel::create(
                label, this, menu_selector(GFNJukeboxLayer::onNextPage)
            ) : nullptr;
            if (button) {
                button->setPosition({ 85.f, -150.f });
                m_menu->addChild(button);
            }
        }
    }

    static constexpr float thirtyFive() { return 35.f; }

    void loadSongs(int page = 1) {
        if (!configureServer()) return;
        page = std::max(page, 1);
        setStatus("Loading community catalogue...");
        auto url = fmt::format(
            "{}/api/v1/catalog/songs?page={}&limit=7&q={}",
            m_baseUrl, page, urlEncode(m_searchQuery)
        );
        this->retain();
        geode::async::spawn(
            geode::utils::web::WebRequest().timeout(std::chrono::seconds(15)).get(url),
            [this](geode::utils::web::WebResponse response) {
                AsyncRelease hold{ this };
                if (!this->getParent()) return;
                if (!response.ok()) {
                    setStatus(fmt::format("Catalogue request failed (HTTP {}).", response.code()));
                    return;
                }
                auto parsed = response.json();
                if (!parsed || !parsed.unwrap().isObject() ||
                    !parsed.unwrap()["items"].isArray() ||
                    !parsed.unwrap()["pagination"].isObject()) {
                    setStatus("The server returned an invalid catalogue.");
                    return;
                }

                auto const& data = parsed.unwrap();
                auto rawPage = data["pagination"]["page"].asInt().unwrapOr(1);
                auto rawPages = data["pagination"]["pages"].asInt().unwrapOr(0);
                m_catalogPage = static_cast<int>(std::clamp<int64_t>(rawPage, 1, 1000000));
                m_catalogPages = static_cast<int>(std::clamp<int64_t>(rawPages, 0, 1000000));

                m_songs.clear();
                for (auto const& row : data["items"]) {
                    auto id = row["gd_song_id"].asString().unwrapOr("");
                    auto name = row["name"].asString().unwrapOr("Unknown song");
                    auto rawCount = row["nongs"].asInt().unwrapOr(0);
                    auto count = static_cast<int>(std::clamp<int64_t>(rawCount, 0, 1000000));
                    if (!id.empty() && id.find_first_not_of("0123456789") == std::string::npos) {
                        m_songs.push_back({ id, name, count });
                    }
                }
                showSongs();
            }
        );
    }

    void loadNongs(std::string songId, int page = 1) {
        m_currentSongId = std::move(songId);
        m_nongPage = std::max(page, 1);
        setStatus("Loading available NONGs...");
        showMenu();
        auto url = fmt::format(
            "{}/api/v1/songs/{}/nongs?page={}&limit=7",
            m_baseUrl, m_currentSongId, m_nongPage
        );
        this->retain();
        geode::async::spawn(
            geode::utils::web::WebRequest().timeout(std::chrono::seconds(15)).get(url),
            [this](geode::utils::web::WebResponse response) {
                AsyncRelease hold{ this };
                if (!this->getParent()) return;
                if (!response.ok()) {
                    setStatus(fmt::format("NONG list request failed (HTTP {}).", response.code()));
                    return;
                }
                auto parsed = response.json();
                if (!parsed || !parsed.unwrap().isObject() ||
                    !parsed.unwrap()["items"].isArray() ||
                    !parsed.unwrap()["pagination"].isObject()) {
                    setStatus("The server returned an invalid paginated NONG list.");
                    return;
                }
                auto const& data = parsed.unwrap();
                auto rawPage = data["pagination"]["page"].asInt().unwrapOr(1);
                auto rawPages = data["pagination"]["pages"].asInt().unwrapOr(0);
                m_nongPage = static_cast<int>(std::clamp<int64_t>(rawPage, 1, 1000000));
                m_nongPages = static_cast<int>(std::clamp<int64_t>(rawPages, 0, 1000000));
                m_nongs.clear();
                for (auto const& row : data["items"]) {
                    NongEntry entry;
                    entry.id = row["id"].asInt().unwrapOr(0);
                    entry.name = row["display_name"].asString().unwrapOr("Community version");
                    entry.format = row["format"].asString().unwrapOr("");
                    entry.sha256 = row["sha256"].asString().unwrapOr("");
                    if (entry.id > 0 && (entry.format == "mp3" || entry.format == "ogg" ||
                        entry.format == "wav" || entry.format == "flac" || entry.format == "m4a")) {
                        m_nongs.push_back(std::move(entry));
                    }
                }
                showNongs();
            }
        );
    }

    void showNongs() {
        showMenu();
        if (m_nongs.empty()) {
            setStatus("No approved NONGs for this song yet.");
            return;
        }
        setStatus(fmt::format(
            "Choose a community version · Page {} / {}",
            m_nongPage, std::max(m_nongPages, 1)
        ));
        auto count = std::min<size_t>(m_nongs.size(), 7);
        for (size_t i = 0; i < count; ++i) {
            auto label = cocos2d::CCLabelTTF::create(m_nongs[i].name.c_str(), "bigFont.fnt", 0.42f);
            if (!label) continue;
            auto item = cocos2d::CCMenuItemLabel::create(
                label, this, menu_selector(GFNJukeboxLayer::onNongSelected)
            );
            if (!item) continue;
            item->setTag(static_cast<int>(i));
            item->setPosition({ 0.f, 105.f - static_cast<float>(i) * thirtyFive() });
            m_menu->addChild(item);
        }
        if (m_nongPage > 1) {
            auto label = cocos2d::CCLabelTTF::create("← Previous", "bigFont.fnt", 0.32f);
            auto button = label ? cocos2d::CCMenuItemLabel::create(
                label, this, menu_selector(GFNJukeboxLayer::onPreviousNongPage)
            ) : nullptr;
            if (button) {
                button->setPosition({ -85.f, -125.f });
                m_menu->addChild(button);
            }
        }
        if (m_nongPage < m_nongPages) {
            auto label = cocos2d::CCLabelTTF::create("Next →", "bigFont.fnt", 0.32f);
            auto button = label ? cocos2d::CCMenuItemLabel::create(
                label, this, menu_selector(GFNJukeboxLayer::onNextNongPage)
            ) : nullptr;
            if (button) {
                button->setPosition({ 85.f, -125.f });
                m_menu->addChild(button);
            }
        }
        auto originalLabel = cocos2d::CCLabelTTF::create("Use original song", "bigFont.fnt", 0.38f);
        auto original = originalLabel ? cocos2d::CCMenuItemLabel::create(
            originalLabel, this, menu_selector(GFNJukeboxLayer::onUseOriginal)
        ) : nullptr;
        if (original) {
            original->setPosition({ 0.f, -150.f });
            m_menu->addChild(original);
        }
        auto backLabel = cocos2d::CCLabelTTF::create("Back to songs", "bigFont.fnt", 0.38f);
        auto back = backLabel ? cocos2d::CCMenuItemLabel::create(
            backLabel, this, menu_selector(GFNJukeboxLayer::onBackToSongs)
        ) : nullptr;
        if (back) {
            back->setPosition({ 0.f, -175.f });
            m_menu->addChild(back);
        }
    }

    std::string formatFileSize(uintmax_t bytes) const {
        if (bytes < 1024) return fmt::format("{} B", bytes);
        if (bytes < 1024 * 1024) return fmt::format("{:.1f} KiB", bytes / 1024.0);
        return fmt::format("{:.1f} MiB", bytes / (1024.0 * 1024.0));
    }

    void showLocalLibrary() {
        m_librarySongIds = gfn::local_library::installedSongIDs();
        const int pageCount = std::max<int>(1, static_cast<int>((m_librarySongIds.size() + LIBRARY_PAGE_SIZE - 1) / LIBRARY_PAGE_SIZE));
        m_libraryPage = std::clamp(m_libraryPage, 1, pageCount);
        showMenu();
        setStatus(m_librarySongIds.empty()
            ? "No local NONGs are registered on this device."
            : fmt::format("Local NONG library · {} songs · Page {} / {}", m_librarySongIds.size(), m_libraryPage, pageCount));

        const size_t start = static_cast<size_t>(m_libraryPage - 1) * LIBRARY_PAGE_SIZE;
        const size_t end = std::min(m_librarySongIds.size(), start + LIBRARY_PAGE_SIZE);
        for (size_t i = start; i < end; ++i) {
            const int songId = m_librarySongIds[i];
            const auto path = gfn::local_library::pathForSong(songId);
            const bool exists = !path.empty() && std::filesystem::exists(path);
            const auto filename = path.empty() ? std::string("No file registered") : std::filesystem::path(path).filename().string();
            const auto size = exists ? formatFileSize(gfn::local_library::fileSizeForSong(songId)) : std::string("Missing file");
            auto label = cocos2d::CCLabelTTF::create(fmt::format("Song {} · {} · {}", songId, filename, size).c_str(), "bigFont.fnt", 0.34f);
            if (!label) continue;
            auto item = cocos2d::CCMenuItemLabel::create(label, this, menu_selector(GFNJukeboxLayer::onRestoreLocalSong));
            if (!item) continue;
            item->setTag(songId);
            const float rowY = 105.f - static_cast<float>(i - start) * thirtyFive();
            item->setPosition({ -35.f, rowY });
            m_menu->addChild(item);

            auto deleteLabel = cocos2d::CCLabelTTF::create("Delete", "bigFont.fnt", 0.30f);
            auto deleteButton = deleteLabel ? cocos2d::CCMenuItemLabel::create(
                deleteLabel, this, menu_selector(GFNJukeboxLayer::onDeleteLocalFile)
            ) : nullptr;
            if (deleteButton) {
                deleteButton->setTag(songId);
                deleteButton->setPosition({ 175.f, rowY });
                m_menu->addChild(deleteButton);
            }
        }

        if (m_libraryPage > 1) {
            auto label = cocos2d::CCLabelTTF::create("← Previous", "bigFont.fnt", 0.32f);
            auto button = label ? cocos2d::CCMenuItemLabel::create(label, this, menu_selector(GFNJukeboxLayer::onPreviousLibraryPage)) : nullptr;
            if (button) { button->setPosition({ -85.f, -145.f }); m_menu->addChild(button); }
        }
        if (m_libraryPage < pageCount) {
            auto label = cocos2d::CCLabelTTF::create("Next →", "bigFont.fnt", 0.32f);
            auto button = label ? cocos2d::CCMenuItemLabel::create(label, this, menu_selector(GFNJukeboxLayer::onNextLibraryPage)) : nullptr;
            if (button) { button->setPosition({ 85.f, -145.f }); m_menu->addChild(button); }
        }
        auto backLabel = cocos2d::CCLabelTTF::create("Back to catalogue", "bigFont.fnt", 0.36f);
        auto back = backLabel ? cocos2d::CCMenuItemLabel::create(backLabel, this, menu_selector(GFNJukeboxLayer::onBackFromLibrary)) : nullptr;
        if (back) { back->setPosition({ 0.f, -175.f }); m_menu->addChild(back); }
    }

    void requestPairingCode() {
        if (!configureServer()) return;

        auto body = fmt::format("{{\"device_id\":\"{}\"}}", m_deviceId);
        auto request = geode::utils::web::WebRequest();
        request.header("Content-Type", "application/json")
            .timeout(std::chrono::seconds(20))
            .bodyString(body);
        setStatus("Requesting one-time website pairing code...");
        this->retain();
        geode::async::spawn(
            request.post(m_baseUrl + "/api/v1/pairing/request"),
            [this](geode::utils::web::WebResponse response) {
                AsyncRelease hold{ this };
                if (!this->getParent()) return;
                if (!response.ok()) {
                    setStatus(fmt::format("Pairing request failed (HTTP {}).", response.code()));
                    return;
                }
                auto parsed = response.json();
                if (!parsed || !parsed.unwrap().isObject()) {
                    setStatus("The server returned an invalid pairing response.");
                    return;
                }
                auto code = parsed.unwrap()["code"].asString().unwrapOr("");
                if (code.size() != 16) {
                    setStatus("The server returned an invalid pairing code.");
                    return;
                }
                setStatus(fmt::format(
                    "PAIR CODE: {} — enter it in the website PAIR tab. Expires in 10 minutes.",
                    code
                ));
            }
        );
    }

    void syncNext(
        std::shared_ptr<std::vector<RemoteSelection>> const& entries,
        size_t index,
        size_t installed,
        size_t cleared,
        size_t failed
    ) {
        // Apply original/unavailable selections locally and locate the next
        // downloadable NONG before starting another asynchronous request.
        while (index < entries->size()) {
            auto const& item = (*entries)[index];
            if (item.songId <= 0) {
                ++failed;
                ++index;
                continue;
            }
            if (item.original || item.unavailable) {
                gfn::local_library::clearPathForSong(item.songId);
                ++cleared;
                ++index;
                continue;
            }
            break;
        }

        if (index >= entries->size()) {
            setStatus(fmt::format(
                "Website sync complete · {} installed, {} reset, {} failed.",
                installed, cleared, failed
            ));
            return;
        }

        auto const item = (*entries)[index];
        if (item.nongId <= 0 ||
            (item.format != "mp3" && item.format != "ogg" &&
             item.format != "wav" && item.format != "flac" &&
             item.format != "m4a") ||
            item.sha256.size() != 64) {
            syncNext(entries, index + 1, installed, cleared, failed + 1);
            return;
        }

        // Remove an older replacement first. If the remote download or
        // integrity check fails, the game's original track remains the fallback.
        gfn::local_library::clearPathForSong(item.songId);
        setStatus(fmt::format(
            "Syncing website selection {}/{}...",
            index + 1, entries->size()
        ));
        auto url = fmt::format(
            "{}/api/v1/nongs/{}/download", m_baseUrl, item.nongId
        );
        this->retain();
        geode::async::spawn(
            geode::utils::web::WebRequest().timeout(std::chrono::seconds(90)).get(url),
            [this, entries, index, installed, cleared, failed, item](geode::utils::web::WebResponse audio) {
                AsyncRelease hold{ this };
                if (!this->getParent()) return;
                if (!audio.ok()) {
                    syncNext(entries, index + 1, installed, cleared, failed + 1);
                    return;
                }

                auto audioBytes = std::span<uint8_t const>(
                    audio.data().data(), audio.data().size()
                );
                if (!validAudioPayload(item.format, audioBytes)) {
                    syncNext(entries, index + 1, installed, cleared, failed + 1);
                    return;
                }
                auto actualHash = geode::sha256(audioBytes).toString();
                if (actualHash != item.sha256) {
                    syncNext(entries, index + 1, installed, cleared, failed + 1);
                    return;
                }

                auto mod = geode::Mod::get();
                if (!mod) {
                    syncNext(entries, index + 1, installed, cleared, failed + 1);
                    return;
                }
                auto target = mod->getSaveDir() /
                    fmt::format("song-{}.{}", item.songId, item.format);
                auto written = audio.into(target);
                if (!written) {
                    syncNext(entries, index + 1, installed, cleared, failed + 1);
                    return;
                }

                gfn::local_library::setPathForSong(item.songId, target.string());
                syncNext(entries, index + 1, installed + 1, cleared, failed);
            }
        );
    }

    void syncWebsiteSelections() {
        if (!configureServer()) return;
        setStatus("Checking selections saved from the website...");
        const auto url = m_baseUrl + "/api/v1/sync/" + m_deviceId;
        this->retain();
        geode::async::spawn(
            geode::utils::web::WebRequest().timeout(std::chrono::seconds(25)).get(url),
            [this](geode::utils::web::WebResponse response) {
                AsyncRelease hold{ this };
                if (!this->getParent()) return;
                if (!response.ok()) {
                    setStatus(fmt::format("Website sync failed (HTTP {}).", response.code()));
                    return;
                }
                auto parsed = response.json();
                if (!parsed || !parsed.unwrap().isObject() ||
                    !parsed.unwrap()["selections"].isArray()) {
                    setStatus("The server returned an invalid sync response.");
                    return;
                }

                auto entries = std::make_shared<std::vector<RemoteSelection>>();
                for (auto const& row : parsed.unwrap()["selections"]) {
                    RemoteSelection item;
                    auto songText = row["gd_song_id"].asString().unwrapOr("");
                    if (!songText.empty() &&
                        songText.find_first_not_of("0123456789") == std::string::npos) {
                        try {
                            item.songId = std::stoi(songText);
                        } catch (...) {
                            item.songId = 0;
                        }
                    }
                    item.nongId = row["nong_id"].asInt().unwrapOr(0);
                    item.original = row["is_original"].asBool().unwrapOr(false);
                    item.unavailable = row["status"].asString().unwrapOr("") == "unavailable";
                    item.format = row["format"].asString().unwrapOr("");
                    item.sha256 = row["sha256"].asString().unwrapOr("");
                    entries->push_back(std::move(item));
                }

                if (entries->empty()) {
                    setStatus("No website selections are linked to this game device yet.");
                    return;
                }
                syncNext(entries, 0, 0, 0, 0);
            }
        );
    }

    void saveSelectionAndDownload(NongEntry entry) {
        if (m_currentSongId.empty()) return;
        int numericSongId = 0;
        try {
            numericSongId = std::stoi(m_currentSongId);
        } catch (...) {
            setStatus("This song ID is outside the supported local ID range.");
            return;
        }
        const auto songId = m_currentSongId;
        const auto baseUrl = m_baseUrl;
        setStatus("Downloading and verifying NONG audio...");
        const auto downloadUrl = baseUrl + "/api/v1/nongs/" + std::to_string(entry.id) + "/download";
        this->retain();
        geode::async::spawn(
            geode::utils::web::WebRequest().timeout(std::chrono::seconds(90)).get(downloadUrl),
            [this, entry, songId, numericSongId, baseUrl](geode::utils::web::WebResponse audio) {
                AsyncRelease hold{ this };
                if (!this->getParent()) return;
                if (!audio.ok()) {
                    setStatus(fmt::format("Download failed (HTTP {}).", audio.code()));
                    return;
                }
                auto audioBytes = std::span<uint8_t const>(
                    audio.data().data(), audio.data().size()
                );
                if (!validAudioPayload(entry.format, audioBytes)) {
                    setStatus("Download rejected: invalid audio container or file exceeds 30 MiB.");
                    return;
                }
                auto actualHash = geode::sha256(audioBytes).toString();
                if (actualHash != entry.sha256) {
                    setStatus("Download rejected: SHA-256 checksum mismatch.");
                    return;
                }
                auto mod = geode::Mod::get();
                if (!mod) return;
                auto filename = fmt::format("song-{}.{}", songId, entry.format);
                auto target = mod->getSaveDir() / filename;
                auto written = audio.into(target);
                if (!written) {
                    setStatus("Could not write the downloaded audio file.");
                    return;
                }

                auto body = fmt::format(
                    "{{\"gd_song_id\":\"{}\",\"nong_id\":{},\"device_id\":\"{}\"}}",
                    songId, entry.id, m_deviceId
                );
                auto request = geode::utils::web::WebRequest();
                request.header("Content-Type", "application/json")
                    .timeout(std::chrono::seconds(20))
                    .bodyString(body);
                setStatus("Audio verified. Saving your selection...");
                this->retain();
                geode::async::spawn(
                    request.post(baseUrl + "/api/v1/selections/save"),
                    [this, numericSongId, target](geode::utils::web::WebResponse saved) {
                        AsyncRelease hold{ this };
                        if (!this->getParent()) return;
                        if (!saved.ok()) {
                            setStatus(fmt::format("Audio downloaded, but selection save failed (HTTP {}).", saved.code()));
                            return;
                        }
                        gfn::local_library::setPathForSong(numericSongId, target.string());
                        setStatus("NONG installed and selected for this song!");
                    }
                );
            }
        );
    }

public:
    static GFNJukeboxLayer* create() {
        auto layer = new GFNJukeboxLayer();
        if (layer && layer->init()) {
            layer->autorelease();
            return layer;
        }
        CC_SAFE_DELETE(layer);
        return nullptr;
    }

    bool init() override {
        if (!CCLayerColor::initWithColor(cocos2d::ccc4(12, 16, 26, 238))) return false;
        auto size = cocos2d::CCDirector::sharedDirector()->getWinSize();
        auto title = cocos2d::CCLabelTTF::create("GD Free NONGs", "bigFont.fnt", 0.8f);
        if (title) {
            title->setPosition({ size.width / 2, size.height * 0.84f });
            this->addChild(title);
        }
        m_status = cocos2d::CCLabelTTF::create("Press Refresh to load the community library.", "goldFont.fnt", 0.42f);
        if (m_status) {
            m_status->setPosition({ size.width / 2, size.height * 0.72f });
            m_status->setDimensions({ size.width * 0.85f, 42.f });
            this->addChild(m_status);
        }

        m_searchInput = geode::TextInput::create(size.width * 0.48f, "Search song name or GD song ID");
        if (m_searchInput) {
            m_searchInput->setMaxCharCount(100);
            m_searchInput->setTextAlign(geode::TextInputAlign::Center);
            m_searchInput->setScale(0.75f);
            m_searchInput->setPosition({ size.width * 0.39f, size.height * 0.59f });
            this->addChild(m_searchInput, 3);
        }

        auto searchLabel = cocos2d::CCLabelTTF::create("Search", "bigFont.fnt", 0.40f);
        auto searchButton = searchLabel ? cocos2d::CCMenuItemLabel::create(
            searchLabel, this, menu_selector(GFNJukeboxLayer::onSearch)
        ) : nullptr;
        auto clearLabel = cocos2d::CCLabelTTF::create("Clear", "bigFont.fnt", 0.36f);
        auto clearButton = clearLabel ? cocos2d::CCMenuItemLabel::create(
            clearLabel, this, menu_selector(GFNJukeboxLayer::onClearSearch)
        ) : nullptr;
        auto searchMenu = cocos2d::CCMenu::create();
        if (searchMenu) {
            if (searchButton) {
                searchButton->setPosition({ -28.f, 0.f });
                searchMenu->addChild(searchButton);
            }
            if (clearButton) {
                clearButton->setPosition({ 55.f, 0.f });
                searchMenu->addChild(clearButton);
            }
            searchMenu->setPosition({ size.width * 0.79f, size.height * 0.59f });
            this->addChild(searchMenu, 4);
        }

        auto libraryLabel = cocos2d::CCLabelTTF::create("My NONGs", "bigFont.fnt", 0.34f);
        auto library = libraryLabel ? cocos2d::CCMenuItemLabel::create(libraryLabel, this, menu_selector(GFNJukeboxLayer::onOpenLocalLibrary)) : nullptr;
        auto refreshLabel = cocos2d::CCLabelTTF::create("Refresh", "bigFont.fnt", 0.38f);
        auto refresh = refreshLabel ? cocos2d::CCMenuItemLabel::create(
            refreshLabel, this, menu_selector(GFNJukeboxLayer::onRefresh)
        ) : nullptr;
        auto pairLabel = cocos2d::CCLabelTTF::create("Pair website", "bigFont.fnt", 0.34f);
        auto pair = pairLabel ? cocos2d::CCMenuItemLabel::create(
            pairLabel, this, menu_selector(GFNJukeboxLayer::onPairWebsite)
        ) : nullptr;
        auto syncLabel = cocos2d::CCLabelTTF::create("Sync website", "bigFont.fnt", 0.34f);
        auto sync = syncLabel ? cocos2d::CCMenuItemLabel::create(
            syncLabel, this, menu_selector(GFNJukeboxLayer::onSyncWebsite)
        ) : nullptr;
        auto closeLabel = cocos2d::CCLabelTTF::create("Close", "bigFont.fnt", 0.38f);
        auto close = closeLabel ? cocos2d::CCMenuItemLabel::create(
            closeLabel, this, menu_selector(GFNJukeboxLayer::onClose)
        ) : nullptr;
        auto controls = cocos2d::CCMenu::create();
        if (controls) {
            if (library) { library->setPosition({ -150.f, 0.f }); controls->addChild(library); }
            if (refresh) {
                refresh->setPosition({ -75.f, 0.f });
                controls->addChild(refresh);
            }
            if (pair) {
                pair->setPosition({ 0.f, 0.f });
                controls->addChild(pair);
            }
            if (sync) {
                sync->setPosition({ 75.f, 0.f });
                controls->addChild(sync);
            }
            if (close) {
                close->setPosition({ 150.f, 0.f });
                controls->addChild(close);
            }
            controls->setPosition({ size.width / 2, size.height * 0.12f });
            controls->setScale(0.8f);
            this->addChild(controls, 3);
        }
        this->setTouchEnabled(true);
        return true;
    }

    void onRefresh(cocos2d::CCObject*) { loadSongs(1); }

    void onOpenLocalLibrary(cocos2d::CCObject*) { m_libraryPage = 1; showLocalLibrary(); }
    void onPreviousLibraryPage(cocos2d::CCObject*) { if (m_libraryPage > 1) { --m_libraryPage; showLocalLibrary(); } }
    void onNextLibraryPage(cocos2d::CCObject*) {
        const auto ids = gfn::local_library::installedSongIDs();
        const int pages = std::max<int>(1, static_cast<int>((ids.size() + LIBRARY_PAGE_SIZE - 1) / LIBRARY_PAGE_SIZE));
        if (m_libraryPage < pages) { ++m_libraryPage; showLocalLibrary(); }
    }
    void onRestoreLocalSong(cocos2d::CCObject* sender) {
        auto item = static_cast<cocos2d::CCMenuItem*>(sender);
        const int songId = item ? item->getTag() : 0;
        if (songId <= 0) return;
        gfn::local_library::clearPathForSong(songId);
        setStatus(fmt::format("Original song {} restored.", songId));
        showLocalLibrary();
    }
    void onDeleteLocalFile(cocos2d::CCObject* sender) {
        auto item = static_cast<cocos2d::CCMenuItem*>(sender);
        const int songId = item ? item->getTag() : 0;
        if (songId <= 0) return;
        if (!gfn::local_library::deleteManagedFileForSong(songId)) {
            setStatus(fmt::format(
                "Could not delete song {}: file missing or outside the mod's managed folder.",
                songId
            ));
            return;
        }
        setStatus(fmt::format("Managed NONG file for song {} deleted.", songId));
        showLocalLibrary();
    }

    void onBackFromLibrary(cocos2d::CCObject*) { showSongs(); }

    void onPairWebsite(cocos2d::CCObject*) { requestPairingCode(); }

    void onSyncWebsite(cocos2d::CCObject*) { syncWebsiteSelections(); }

    void onSearch(cocos2d::CCObject*) {
        if (!m_searchInput) return;
        m_searchInput->defocus();
        m_searchQuery = m_searchInput->getString();
        loadSongs(1);
    }

    void onClearSearch(cocos2d::CCObject*) {
        if (m_searchInput) {
            m_searchInput->setString("", false);
            m_searchInput->defocus();
        }
        m_searchQuery.clear();
        loadSongs(1);
    }

    void onPreviousPage(cocos2d::CCObject*) {
        if (m_catalogPage > 1) loadSongs(m_catalogPage - 1);
    }

    void onNextPage(cocos2d::CCObject*) {
        if (m_catalogPage < m_catalogPages) loadSongs(m_catalogPage + 1);
    }

    void onPreviousNongPage(cocos2d::CCObject*) {
        if (m_nongPage > 1) loadNongs(m_currentSongId, m_nongPage - 1);
    }

    void onNextNongPage(cocos2d::CCObject*) {
        if (m_nongPage < m_nongPages) loadNongs(m_currentSongId, m_nongPage + 1);
    }

    void onSongSelected(cocos2d::CCObject* sender) {
        auto item = static_cast<cocos2d::CCMenuItem*>(sender);
        auto index = item ? item->getTag() : -1;
        if (index < 0 || static_cast<size_t>(index) >= m_songs.size()) return;
        loadNongs(m_songs[index].id);
    }

    void onNongSelected(cocos2d::CCObject* sender) {
        auto item = static_cast<cocos2d::CCMenuItem*>(sender);
        auto index = item ? item->getTag() : -1;
        if (index < 0 || static_cast<size_t>(index) >= m_nongs.size()) return;
        saveSelectionAndDownload(m_nongs[index]);
    }

    void onUseOriginal(cocos2d::CCObject*) {
        if (m_currentSongId.empty()) return;
        int numericSongId = 0;
        try {
            numericSongId = std::stoi(m_currentSongId);
        } catch (...) {
            setStatus("This song ID cannot be mapped to a local Geometry Dash song.");
            return;
        }
        auto body = fmt::format(
            "{{\"gd_song_id\":\"{}\",\"nong_id\":null,\"device_id\":\"{}\"}}",
            m_currentSongId, m_deviceId
        );
        auto request = geode::utils::web::WebRequest();
        request.header("Content-Type", "application/json")
            .timeout(std::chrono::seconds(20))
            .bodyString(body);
        setStatus("Restoring original song selection...");
        this->retain();
        geode::async::spawn(
            request.post(m_baseUrl + "/api/v1/selections/save"),
            [this, numericSongId](geode::utils::web::WebResponse response) {
                AsyncRelease hold{ this };
                if (!this->getParent()) return;
                if (!response.ok()) {
                    setStatus(fmt::format("Could not save original selection (HTTP {}).", response.code()));
                    return;
                }
                gfn::local_library::clearPathForSong(numericSongId);
                setStatus("Original Geometry Dash song restored.");
            }
        );
    }

    void onBackToSongs(cocos2d::CCObject*) { showSongs(); }

    void onClose(cocos2d::CCObject*) {
        this->removeFromParentAndCleanup(true);
    }
};
