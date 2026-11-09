#include <Geode/Geode.hpp>
#include <Geode/modify/CCTouchDispatcher.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/ui/GeodeUI.hpp>

#include <algorithm>
#include <cmath>

using namespace geode::prelude;

namespace {
    std::string getClickSoundPath() {
        return (Mod::get()->getResourcesDir() / "audio" / "click.wav").string();
    }

    void playClickSound() {
        auto* mod = Mod::get();
        if (!mod->getSettingValue<bool>("enabled")) return;

        float volume = mod->getSettingValue<float>("volume");
        if (!std::isfinite(volume)) volume = 0.0f;
        volume = std::clamp(volume, 0.0f, 5.0f);
        if (volume <= 0.001f) return;

        auto* audio = FMODAudioEngine::sharedEngine();
        if (!audio) {
            log::error("[RealMouseClick] FMODAudioEngine is unavailable");
            return;
        }

        const auto path = getClickSoundPath();
        // Do not depend on preloadEffect's cache state: ask FMOD to play the
        // packaged resource directly for each touch.
        const int channel = audio->playEffect(path, 1.0f, 0.0f, volume / 5.0f);
        if (channel < 0) {
            log::error("[RealMouseClick] FMOD could not play click.wav: {}", path);
        }
    }
}

// Test 10's intended input source: actual Cocos touch-begin events.
// Call the original dispatcher first so the game and other mods still receive
// the touch normally, then play one click for each touch in the event set.
class $modify(RealMouseClickTouchDispatcher, CCTouchDispatcher) {
    void touchesBegan(CCSet* touches, CCEvent* event) {
        CCTouchDispatcher::touchesBegan(touches, event);

        if (!touches) return;
        for (auto it = touches->begin(); it != touches->end(); ++it) {
            playClickSound();
        }
    }
};

// Settings shortcut in the in-level pause menu. Pressing CLICK also tests audio.
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
        playClickSound();
        openSettingsPopup(Mod::get());
    }
};
