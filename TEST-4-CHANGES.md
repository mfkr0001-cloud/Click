# Real Mouse Click — Test 4 changes

- Read the Geode `float` volume setting as `float` and clamp it to 0.0–1.0 before playback.
- Check the sound resource with the non-throwing `std::filesystem` overload so a filesystem error cannot escape into touch handling.
- Check that the FMOD engine pointer is available before requesting playback.
- Keep the original Cocos2d touch dispatcher call first; emit sound only for touch-begin phase (`index == 0`).
- Update metadata, CMake project version, and Actions artifact name to Test 4.

## Still requiring device validation

This archive has not been compiled against the exact Geode SDK checkout and has not been tested in Geometry Dash on Android. GitHub Actions must confirm compilation, then an Android device test must confirm actual audio playback and normal touch behavior.

The included `click.wav` remains a generated temporary click sound, not the selected Pixabay recording.
