#include <Geode/Geode.hpp>
#include <Geode/modify/CCTouchDispatcher.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/ui/GeodeUI.hpp>

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <system_error>

using namespace geode::prelude;

namespace {
    void playClickSound() {
        auto* mod = Mod::get();
        if (!mod) {
            log::error("[RealMouseClick][DIAG] Mod::get() returned null");
            return;
        }

        if (!mod->getSettingValue<bool>("enabled")) {
            log::info("[RealMouseClick][DIAG] Touch received, but sound is disabled");
            return;
        }

        float volume = mod->getSettingValue<float>("volume");
        if (!std::isfinite(volume)) volume = 0.0f;
        volume = std::clamp(volume, 0.0f, 5.0f);
        if (volume <= 0.001f) {
            log::info("[RealMouseClick][DIAG] Touch received, but volume is zero");
            return;
        }

        const auto resourcePath = mod->getResourcesDir() / "click.wav";
        std::error_code fileError;
        const bool fileExists = std::filesystem::exists(resourcePath, fileError);

        if (fileError) {
            log::error(
                "[RealMouseClick][DIAG] Resource filesystem check failed: {} ({})",
                resourcePath.string(), fileError.message()
            );
        } else if (!fileExists) {
            log::error(
                "[RealMouseClick][DIAG] click.wav NOT FOUND at: {}",
                resourcePath.string()
            );
            return;
        } else {
            std::error_code sizeError;
            const auto fileSize = std::filesystem::file_size(resourcePath, sizeError);
            if (sizeError) {
                log::warn(
                    "[RealMouseClick][DIAG] WAV exists but file_size failed: {}",
                    sizeError.message()
                );
            } else {
                log::info(
                    "[RealMouseClick][DIAG] WAV found: {} ({} bytes)",
                    resourcePath.string(), fileSize
                );
            }
        }

        auto* audio = FMODAudioEngine::sharedEngine();
        if (!audio) {
            log::error("[RealMouseClick][DIAG] FMODAudioEngine::sharedEngine() returned null");
            return;
        }

        const auto path = resourcePath.string();
        const int channel = audio->playEffect(path, 1.0f, 0.0f, volume / 5.0f);

        if (channel < 0) {
            log::error(
                "[RealMouseClick][DIAG] playEffect FAILED; channel={}, path={}, volume={}",
                channel, path, volume
            );
        } else {
            log::info(
                "[RealMouseClick][DIAG] playEffect returned channel={}, volume={}",
                channel, volume
            );
        }
    }
}

// Observe screen touches at the Cocos2d dispatcher level.
// The original dispatcher is always called so normal GD input remains intact.
class $modify(RealMouseClickTouchDispatcher, CCTouchDispatcher) {
    void touchesBegan(CCSet* touches, CCEvent* event) {
        CCTouchDispatcher::touchesBegan(touches, event);

        if (!touches) {
            log::warn("[RealMouseClick][DIAG] touchesBegan called with null CCSet");
            return;
        }

        auto* playLayer = PlayLayer::get();
        if (!playLayer) {
            return;
        }

        log::info("[RealMouseClick][DIAG] touchesBegan reached during active PlayLayer");

        for (auto it = touches->begin(); it != touches->end(); ++it) {
            playClickSound();
        }
    }
};

// Preserve the existing pause-menu settings shortcut.
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
