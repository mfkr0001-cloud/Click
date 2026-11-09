# Real Mouse Click — Android Test 10

**Final planned test before the remaster.**

- Target: Geometry Dash 2.2081
- Geode SDK metadata: 5.11.2
- Platform: Android ARM64 only
- Mod ID: `mfkr0001.real-mouse-click`
- Version: `1.0.10-test10`

## Test 10 focus
Final strict validation of the bundled canonical 44-byte PCM WAV. The validator checks the PCM format, `fmt ` chunk size, channel count, bit depth, sample rate, block alignment, byte rate, RIFF-declared total size, and exact data payload size. Derived arithmetic uses 64-bit values to avoid overflow.

The touch hook retains the Test 8/9 behavior: it forwards the original touch event to Geometry Dash, then attempts one click sound for every touch in the received `CCSet`.

## Preserved
- Enable/disable setting.
- Volume slider from 0% to 100%, with clamping.
- FMOD availability diagnostics.
- No artificial debounce.
- Existing temporary WAV resource.
- Android ARM64 build workflow only.

## Build
Run GitHub Actions. Expected artifact: `RealMouseClick-Android64-test10`.

## Important
A valid archive and WAV file do not prove successful compilation or runtime playback. Review the GitHub Actions log, install the resulting `.geode` on Android, and test normal, rapid and multi-touch input before starting the remaster. The bundled WAV remains a temporary test sound, not the selected Pixabay recording.
