#include <Geode/Geode.hpp>
#include <Geode/loader/GameEvent.hpp>
#include <Geode/loader/SettingV3.hpp>
#include <Geode/modify/MenuLayer.hpp>

#include "ui/JukeboxLayer.hpp"

using namespace geode::prelude;

namespace {
constexpr int GFN_JUKEBOX_TAG = 0x47464E;

void openGFNJukebox() {
    auto mod = Mod::get();
    if (!mod || !mod->getSettingValue<bool>("enabled")) {
        return;
    }

    auto scene = CCDirector::sharedDirector()->getRunningScene();
    if (!scene) {
        log::warn("Cannot open GD Free NONGs: no running scene.");
        return;
    }

    if (scene->getChildByTag(GFN_JUKEBOX_TAG)) {
        return;
    }

    auto jukebox = GFNJukeboxLayer::create();
    if (jukebox) {
        jukebox->setTag(GFN_JUKEBOX_TAG);
        scene->addChild(jukebox, 1000);
        log::info("GD Free NONGs Jukebox opened.");
    }
}
}

$on_mod(Loaded) {
    log::info("GD Free NONGs v0.3.0 (offline) loaded.");
}

$on_game(Loaded) {
    listenForKeybindSettingPresses(
        "open-jukebox",
        [](Keybind const&, bool down, bool repeat, double) {
            if (!down || repeat) {
                return;
            }
            openGFNJukebox();
        }
    );
}

class $modify(GFNMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) {
            return false;
        }

        auto mod = Mod::get();
        if (!mod || !mod->getSettingValue<bool>("enabled")) {
            return true;
        }

        // A visible touch target makes the Jukebox accessible on Android
        // devices that do not have a physical keyboard attached.
        auto label = cocos2d::CCLabelBMFont::create("NONGs", "goldFont.fnt");
        if (!label) {
            log::warn("Could not create the GD Free NONGs menu label.");
            return true;
        }

        auto button = cocos2d::CCMenuItemLabel::create(
            label, this, menu_selector(GFNMenuLayer::onOpenGFNJukebox)
        );
        if (!button) {
            log::warn("Could not create the GD Free NONGs menu button.");
            return true;
        }

        button->setID("gfn-open-jukebox"_spr);
        button->setScale(0.75f);

        if (auto menu = this->getChildByID("bottom-menu")) {
            menu->addChild(button);
            menu->updateLayout();
        } else {
            auto fallbackMenu = cocos2d::CCMenu::create();
            if (fallbackMenu) {
                fallbackMenu->addChild(button);
                auto size = cocos2d::CCDirector::sharedDirector()->getWinSize();
                fallbackMenu->setPosition({ size.width * 0.82f, size.height * 0.12f });
                this->addChild(fallbackMenu, 10);
            }
        }

        log::info("GD Free NONGs touch button added to the main menu.");
        return true;
    }

    void onOpenGFNJukebox(cocos2d::CCObject*) {
        openGFNJukebox();
    }
};
