# Test 6 changes

1. Reverted the touch hook to the explicit `touchesBegan(CCSet*, CCEvent*)` callback used by the initial implementation. Test 5's lower-level `touches(..., index)` override may be SDK/API-sensitive and needs actual compiler validation.
2. Removed the global 25 ms debounce. It could suppress legitimate rapid taps, so Test 6 prioritizes one sound per begin-touch callback.
3. Kept the original game's touch processing call before playing audio.
4. Kept enabled/volume settings, clamped volume, resource existence diagnostics, and FMOD availability diagnostics.
5. Updated mod version and artifact name to test6.

Not verified: Android compilation, runtime hook behavior, actual audio playback, and phone compatibility. The audio remains the temporary generated WAV.
