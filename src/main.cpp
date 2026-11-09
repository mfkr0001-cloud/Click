#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/ui/GeodeUI.hpp>

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <system_error>

using namespace geode::prelude;

namespace {
    const std::filesystem::path& clickSoundPath() {
        static const std::filesystem::path path =
            Mod::get()->getResourcesDir() / "audio" / "click.wav";
        return path;
    }

    bool clickSoundIsValid() {
        static const bool valid = [] {
            const auto& path = clickSoundPath();
            std::error_code error;

            if (!std::filesystem::is_regular_file(path, error) || error) {
                log::error("[RealMouseClick] Audio file is missing or inaccessible: {}", path.string());
                return false;
            }

            const auto fileSize = std::filesystem::file_size(path, error);
            if (error || fileSize < 44) {
                log::error("[RealMouseClick] WAV file is too small or its size cannot be read");
                return false;
            }

            std::ifstream file(path, std::ios::binary);
            unsigned char header[44] = {};
            file.read(reinterpret_cast<char*>(header), sizeof(header));

            const bool hasExpectedChunks =
                file.gcount() == sizeof(header) &&
                header[0] == 'R' && header[1] == 'I' &&
                header[2] == 'F' && header[3] == 'F' &&
                header[8] == 'W' && header[9] == 'A' &&
                header[10] == 'V' && header[11] == 'E' &&
                header[12] == 'f' && header[13] == 'm' &&
                header[14] == 't' && header[15] == ' ' &&
                header[36] == 'd' && header[37] == 'a' &&
                header[38] == 't' && header[39] == 'a';

            if (!hasExpectedChunks) {
                log::error("[RealMouseClick] WAV is not in the expected PCM layout");
                return false;
            }

            const auto readU16 = [&header](unsigned int offset) -> unsigned int {
                return static_cast<unsigned int>(header[offset]) |
                    (static_cast<unsigned int>(header[offset + 1]) << 8);
            };
            const auto readU32 = [&header](unsigned int offset) -> unsigned int {
                return static_cast<unsigned int>(header[offset]) |
                    (static_cast<unsigned int>(header[offset + 1]) << 8) |
                    (static_cast<unsigned int>(header[offset + 2]) << 16) |
                    (static_cast<unsigned int>(header[offset + 3]) << 24);
            };

            const unsigned int riffSize = readU32(4);
            const unsigned int formatSize = readU32(16);
            const unsigned int audioFormat = readU16(20);
            const unsigned int channels = readU16(22);
            const unsigned int sampleRate = readU32(24);
            const unsigned int byteRate = readU32(28);
            const unsigned int blockAlign = readU16(32);
            const unsigned int bitsPerSample = readU16(34);
            const unsigned int dataSize = readU32(40);

            // Test 10: validate the complete canonical 44-byte PCM WAV layout.
            // Use 64-bit arithmetic for size/rate calculations to avoid overflow.
            const auto expectedBlockAlign =
                static_cast<unsigned long long>(channels) * (bitsPerSample / 8);
            const auto expectedByteRate =
                static_cast<unsigned long long>(sampleRate) * expectedBlockAlign;
            const auto declaredFileSize =
                static_cast<unsigned long long>(riffSize) + 8ULL;

            const bool validPcm =
                audioFormat == 1 &&
                formatSize == 16 &&
                channels >= 1 && channels <= 2 &&
                sampleRate > 0 &&
                (bitsPerSample == 8 || bitsPerSample == 16 ||
                 bitsPerSample == 24 || bitsPerSample == 32) &&
                blockAlign == expectedBlockAlign &&
                byteRate == expectedByteRate &&
                dataSize > 0 &&
                dataSize % blockAlign == 0 &&
                declaredFileSize == static_cast<unsigned long long>(fileSize) &&
                static_cast<unsigned long long>(dataSize) ==
                    static_cast<unsigned long long>(fileSize) - 44ULL;

            if (!validPcm) {
                log::error("[RealMouseClick] WAV PCM metadata or chunk sizes are invalid");
                return false;
            }

            log::info(
                "[RealMouseClick] WAV validated: {} Hz, {} channel(s), {}-bit PCM",
                sampleRate, channels, bitsPerSample
            );
            return true;
        }();
        return valid;
    }
    void playClickSound() {
        auto* mod = Mod::get();
        if (!mod->getSettingValue<bool>("enabled")) {
            return;
        }

        float volume = mod->getSettingValue<float>("volume");
        if (!std::isfinite(volume)) {
            volume = 0.0f;
        }
        volume = std::clamp(volume, 0.0f, 5.0f);
        // Convert the 0–5 setting to FMOD gain (0.0–1.0).

        if (volume <= 0.001f || !clickSoundIsValid()) {
            return;
        }

        auto* audio = FMODAudioEngine::sharedEngine();
        if (!audio) {
            static bool logged = false;
            if (!logged) {
                log::error("[RealMouseClick] FMODAudioEngine is unavailable");
                logged = true;
            }
            return;
        }

        // Explicitly preload the packaged WAV before requesting playback.
        // The sound is loaded once and then reused for every gameplay press.
        static bool preloadAttempted = false;
        if (!preloadAttempted) {
            preloadAttempted = true;
            auto* loadedSound = audio->preloadEffect(clickSoundPath().string());
            if (!loadedSound) {
                log::error("[RealMouseClick] FMOD could not preload click.wav at: {}",
                    clickSoundPath().string());
            } else {
                log::info("[RealMouseClick] click.wav preloaded successfully");
            }
        }

        const int channel = audio->playEffect(
            clickSoundPath().string(),
            1.0f,
            0.0f,
            volume / 5.0f
        );
        if (channel < 0) {
            static bool loggedPlaybackFailure = false;
            if (!loggedPlaybackFailure) {
                log::error("[RealMouseClick] FMOD failed to start click.wav playback");
                loggedPlaybackFailure = true;
            }
        }
    }
}

// Listen to Geometry Dash's actual gameplay input, not raw screen touches.
// This fires only when the game registers a button press (down == true),
// so menus and ordinary screen taps do not produce click sounds.
class $modify(RealMouseClickGameInput, GJBaseGameLayer) {
    void handleButton(bool down, int button, bool isPlayer1) {
        GJBaseGameLayer::handleButton(down, button, isPlayer1);

        if (down) {
            playClickSound();
        }
    }
};


// Adds a shortcut to the mod's persistent Geode settings from the
// in-level pause menu. The actual settings remain defined in mod.json.
class $modify(RealMouseClickPauseLayer, PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();

        auto winSize = CCDirector::sharedDirector()->getWinSize();

        auto* menu = CCMenu::create();
        if (!menu) {
            log::warn("[RealMouseClick] Could not create pause-menu shortcut");
            return;
        }

        auto* buttonSprite = ButtonSprite::create(
            "CLICK",
            70,
            true,
            "goldFont.fnt",
            "GJ_button_01.png",
            30.0f,
            0.6f
        );
        if (!buttonSprite) {
            log::warn("[RealMouseClick] Could not create pause-menu button sprite");
            return;
        }

        auto* button = CCMenuItemSpriteExtra::create(
            buttonSprite,
            this,
            menu_selector(RealMouseClickPauseLayer::onRealMouseClickSettings)
        );
        if (!button) {
            log::warn("[RealMouseClick] Could not create pause-menu button");
            return;
        }

        menu->addChild(button);
        menu->setPosition({ winSize.width * 0.88f, winSize.height * 0.82f });
        menu->setID("real-mouse-click-settings-menu");
        this->addChild(menu, 1000);
    }

    void onRealMouseClickSettings(CCObject*) {
        // Opens Geode's native settings UI; enabled/volume values are saved
        // automatically by Geode and are shared with the touch sound hook.
        openSettingsPopup(Mod::get());
    }
};
