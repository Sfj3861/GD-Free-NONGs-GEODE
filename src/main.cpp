#include <Geode/Geode.hpp>
#include <Geode/loader/GameEvent.hpp>
#include <Geode/loader/SettingV3.hpp>
#include <Geode/modify/MenuLayer.hpp>

using namespace geode::prelude;

$on_mod(Loaded) {
    log::info("GD Free NONGs v0.1.1 loaded.");
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

            // The hotkey is registered and configurable. The in-game Jukebox
            // layer will be connected here in the next implementation step.
            log::info("GD Free NONGs Jukebox hotkey pressed.");
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
