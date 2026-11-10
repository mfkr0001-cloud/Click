
#include <Geode/Geode.hpp>
#include <Geode/modify/CCTouchDispatcher.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/ui/GeodeUI.hpp>

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <system_error>
#include <string>

using namespace geode::prelude;

namespace {
    unsigned long long s_touchCount = 0;
    unsigned long long s_dispatcherCallCount = 0;

    bool s_loggedFirstGameplayTouch = false;

    std::string s_playStatus = "WAITING";
    std::string s_wavStatus = "CHECKING";
    std::string s_fmodStatus = "CHECKING";

    float getClickVolume() {
        auto* mod = Mod::get();
        if (!mod) return 0.0f;

        float volume =
            mod->getSettingValue<float>("volume");

        if (!std::isfinite(volume))
            volume = 0.0f;

        return std::clamp(volume, 0.0f, 5.0f);
    }

    bool isClickEnabled() {
        auto* mod = Mod::get();

        return mod &&
            mod->getSettingValue<bool>("enabled");
    }

    // ИСПРАВЛЕННЫЙ ПУТЬ К WAV
    std::filesystem::path getClickPath() {
        auto* mod = Mod::get();

        if (!mod) return {};

        return mod->getResourcesDir()
            / "audio"
            / "click_loud.wav";
    }

    void playClickSound() {
        if (!isClickEnabled()) {
            s_playStatus = "DISABLED";
            return;
        }

        const float volume = getClickVolume();

        if (volume <= 0.001f) {
            s_playStatus = "VOLUME ZERO";
            return;
        }

        const auto path = getClickPath();

        std::error_code error;

        const bool exists =
            !path.empty() &&
            std::filesystem::exists(path, error) &&
            !error;

        if (!exists) {
            s_wavStatus = "MISSING";
            s_playStatus = "FILE ERROR";

            log::error(
                "[RealMouseClick] WAV missing: {}",
                path.string()
            );

            return;
        }

        s_wavStatus = "FOUND";

        auto* audio = FMODAudioEngine::sharedEngine();

        if (!audio) {
            s_fmodStatus = "UNAVAILABLE";
            s_playStatus = "FMOD NULL";
            return;
        }

        s_fmodStatus = "READY";

        const int channel = audio->playEffect(
            path.string(),
            1.0f,
            0.0f,
            volume / 5.0f
        );

        if (channel < 0) {
            s_playStatus = "PLAY FAILED";

            log::error(
                "[RealMouseClick] playEffect failed: {}",
                channel
            );
        }
        else {
            s_playStatus =
                fmt::format("PLAY OK ({})", channel);
        }
    }

    std::string makeMonitorText() {
        const bool enabled = isClickEnabled();

        const int volumePercent =
            static_cast<int>(
                std::round(getClickVolume() * 20.0f)
            );

        const auto path = getClickPath();

        std::error_code error;

        const bool exists =
            !path.empty() &&
            std::filesystem::exists(path, error) &&
            !error;

        s_wavStatus = exists ? "FOUND" : "MISSING";

        s_fmodStatus =
            FMODAudioEngine::sharedEngine()
            ? "READY"
            : "UNAVAILABLE";

        return fmt::format(
            "REAL MOUSE CLICK\n"
            "SOUND: {}\n"
            "VOLUME: {}%\n"
            "TOUCHES: {}\n"
            "DISPATCH: {}\n"
            "WAV: {}\n"
            "FMOD: {}\n"
            "PLAY: {}",

            enabled ? "ON" : "OFF",
            volumePercent,
            s_touchCount,
            s_dispatcherCallCount,
            s_wavStatus,
            s_fmodStatus,
            s_playStatus
        );
    }
}


// ОБРАБОТЧИК КАСАНИЙ
class $modify(
    RealMouseClickTouchDispatcher,
    CCTouchDispatcher
) {
    void touchesBegan(
        CCSet* touches,
        CCEvent* event
    ) {
        ++s_dispatcherCallCount;

        // Не блокируем стандартные касания GD.
        CCTouchDispatcher::touchesBegan(
            touches,
            event
        );

        if (!touches || !PlayLayer::get())
            return;

        for (auto it = touches->begin();
             it != touches->end();
             ++it) {

            ++s_touchCount;

            if (!s_loggedFirstGameplayTouch) {
                s_loggedFirstGameplayTouch = true;

                log::info(
                    "[RealMouseClick] First gameplay touch received"
                );
            }

            playClickSound();
        }
    }
};


// ИГРОВОЙ МОНИТОР
class $modify(
    RealMouseClickPlayLayer,
    PlayLayer
) {
    bool init(
        GJGameLevel* level,
        bool useReplay,
        bool dontCreateObjects
    ) {
        if (!PlayLayer::init(
            level,
            useReplay,
            dontCreateObjects
        )) {
            return false;
        }

        s_touchCount = 0;
        s_dispatcherCallCount = 0;

        s_loggedFirstGameplayTouch = false;

        s_playStatus = "WAITING";
        s_wavStatus = "CHECKING";
        s_fmodStatus = "CHECKING";

        auto* mod = Mod::get();

        if (!mod ||
            !mod->getSettingValue<bool>("show-monitor")) {
            return true;
        }

        const auto winSize =
            CCDirector::sharedDirector()->getWinSize();

        auto* panel = CCLayerColor::create(
            ccc4(0, 0, 0, 190),
            224.0f,
            140.0f
        );

        if (!panel) {
            log::warn(
                "[RealMouseClick] Monitor panel creation failed"
            );

            return true;
        }

        panel->setAnchorPoint({0.0f, 1.0f});

        panel->setPosition({
            10.0f,
            winSize.height - 10.0f
        });

        panel->setID(
            "real-mouse-click-monitor-panel"
        );

        this->addChild(panel, 10000);

        auto* label = CCLabelBMFont::create(
            makeMonitorText().c_str(),
            "bigFont.fnt"
        );

        if (!label)
            return true;

        label->setAnchorPoint({0.0f, 1.0f});

        label->setPosition({
            8.0f,
            132.0f
        });

        label->setScale(0.30f);

        label->setID(
            "real-mouse-click-monitor-label"
        );

        panel->addChild(label);

        log::info(
            "[RealMouseClick] Monitor created"
        );

        return true;
    }

    void update(float dt) {
        PlayLayer::update(dt);

        static float timer = 0.0f;

        timer += dt;

        if (timer < 0.20f)
            return;

        timer = 0.0f;

        auto* panel = this->getChildByID(
            "real-mouse-click-monitor-panel"
        );

        if (!panel)
            return;

        auto* label =
            typeinfo_cast<CCLabelBMFont*>(
                panel->getChildByID(
                    "real-mouse-click-monitor-label"
                )
            );

        if (label) {
            const auto text = makeMonitorText();

            label->setString(text.c_str());
        }
    }
};


// КНОПКА НАСТРОЕК
class $modify(
    RealMouseClickPauseLayer,
    PauseLayer
) {
    void customSetup() {
        PauseLayer::customSetup();

        const auto winSize =
            CCDirector::sharedDirector()->getWinSize();

        auto* menu = CCMenu::create();

        if (!menu)
            return;

        auto* sprite = ButtonSprite::create(
            "CLICK",
            70,
            true,
            "goldFont.fnt",
            "GJ_button_01.png",
            30.0f,
            0.6f
        );

        if (!sprite)
            return;

        auto* button = CCMenuItemSpriteExtra::create(
            sprite,
            this,
            menu_selector(
                RealMouseClickPauseLayer::
                onRealMouseClickSettings
            )
        );

        if (!button)
            return;

        menu->addChild(button);

        menu->setPosition({
            winSize.width * 0.88f,
            winSize.height * 0.82f
        });

        menu->setID(
            "real-mouse-click-settings-menu"
        );

        this->addChild(menu, 1000);
    }

    void onRealMouseClickSettings(CCObject*) {
        openSettingsPopup(Mod::get());
    }
};
