#pragma once

#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>
#include "../core/LocalLibrary.hpp"
#include <chrono>
#include <string>
#include <vector>

class GFNJukeboxLayer final : public cocos2d::CCLayerColor {
    struct SongEntry {
        std::string id;
        std::string name;
        int nongs = 0;
    };
    struct NongEntry {
        int id = 0;
        std::string name;
        std::string format;
        std::string sha256;
    };

    std::string m_baseUrl;
    std::string m_deviceId;
    std::string m_currentSongId;
    std::vector<SongEntry> m_songs;
    std::vector<NongEntry> m_nongs;
    cocos2d::CCLabelTTF* m_status = nullptr;
    cocos2d::CCMenu* m_menu = nullptr;

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
        if (m_baseUrl.rfind("https://", 0) != 0 &&
            m_baseUrl.rfind("http://127.0.0.1", 0) != 0 &&
            m_baseUrl.rfind("http://localhost", 0) != 0) {
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
        setStatus("Community song catalogue");
        if (m_songs.empty()) {
            setStatus("No songs found in the community catalogue.");
            return;
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
            item->setPosition({ 0.f, 105.f - static_cast<float>(i) *  thirtyFive() });
            m_menu->addChild(item);
        }
    }

    static constexpr float thirtyFive() { return 35.f; }

    void loadSongs() {
        if (!configureServer()) return;
        setStatus("Loading community catalogue...");
        auto url = m_baseUrl + "/api/v1/songs";
        geode::async::spawn(
            geode::utils::web::WebRequest().timeout(std::chrono::seconds(15)).get(url),
            [this](geode::utils::web::WebResponse response) {
                if (!this->getParent()) return;
                if (!response.ok()) {
                    setStatus(fmt::format("Catalogue request failed (HTTP {}).", response.code()));
                    return;
                }
                auto parsed = response.json();
                if (!parsed || !parsed.unwrap().isArray()) {
                    setStatus("The server returned an invalid catalogue.");
                    return;
                }
                m_songs.clear();
                for (auto const& row : parsed.unwrap()) {
                    auto id = row["gd_song_id"].asString().unwrapOr("");
                    auto name = row["name"].asString().unwrapOr("Unknown song");
                    auto count = row["nongs"].asInt().unwrapOr(0);
                    if (!id.empty()) m_songs.push_back({ id, name, count });
                }
                showSongs();
            }
        );
    }

    void loadNongs(std::string songId) {
        m_currentSongId = std::move(songId);
        setStatus("Loading available NONGs...");
        showMenu();
        auto url = m_baseUrl + "/api/v1/songs/" + m_currentSongId + "/nongs";
        geode::async::spawn(
            geode::utils::web::WebRequest().timeout(std::chrono::seconds(15)).get(url),
            [this](geode::utils::web::WebResponse response) {
                if (!this->getParent()) return;
                if (!response.ok()) {
                    setStatus(fmt::format("NONG list request failed (HTTP {}).", response.code()));
                    return;
                }
                auto parsed = response.json();
                if (!parsed || !parsed.unwrap().isArray()) {
                    setStatus("The server returned an invalid NONG list.");
                    return;
                }
                m_nongs.clear();
                for (auto const& row : parsed.unwrap()) {
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
        setStatus("Choose a community version:");
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
        auto backLabel = cocos2d::CCLabelTTF::create("Back to songs", "bigFont.fnt", 0.38f);
        auto back = backLabel ? cocos2d::CCMenuItemLabel::create(
            backLabel, this, menu_selector(GFNJukeboxLayer::onBackToSongs)
        ) : nullptr;
        if (back) {
            back->setPosition({ 0.f, -145.f });
            m_menu->addChild(back);
        }
    }

    void saveSelectionAndDownload(NongEntry entry) {
        auto mod = geode::Mod::get();
        if (!mod) return;
        setStatus("Saving selection and downloading audio...");
        auto body = fmt::format(
            "{{\"gd_song_id\":\"{}\",\"nong_id\":{},\"device_id\":\"{}\"}}",
            m_currentSongId, entry.id, m_deviceId
        );
        auto request = geode::utils::web::WebRequest();
        request.header("Content-Type", "application/json")
            .timeout(std::chrono::seconds(30))
            .bodyString(body);
        auto saveUrl = m_baseUrl + "/api/v1/selections/save";
        geode::async::spawn(
            request.post(saveUrl),
            [this, entry](geode::utils::web::WebResponse saved) {
                if (!this->getParent()) return;
                if (!saved.ok()) {
                    setStatus(fmt::format("Could not save selection (HTTP {}).", saved.code()));
                    return;
                }
                auto downloadUrl = m_baseUrl + "/api/v1/nongs/" + std::to_string(entry.id) + "/download";
                geode::async::spawn(
                    geode::utils::web::WebRequest().timeout(std::chrono::seconds(90)).get(downloadUrl),
                    [this, entry](geode::utils::web::WebResponse audio) {
                        if (!this->getParent()) return;
                        if (!audio.ok()) {
                            setStatus(fmt::format("Download failed (HTTP {}).", audio.code()));
                            return;
                        }
                        auto mod = geode::Mod::get();
                        if (!mod) return;
                        auto filename = fmt::format("song-{}.{}", m_currentSongId, entry.format);
                        auto target = mod->getSaveDir() / filename;
                        auto written = audio.into(target);
                        if (!written) {
                            setStatus("Could not write the downloaded audio file.");
                            return;
                        }
                        gfn::local_library::setPathForSong(std::stoi(m_currentSongId), target.string());
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

        auto refreshLabel = cocos2d::CCLabelTTF::create("Refresh catalogue", "bigFont.fnt", 0.48f);
        auto refresh = refreshLabel ? cocos2d::CCMenuItemLabel::create(
            refreshLabel, this, menu_selector(GFNJukeboxLayer::onRefresh)
        ) : nullptr;
        auto closeLabel = cocos2d::CCLabelTTF::create("Close", "bigFont.fnt", 0.48f);
        auto close = closeLabel ? cocos2d::CCMenuItemLabel::create(
            closeLabel, this, menu_selector(GFNJukeboxLayer::onClose)
        ) : nullptr;
        auto controls = cocos2d::CCMenu::create();
        if (controls) {
            if (refresh) {
                refresh->setPosition({ -85.f, 0.f });
                controls->addChild(refresh);
            }
            if (close) {
                close->setPosition({ 85.f, 0.f });
                controls->addChild(close);
            }
            controls->setPosition({ size.width / 2, size.height * 0.12f });
            this->addChild(controls, 3);
        }
        this->setTouchEnabled(true);
        return true;
    }

    void onRefresh(cocos2d::CCObject*) { loadSongs(); }

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

    void onBackToSongs(cocos2d::CCObject*) { showSongs(); }

    void onClose(cocos2d::CCObject*) {
        this->removeFromParentAndCleanup(true);
    }
};
