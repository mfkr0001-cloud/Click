# Real Mouse Click — Test 10 (final pre-remaster test)

## Changes from Test 9
- Final strict validation for the project's current canonical 44-byte PCM WAV layout.
- Requires PCM format code 1 and a 16-byte `fmt ` chunk.
- Verifies channel count, supported bit depth, sample rate, block alignment and byte rate.
- Verifies the RIFF-declared file size matches the actual file size.
- Verifies the data chunk exactly matches the remaining file payload and is aligned to complete PCM frames.
- Uses 64-bit arithmetic for derived size/rate checks to avoid 32-bit overflow.

## Preserved
- Android ARM64 GitHub Actions workflow.
- Geometry Dash 2.2081 and Geode SDK metadata 5.11.2.
- Mod ID `mfkr0001.real-mouse-click`.
- Per-touch playback for each touch in the `CCSet` received by `touchesBegan`.
- Enable/disable and volume settings.
- FMOD availability diagnostics.
- No debounce.
- Existing temporary WAV file.

## Validation status
The bundled WAV header and declared sizes were checked against the Test 10 rules. This archive has not been compiled by GitHub Actions, and audio playback has not been verified on a physical Android device. The bundled WAV is still a temporary test sound, not the selected Pixabay recording.

Test 10 is the last planned test before the remaster. The remaster should begin only after reviewing the Test 10 build log and device test results.
