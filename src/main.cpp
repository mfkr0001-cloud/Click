#include <Geode/Geode.hpp>
#include <Geode/modify/CCTouchDispatcher.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/ui/GeodeUI.hpp>

#include <algorithm>
#include <cmath>

using namespace geode::prelude;

namespace {
    void playClickSound() {
        auto* mod = Mod::get();
        if (!mod || !mod->getSettingValue<bool>("enabled")) return;

        float volume = mod->getSettingValue<float>("volume");
        if (!std::isfinite(volume)) volume = 0.0f;
        volume = std::clamp(volume, 0.0f, 5.0f);
        if (volume <= 0.001f) return;

        const auto path = (mod->getResourcesDir() / "click.wav").string();
        auto* audio = FMODAudioEngine::sharedEngine();
        if (!audio) {
            log::error("[RealMouseClick] FMODAudioEngine is unavailable");
            return;
        }

        const int channel = audio->playEffect(path, 1.0f, 0.0f, volume / 5.0f);
        if (channel < 0) {
            log::error("[RealMouseClick] FMOD failed to play click.wav: {}", path);
        }
    }
}

// Catch every touch at the Cocos2d dispatcher level, regardless of its screen
// position. PlayLayer::get() keeps this limited to an active Geometry Dash level.
class $modify(RealMouseClickTouchDispatcher, CCTouchDispatcher) {
    void touchesBegan(CCSet* touches, CCEvent* event) {
        CCTouchDispatcher::touchesBegan(touches, event);

        if (!touches || !PlayLayer::get()) return;

        for (auto it = touches->begin(); it != touches->end(); ++it) {
            playClickSound();
        }
    }
};

// Keep the existing pause-menu settings shortcut. The touch itself is already
// detected by CCTouchDispatcher, so this callback does not play a second sound.
class $modify(RealMouseClickPauseLayer, PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();

        auto winSize = CCDirector::sharedDirector()->getWinSize();
        auto* menu = CCMenu::create();
        if (!menu) return;

        auto* buttonSprite = ButtonSprite::create(
            "CLICK", 70, true, "goldFont.fnt", "GJ_button_01.png", 30.0f, 0.6f
        );
        if (!buttonSprite) return;

        auto* button = CCMenuItemSpriteExtra::create(
            buttonSprite,
            this,
            menu_selector(RealMouseClickPauseLayer::onRealMouseClickSettings)
        );
        if (!button) return;

        menu->addChild(button);
        menu->setPosition({winSize.width * 0.88f, winSize.height * 0.82f});
        menu->setID("real-mouse-click-settings-menu");
        this->addChild(menu, 1000);
    }

    void onRealMouseClickSettings(CCObject*) {
        openSettingsPopup(Mod::get());
    }
};
