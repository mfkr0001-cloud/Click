# Test 9 changes

## Main change
- Strengthens validation of the bundled WAV before playback.
- Checks the RIFF/WAVE signature, expected `fmt ` and `data` chunk positions, PCM format, channel count, sample rate, byte rate, block alignment, bit depth, and declared file/chunk sizes.
- Keeps the Test 8 behavior: one playback attempt per touch in the `CCSet` received by `touchesBegan`.
- Uses the touch-set's own count type in the loop.

## Preserved
- Android ARM64 workflow only.
- Geometry Dash 2.2081 / Geode SDK metadata 5.11.2.
- Enable/disable setting and volume slider.
- Volume clamping and FMOD availability diagnostics.
- No debounce.
- Existing temporary WAV resource.

## Validation limits
The supplied WAV's 44-byte PCM header was checked against the new validation rules. This ZIP has not been compiled by GitHub Actions and playback has not been verified on an Android device. The WAV remains a temporary test sound, not the selected Pixabay recording.
