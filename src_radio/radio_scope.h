#pragma once
// The radio's XY oscilloscope: the music player's OscilloscopeVisualizer (same phosphor buffer, mono phase portrait and
// auto-gain) with more controls: line interpolation on/off, a Z axis (beam intensity), trace length, 45 degree rotation
// (M/S view), mono phase portrait on/off, and per-subpixel brightness + a "frequency" value per cell, so the UI can draw
// the picture as braille or as plain pixels and colour it by temperature. The player's own class stays untouched.
#include <array>
#include <cstdint>
#include <mutex>
#include <vector>

class RadioScope {
public:
    struct Cell {
        uint8_t braille = 0;                 // U+2800 dot pattern, 0 = empty cell
        uint8_t level = 0;                   // 0..255 brightness of the brightest subpixel
        std::array<uint8_t, 8> sub{};        // brightness per subpixel (index = row * 2 + column, 0 below the dot threshold)
        uint8_t hue = 0;                     // 0..255 "frequency" of the brightest subpixel (beam speed: slow 0 .. fast 255)
    };
    struct Params {
        float decay = 0.80f;                 // afterglow per frame
        float dot_threshold = 0.28f;         // brightness a subpixel needs to light
        float tail_brightness = 0.45f;       // oldest sample of the trace relative to the newest
        bool interpolate = true;             // connect the samples with lines (off: only the sample dots)
        bool z_axis = false;                 // beam intensity follows the Z signal
        float z_depth = 0.70f;               // how strongly Z darkens the beam, 0 .. 1
        int z_source = 0;                    // 0 = beam speed (fast = dim, like a CRT), 1 = signal level (far from centre = bright)
        int trace = 1024;                    // samples drawn per frame, 128 .. 1024
        bool rotate = false;                 // turn the picture by 45 degrees: mid on the vertical, side on the horizontal axis
        bool mono_phase = true;              // near-mono signals cross-fade to a phase portrait (else a diagonal line)
        float glow = 0.60f;                  // render_image(): strength / size of the bloom around the beam, 0 .. 1
    };
    void push_frames(const float* interleaved_lr, size_t frames);
    void reset();
    std::vector<std::vector<Cell>> render(int cols, int rows, const Params& params);
    // w x h pixel picture: `level` = brightness 0..255 (bloom included), `hue` = the "frequency" 0..255 of the nearest beam pixel.
    void render_image(int w, int h, const Params& params, std::vector<uint8_t>& level, std::vector<uint8_t>& hue);
private:
    void paint(int pw, int ph, const Params& params);   // beam -> glow_ / hue_ (render thread)
    static constexpr int kRingFrames = 2048;
    static constexpr int kWindow = 1024;
    std::mutex mtx_;
    std::array<float, kRingFrames> ch0_{};
    std::array<float, kRingFrames> ch1_{};
    int write_ = 0;
    float peak_ = 0.05f;
    bool cleared_ = false;
    std::vector<float> glow_;
    std::vector<float> hue_;
    int glow_w_ = 0, glow_h_ = 0;
    float mono_ = 0.0f;
    float scale_ = 0.0f;
    float hue_lo_ = 0.25f, hue_hi_ = 0.85f;   // smoothed range of the beam speeds, so every palette gets used
};
