#pragma once

#include <Geode/Geode.hpp>

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
        auto title = cocos2d::CCLabelTTF::create(
            "GD Free NONGs",
            "bigFont.fnt",
            0.8f
        );
        if (title) {
            title->setPosition({ winSize.width / 2, winSize.height * 0.72f });
            this->addChild(title);
        }

        auto info = cocos2d::CCLabelTTF::create(
            "Your local NONG library will appear here.",
            "goldFont.fnt",
            0.45f
        );
        if (info) {
            info->setPosition({ winSize.width / 2, winSize.height * 0.52f });
            this->addChild(info);
        }

        auto closeLabel = cocos2d::CCLabelTTF::create("Close", "bigFont.fnt", 0.55f);
        auto close = closeLabel ? cocos2d::CCMenuItemLabel::create(
            closeLabel,
            this,
            menu_selector(GFNJukeboxLayer::onClose)
        ) : nullptr;
        if (close) {
            auto menu = cocos2d::CCMenu::create(close, nullptr);
            menu->setPosition({ winSize.width / 2, winSize.height * 0.28f });
            this->addChild(menu);
        }

        this->setTouchEnabled(true);
        return true;
    }

    void onClose(cocos2d::CCObject*) {
        this->removeFromParentAndCleanup(true);
    }
};
