# Test 8 changes

1. Changes playback from once per `touchesBegan` callback to once per touch in the callback.
2. Keeps the original `CCTouchDispatcher::touchesBegan` call before the sound logic.
3. Keeps the Test 7 WAV existence and RIFF/WAVE header validation.
4. Keeps enabled/volume settings, volume clamping, FMOD availability diagnostics, and no debounce.
5. Updates the mod/CMake version and Android ARM64 artifact name to Test 8.

## Validation limits

The ZIP structure and WAV header can be checked locally. This does not prove that GitHub Actions compiles the mod or that FMOD playback works on a device. Test 8 must be built and installed on Android before runtime behavior is confirmed.

The bundled WAV is still a temporary test sound, not the selected Pixabay recording.
