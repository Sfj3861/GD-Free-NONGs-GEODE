#pragma once

#include <Geode/Geode.hpp>
#include <Geode/utils/file.hpp>
#include "../core/LocalLibrary.hpp"
#include <algorithm>
#include <cstdint>
#include <cctype>
#include <cstring>
#include <span>
#include <filesystem>
#include <string>
#include <utility>
#include <vector>

class GFNJukeboxLayer final : public cocos2d::CCLayerColor {
    static constexpr size_t PAGE_SIZE = 7;
    struct AsyncRelease { GFNJukeboxLayer* layer; ~AsyncRelease() { layer->release(); } };
    int m_page = 1;
    std::vector<int> m_songIds;
    cocos2d::CCLabelTTF* m_status = nullptr;
    cocos2d::CCMenu* m_menu = nullptr;

    void setStatus(std::string const& message) {
        if (m_status) m_status->setString(message.c_str());
    }

    void clearMenu() {
        if (m_menu) {
            m_menu->removeFromParentAndCleanup(true);
            m_menu = nullptr;
        }
    }

    void addButton(std::string const& text, cocos2d::CCPoint position,
                   cocos2d::SEL_MenuHandler callback, int tag = 0) {
        auto label = cocos2d::CCLabelTTF::create(text.c_str(), "bigFont.fnt", 0.34f);
        auto item = label ? cocos2d::CCMenuItemLabel::create(label, this, callback) : nullptr;
        if (!item || !m_menu) return;
        item->setPosition(position);
        item->setTag(tag);
        m_menu->addChild(item);
    }

    static bool validAudioPayload(std::string const& extension, std::span<uint8_t const> bytes) {
        constexpr size_t MAX_AUDIO_BYTES = 30u * 1024u * 1024u;
        if (bytes.empty() || bytes.size() > MAX_AUDIO_BYTES) return false;
        if (extension == "mp3") {
            return (bytes.size() >= 3 && std::memcmp(bytes.data(), "ID3", 3) == 0) ||
                (bytes.size() >= 2 && bytes[0] == 0xFF && (bytes[1] & 0xE0) == 0xE0);
        }
        if (extension == "ogg") return bytes.size() >= 4 && std::memcmp(bytes.data(), "OggS", 4) == 0;
        if (extension == "wav") return bytes.size() >= 12 &&
            std::memcmp(bytes.data(), "RIFF", 4) == 0 && std::memcmp(bytes.data() + 8, "WAVE", 4) == 0;
        if (extension == "flac") return bytes.size() >= 4 && std::memcmp(bytes.data(), "fLaC", 4) == 0;
        if (extension == "m4a") return bytes.size() >= 8 && std::memcmp(bytes.data() + 4, "ftyp", 4) == 0;
        return false;
    }

    void showLibrary() {
        m_songIds = gfn::local_library::installedSongIDs();
        const int pages = std::max<int>(1, static_cast<int>((m_songIds.size() + PAGE_SIZE - 1) / PAGE_SIZE));
        m_page = std::clamp(m_page, 1, pages);

        clearMenu();
        m_menu = cocos2d::CCMenu::create();
        if (!m_menu) return;
        auto size = cocos2d::CCDirector::sharedDirector()->getWinSize();
        m_menu->setPosition({ size.width / 2, size.height * 0.46f });
        this->addChild(m_menu, 2);

        if (m_songIds.empty()) {
            setStatus("No local NONGs yet. Import a file named song-ID.mp3/ogg/wav/flac/m4a.");
        } else {
            setStatus(fmt::format("Local NONG library · {} songs · Page {} / {}",
                m_songIds.size(), m_page, pages));
        }

        const size_t start = static_cast<size_t>(m_page - 1) * PAGE_SIZE;
        const size_t end = std::min(m_songIds.size(), start + PAGE_SIZE);
        for (size_t index = start; index < end; ++index) {
            const int songId = m_songIds[index];
            const auto path = gfn::local_library::pathForSong(songId);
            std::error_code error;
            const bool exists = !path.empty() &&
                std::filesystem::is_regular_file(path, error) && !error;
            const auto filename = path.empty() ? std::string("Missing file") :
                std::filesystem::path(path).filename().string();
            const auto sizeText = exists
                ? fmt::format("{} bytes", gfn::local_library::fileSizeForSong(songId))
                : std::string("Missing");
            auto label = cocos2d::CCLabelTTF::create(
                fmt::format("GD {} · {} · {}", songId, filename, sizeText).c_str(),
                "bigFont.fnt", 0.29f
            );
            if (label) {
                auto row = cocos2d::CCMenuItemLabel::create(
                    label, this, menu_selector(GFNJukeboxLayer::onRefresh)
                );
                if (row) {
                    row->setPosition({ -30.f, 105.f - static_cast<float>(index - start) * 35.f });
                    m_menu->addChild(row);
                }
            }
            addButton("Delete", { 175.f, 105.f - static_cast<float>(index - start) * 35.f },
                menu_selector(GFNJukeboxLayer::onDelete), songId);
        }

        if (m_page > 1) addButton("< Previous", { -85.f, -145.f },
            menu_selector(GFNJukeboxLayer::onPrevious));
        if (m_page < pages) addButton("Next >", { 85.f, -145.f },
            menu_selector(GFNJukeboxLayer::onNext));
        addButton("Import audio", { 0.f, -175.f },
            menu_selector(GFNJukeboxLayer::onImportAudio));
        addButton("Refresh library", { 0.f, -205.f },
            menu_selector(GFNJukeboxLayer::onRefresh));
        addButton("Close", { 0.f, -235.f },
            menu_selector(GFNJukeboxLayer::onClose));
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
        auto title = cocos2d::CCLabelTTF::create("GD Free NONGs · Offline", "bigFont.fnt", 0.62f);
        if (title) {
            title->setPosition({ size.width / 2, size.height * 0.84f });
            this->addChild(title);
        }
        m_status = cocos2d::CCLabelTTF::create("Scanning local audio files...", "goldFont.fnt", 0.36f);
        if (m_status) {
            m_status->setPosition({ size.width / 2, size.height * 0.72f });
            m_status->setDimensions({ size.width * 0.88f, 48.f });
            this->addChild(m_status);
        }
        showLibrary();
        return true;
    }

    void onImportAudio(cocos2d::CCObject*) {
        setStatus("Choose a NONG named song-<GD song ID>.<format>.");
        geode::utils::file::FilePickOptions options {
            .filters = { geode::utils::file::FilePickOptions::Filter {
                .description = "NONG audio files",
                .files = { "*.mp3", "*.ogg", "*.wav", "*.flac", "*.m4a" },
            }}
        };
        this->retain();
        geode::async::spawn(
            geode::utils::file::pick(
                geode::utils::file::PickMode::OpenFile,
                std::move(options)
            ),
            [this](geode::utils::file::PickResult result) {
                AsyncRelease hold{ this };
                if (!this->getParent()) return;
                if (!result.isOk()) {
                    setStatus(fmt::format("File picker failed: {}", result.unwrapErr()));
                    return;
                }
                auto selected = std::move(result).unwrap();
                if (!selected.has_value()) {
                    setStatus("Import cancelled.");
                    return;
                }

                const auto source = selected.value();
                const auto filename = std::filesystem::path(source.string()).filename().string();
                if (filename.rfind("song-", 0) != 0) {
                    setStatus("Invalid filename. Expected song-<GD song ID>.<format>.");
                    return;
                }
                const auto dot = filename.find('.', 5);
                if (dot == std::string::npos) {
                    setStatus("Invalid filename. The song ID and audio extension are required.");
                    return;
                }

                int songId = 0;
                try {
                    const auto idText = filename.substr(5, dot - 5);
                    size_t consumed = 0;
                    songId = std::stoi(idText, &consumed);
                    if (consumed != idText.size() || songId <= 0) songId = 0;
                } catch (...) {
                    songId = 0;
                }
                if (songId <= 0) {
                    setStatus("Invalid Geometry Dash song ID in filename.");
                    return;
                }

                auto extension = filename.substr(dot + 1);
                std::transform(extension.begin(), extension.end(), extension.begin(),
                    [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
                if (extension != "mp3" && extension != "ogg" && extension != "wav" &&
                    extension != "flac" && extension != "m4a") {
                    setStatus("Unsupported audio format.");
                    return;
                }

                auto data = geode::utils::file::readBinary(source);
                if (!data.isOk()) {
                    setStatus(fmt::format("Could not read audio file: {}", data.unwrapErr()));
                    return;
                }
                auto bytes = std::move(data).unwrap();
                std::span<uint8_t const> view(bytes.data(), bytes.size());
                if (!validAudioPayload(extension, view)) {
                    setStatus("Invalid audio file or file exceeds 30 MiB.");
                    return;
                }

                auto mod = geode::Mod::get();
                if (!mod) {
                    setStatus("Could not access the Geode mod data directory.");
                    return;
                }
                const auto targetName = fmt::format("song-{}.{}", songId, extension);
                const auto target = mod->getSaveDir() / targetName;
                auto written = geode::utils::file::writeBinarySafe(target, view);
                if (!written.isOk()) {
                    setStatus(fmt::format("Could not save local NONG: {}", written.unwrapErr()));
                    return;
                }

                gfn::local_library::setPathForSong(songId, target.string());
                m_page = 1;
                showLibrary();
                setStatus(fmt::format("Imported and activated local NONG for GD song {}.", songId));
            }
        );
    }

    void onRefresh(cocos2d::CCObject*) {
        m_page = 1;
        showLibrary();
        setStatus(fmt::format("Scanned local library · {} songs found.", m_songIds.size()));
    }

    void onPrevious(cocos2d::CCObject*) {
        if (m_page > 1) { --m_page; showLibrary(); }
    }

    void onNext(cocos2d::CCObject*) {
        ++m_page;
        showLibrary();
    }

    void onDelete(cocos2d::CCObject* sender) {
        auto item = static_cast<cocos2d::CCMenuItem*>(sender);
        const int songId = item ? item->getTag() : 0;
        if (songId <= 0) return;
        if (!gfn::local_library::deleteManagedFileForSong(songId)) {
            setStatus(fmt::format("Could not delete GD {}: file missing or not managed by this mod.", songId));
            return;
        }
        showLibrary();
        setStatus(fmt::format("Deleted local NONG for GD {}. The original song is restored.", songId));
    }

    void onClose(cocos2d::CCObject*) {
        this->removeFromParentAndCleanup(true);
    }
};
