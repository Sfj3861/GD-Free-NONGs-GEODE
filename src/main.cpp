#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>

using namespace geode::prelude;

$on_mod(Loaded) {
    log::info("GD Free NONGs v0.1.0 loaded.");
}

class $modify(GFNMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) {
            return false;
        }

        auto mod = Mod::get();
        if (mod && mod->getSettingValue<bool>("enabled")) {
            log::info("GD Free NONGs is enabled. Music resolver is not installed yet.");
        }

        return true;
    }
};
