#pragma once

#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>
#include <chrono>

class GFNJukeboxLayer final : public cocos2d::CCLayerColor {
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
        if (!CCLayerColor::initWithColor(cocos2d::ccc4(12, 16, 26, 238))) {
            return false;
        }

        auto winSize = cocos2d::CCDirector::sharedDirector()->getWinSize();
        auto title = cocos2d::CCLabelTTF::create("GD Free NONGs", "bigFont.fnt", 0.8f);
        if (title) {
            title->setPosition({ winSize.width / 2, winSize.height * 0.76f });
            this->addChild(title);
        }

        auto info = cocos2d::CCLabelTTF::create(
            "Community library connection",
            "goldFont.fnt",
            0.5f
        );
        if (info) {
            info->setPosition({ winSize.width / 2, winSize.height * 0.62f });
            this->addChild(info);
        }

        auto testLabel = cocos2d::CCLabelTTF::create("Test API connection", "bigFont.fnt", 0.5f);
        auto test = testLabel ? cocos2d::CCMenuItemLabel::create(
            testLabel, this, menu_selector(GFNJukeboxLayer::onTestConnection)
        ) : nullptr;
        auto closeLabel = cocos2d::CCLabelTTF::create("Close", "bigFont.fnt", 0.5f);
        auto close = closeLabel ? cocos2d::CCMenuItemLabel::create(
            closeLabel, this, menu_selector(GFNJukeboxLayer::onClose)
        ) : nullptr;

        auto menu = cocos2d::CCMenu::create();
        if (menu) {
            if (test) {
                test->setPosition({ 0.f, 24.f });
                menu->addChild(test);
            }
            if (close) {
                close->setPosition({ 0.f, -32.f });
                menu->addChild(close);
            }
            menu->setPosition({ winSize.width / 2, winSize.height * 0.35f });
            this->addChild(menu);
        }

        this->setTouchEnabled(true);
        return true;
    }

    void onTestConnection(cocos2d::CCObject*) {
        auto mod = geode::Mod::get();
        auto baseUrl = mod ? mod->getSettingValue<std::string>("api-url") : std::string{};
        while (!baseUrl.empty() && baseUrl.back() == '/') {
            baseUrl.pop_back();
        }
        if (baseUrl.empty()) {
            FLAlertLayer::create(
                "API not configured",
                "Set the GD Free NONGs API URL in the mod settings first.",
                "OK"
            )->show();
            return;
        }
        if (baseUrl.rfind("https://", 0) != 0 && baseUrl.rfind("http://127.0.0.1", 0) != 0 && baseUrl.rfind("http://localhost", 0) != 0) {
            FLAlertLayer::create(
                "Invalid API URL",
                "Use HTTPS for remote servers. HTTP is allowed only for localhost development.",
                "OK"
            )->show();
            return;
        }

        geode::async::spawn(
            geode::utils::web::WebRequest()
                .timeout(std::chrono::seconds(10))
                .get(baseUrl + "/health"),
            [](geode::utils::web::WebResponse response) {
                if (response.ok()) {
                    FLAlertLayer::create(
                        "API connection successful",
                        fmt::format("Server responded with HTTP {}.", response.code()).c_str(),
                        "OK"
                    )->show();
                } else {
                    FLAlertLayer::create(
                        "API connection failed",
                        fmt::format("HTTP {}. Check the URL and server status.", response.code()).c_str(),
                        "OK"
                    )->show();
                }
            }
        );
    }

    void onClose(cocos2d::CCObject*) {
        this->removeFromParentAndCleanup(true);
    }
};
