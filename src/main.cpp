#include <Geode/Geode.hpp>
#include <Geode/loader/GameEvent.hpp>
#include <Geode/loader/SettingV3.hpp>
#include <Geode/modify/MenuLayer.hpp>

#include "ui/JukeboxLayer.hpp"

using namespace geode::prelude;

$on_mod(Loaded) {
    log::info("GD Free NONGs v0.2.0 loaded.");
}

$on_game(Loaded) {
    listenForKeybindSettingPresses(
        "open-jukebox",
        [](Keybind const&, bool down, bool repeat, double) {
            if (!down || repeat) {
                return;
            }

            auto mod = Mod::get();
            if (!mod || !mod->getSettingValue<bool>("enabled")) {
                return;
            }

            auto scene = CCDirector::sharedDirector()->getRunningScene();
            if (!scene) {
                log::warn("Cannot open GD Free NONGs: no running scene.");
                return;
            }

            if (scene->getChildByTag(0x47464E)) {
                return;
            }

            auto jukebox = GFNJukeboxLayer::create();
            if (jukebox) {
                jukebox->setTag(0x47464E);
                scene->addChild(jukebox, 1000);
                log::info("GD Free NONGs Jukebox opened.");
            }
        }
    );
}

class $modify(GFNMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) {
            return false;
        }

        auto mod = Mod::get();
        if (mod && mod->getSettingValue<bool>("enabled")) {
            log::info("GD Free NONGs is enabled.");
        }

        return true;
    }
};
