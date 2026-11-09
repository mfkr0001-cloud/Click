# Real Mouse Click — Test 5 changes

- Added a 25 ms duplicate-event guard around touch-begin sound playback. This is intended to suppress duplicate callbacks in one frame without noticeably delaying normal taps.
- Kept the original Geometry Dash touch dispatcher call first, preserving game input.
- Kept non-throwing audio-resource checks, FMOD availability checks, enable/disable and volume settings.
- Updated mod/CMake version to 1.0.5 and GitHub Actions artifact label to Test 5.

## Test focus

1. Build Android ARM64 with GitHub Actions.
2. Confirm one click sound per tap.
3. Tap rapidly and check that legitimate rapid taps still produce sound.
4. Check multi-touch, menus, gameplay and editor input.
5. Toggle the mod and test volume at 0%, 50% and 100%.

## Not yet verified

The ZIP is structurally checked, but this source has not been compiled against the user's exact Geode SDK checkout or tested on an Android device. The included click.wav is a generated temporary sound, not the selected Pixabay recording. Do not treat the mod as device-tested until the Actions build and phone test pass.
