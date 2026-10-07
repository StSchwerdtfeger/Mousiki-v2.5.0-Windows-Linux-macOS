#include "radio_scope.h"
#include <algorithm>
#include <cmath>

namespace {

// kBrailleMap[y][x] = bit of that subpixel inside a 2x4 braille cell.
const uint8_t kBrailleMap[4][2] = {
    {0x01, 0x08}, {0x02, 0x10}, {0x04, 0x20}, {0x40, 0x80}
};

// Mono detection: energy ratio (L-R)^2 / (L+R)^2 over the drawn window.
// Below kMonoFull the scope is fully in phase-portrait mode, above
// kMonoNone it is a true XY scope, in between the two are cross-faded.
constexpr float kMonoFull = 0.003f; // about -25 dB of side energy
constexpr float kMonoNone = 0.020f; // about -17 dB
constexpr int kDeriv = 3;           // half-width of the central difference for ds/dt

} // namespace

void RadioScope::push_frames(const float* interleaved_lr, size_t frames) {
    if (!interleaved_lr || frames == 0) return;
    if (frames > static_cast<size_t>(kRingFrames)) {
        interleaved_lr += 2 * (frames - static_cast<size_t>(kRingFrames));
        frames = static_cast<size_t>(kRingFrames);
    }
    std::lock_guard<std::mutex> lk(mtx_);
    for (size_t i = 0; i < frames; ++i) {
        ch0_[write_] = interleaved_lr[2 * i];
        ch1_[write_] = interleaved_lr[2 * i + 1];
        write_ = (write_ + 1) % kRingFrames;
    }
}

void RadioScope::reset() {
    std::lock_guard<std::mutex> lk(mtx_);
    ch0_.fill(0.0f);
    ch1_.fill(0.0f);
    write_ = 0;
    peak_ = 0.05f;
    cleared_ = true; // glow_ belongs to the render thread; it wipes it on its next frame
}

void RadioScope::paint(int pw, int ph, const Params& params) {
    const float decay = std::clamp(params.decay, 0.0f, 0.99f);
    const float tail = std::clamp(params.tail_brightness, 0.0f, 1.0f);

    const int kWin = std::clamp(params.trace, 128, kWindow);   // samples drawn this frame (the newest ones)
    // --- snapshot the newest kWin frames, oldest -> newest -------------
    std::vector<float> xs(kWin), ys(kWin);
    float peak;
    bool wipe;
    {
        std::lock_guard<std::mutex> lk(mtx_);
        for (int i = 0; i < kWin; ++i) {
            const int idx = (write_ - kWin + i + kRingFrames) % kRingFrames;
            xs[i] = ch0_[idx];
            ys[i] = ch1_[idx];
        }
        peak = peak_;
        wipe = cleared_;
        cleared_ = false;
    }

    // --- phosphor buffer: resize / wipe / decay --------------------------
    if (wipe) { mono_ = 0.0f; scale_ = 0.0f; }
    if (glow_w_ != pw || glow_h_ != ph || wipe) {
        glow_.assign(static_cast<size_t>(pw) * ph, 0.0f);
        hue_.assign(static_cast<size_t>(pw) * ph, 0.0f);
        glow_w_ = pw;
        glow_h_ = ph;
    } else {
        for (float& v : glow_) v *= decay;
    }

    // --- mono detection + phase-portrait cross-fade ---------------------
    {
        double em = 0.0, es = 0.0;
        for (int i = 0; i < kWin; ++i) {
            const double m = xs[i] + ys[i], s = xs[i] - ys[i];
            em += m * m;
            es += s * s;
        }
        const float ratio = static_cast<float>(es / (em + 1e-9));
        const float target = params.mono_phase ? std::clamp((kMonoNone - ratio) / (kMonoNone - kMonoFull), 0.0f, 1.0f) : 0.0f;
        mono_ += (target - mono_) * 0.25f;

        if (mono_ > 0.001f) {
            std::vector<float> mid(kWin), q(kWin);
            for (int i = 0; i < kWin; ++i) mid[i] = 0.5f * (xs[i] + ys[i]);
            double rs = 0.0, rq = 0.0;
            for (int i = 0; i < kWin; ++i) {
                const int a = std::max(i - kDeriv, 0), b = std::min(i + kDeriv, kWin - 1);
                q[i] = mid[b] - mid[a];
                rs += static_cast<double>(mid[i]) * mid[i];
                rq += static_cast<double>(q[i]) * q[i];
            }
            // Scale ds/dt so its rms equals the signal's rms: a pure tone
            // then draws a circle whatever its pitch.
            const float want = static_cast<float>(std::sqrt(rs / kWin) / std::max(std::sqrt(rq / kWin), 1e-6));
            const float clamped = std::min(want, 400.0f);
            scale_ = scale_ <= 0.0f ? clamped : scale_ + (clamped - scale_) * 0.2f;
            for (int i = 0; i < kWin; ++i) {
                xs[i] = xs[i] + (mid[i] - xs[i]) * mono_;
                ys[i] = ys[i] + (q[i] * scale_ - ys[i]) * mono_;
            }
        }
    }

    // --- 45 degree rotation: side (L-R) on the horizontal, mid (L+R) on the vertical axis ------------
    if (params.rotate) {
        constexpr float r2 = 0.70710678f;
        for (int i = 0; i < kWin; ++i) {
            const float a = xs[i], b = ys[i];
            xs[i] = (a - b) * r2;
            ys[i] = (a + b) * r2;
        }
    }

    // --- auto-gain (fast attack, slow release) ---------------------------
    float frame_peak = 0.0f;
    for (int i = 0; i < kWin; ++i)
        frame_peak = std::max(frame_peak, std::max(std::fabs(xs[i]), std::fabs(ys[i])));
    const float gain = 1.0f / std::max(peak, 0.05f);
    {
        std::lock_guard<std::mutex> lk(mtx_);
        peak_ = frame_peak > peak ? frame_peak : peak * 0.97f + frame_peak * 0.03f;
    }

    // --- draw the beam ---------------------------------------------------
    const int side = std::min(pw, ph);
    const float cx = (pw - 1) * 0.5f, cy = (ph - 1) * 0.5f;
    const float amp = (side - 1) * 0.5f * 0.94f;

    float cur_hue = 0.0f;   // "frequency" of the segment being drawn
    auto plot = [&](int x, int y, float v) {
        if (x < 0 || x >= pw || y < 0 || y >= ph || v <= 0.0f) return;
        const size_t at = static_cast<size_t>(y) * pw + x;
        float& g = glow_[at];
        if (v >= g) hue_[at] = cur_hue;
        g = std::max(g, std::min(v, 1.0f));
    };
    // Xiaolin Wu anti-aliased line; `i0`/`i1` = beam brightness at the ends.
    auto wu = [&](float x0, float y0, float x1, float y1, float i0, float i1) {
        const bool steep = std::fabs(y1 - y0) > std::fabs(x1 - x0);
        if (steep) { std::swap(x0, y0); std::swap(x1, y1); }
        if (x0 > x1) { std::swap(x0, x1); std::swap(y0, y1); std::swap(i0, i1); }
        auto put = [&](int a, int b, float cov, float inten) {
            if (steep) plot(b, a, cov * inten); else plot(a, b, cov * inten);
        };
        const float dx = x1 - x0, dy = y1 - y0;
        if (dx < 1e-4f) { // a single point (silence / stationary beam)
            put(static_cast<int>(std::lround(x0)), static_cast<int>(std::lround(y0)), 1.0f, i1);
            return;
        }
        const float grad = dy / dx;
        const int xa = static_cast<int>(std::lround(x0));
        const int xb = static_cast<int>(std::lround(x1));
        for (int x = xa; x <= xb; ++x) {
            const float t = (x1 > x0) ? std::clamp((x - x0) / dx, 0.0f, 1.0f) : 0.0f;
            const float inten = i0 + (i1 - i0) * t;
            const float y = y0 + grad * (x - x0);
            const int yi = static_cast<int>(std::floor(y));
            const float f = y - yi;
            put(x, yi, 1.0f - f, inten);
            put(x, yi + 1, f, inten);
        }
    };

    // Per sample: position, beam speed (-> Z and the "frequency" colour). Speed is in picture units (-1..1) per sample;
    // a full-scale sine of f Hz moves about 2*pi*f/rate per sample, so log(speed) is a usable pitch scale (40 Hz .. 8 kHz).
    std::vector<float> px_(kWin), py_(kWin), vx_(kWin), vy_(kWin);
    for (int i = 0; i < kWin; ++i) {
        vx_[i] = std::clamp(xs[i] * gain, -1.0f, 1.0f);
        vy_[i] = std::clamp(ys[i] * gain, -1.0f, 1.0f);
        px_[i] = cx + vx_[i] * amp;
        py_[i] = cy - vy_[i] * amp;   // +R is up
    }
    const int z_src = params.z_source;
    const float z_depth = std::clamp(params.z_depth, 0.0f, 1.0f);
    // The "frequency" colour: beam speed on a log scale, stretched over the range the signal really uses (5 % .. 95 % of this
    // frame's speeds, smoothed), so a pure tone and a busy mix both sweep through the whole palette instead of sitting at one end.
    std::vector<float> ht(kWin, 0.0f);
    {
        for (int i = 1; i < kWin; ++i) {
            const float sp = std::hypot(vx_[i] - vx_[i - 1], vy_[i] - vy_[i - 1]);
            ht[i] = std::clamp((std::log10(std::max(sp, 1e-4f)) + 2.3f) / 2.3f, 0.0f, 1.0f);
        }
        std::vector<float> sorted(ht.begin() + 1, ht.end());
        std::sort(sorted.begin(), sorted.end());
        const float lo = sorted[sorted.size() * 5 / 100], hi = sorted[sorted.size() * 95 / 100];
        hue_lo_ += (lo - hue_lo_) * 0.08f;
        hue_hi_ += (std::max(hi, lo + 0.12f) - hue_hi_) * 0.08f;
    }
    float hue_s = 0.0f;
    float px = cx, py = cy, pi = tail;
    for (int i = 0; i < kWin; ++i) {
        const float x = px_[i], y = py_[i];
        float inten = tail + (1.0f - tail) * (static_cast<float>(i) / (kWin - 1));
        const float sp = i > 0 ? std::hypot(vx_[i] - vx_[i - 1], vy_[i] - vy_[i - 1]) : 0.0f;
        if (params.z_axis) {
            const float z = z_src == 1 ? std::clamp(std::hypot(vx_[i], vy_[i]) * 0.70710678f * 1.4f, 0.0f, 1.0f)
                                       : 1.0f / (1.0f + 10.0f * sp);
            inten *= (1.0f - z_depth) + z_depth * z;
        }
        const float hn = std::clamp((ht[i] - hue_lo_) / std::max(0.05f, hue_hi_ - hue_lo_), 0.0f, 1.0f);
        hue_s += (hn - hue_s) * 0.10f;
        cur_hue = hue_s;
        if (params.interpolate && i > 0) wu(px, py, x, y, pi, inten);
        else wu(x, y, x, y, inten, inten);
        px = x; py = y; pi = inten;
    }

}

std::vector<std::vector<RadioScope::Cell>>
RadioScope::render(int cols, int rows, const Params& params) {
    const float dot_threshold = std::clamp(params.dot_threshold, 0.01f, 1.0f);
    std::vector<std::vector<Cell>> out(std::max(0, rows));
    for (auto& r : out) r.assign(std::max(0, cols), Cell{});
    if (cols <= 0 || rows <= 0) return out;
    const int pw = cols * 2, ph = rows * 4;
    paint(pw, ph, params);

    // --- pack the buffer into cells -------------------------------------
    for (int ty = 0; ty < rows; ++ty) {
        for (int tx = 0; tx < cols; ++tx) {
            uint8_t pattern = 0;
            float level = 0.0f, hue = 0.0f;
            Cell& cell = out[ty][tx];
            for (int yy = 0; yy < 4; ++yy) {
                for (int xx = 0; xx < 2; ++xx) {
                    const size_t at = static_cast<size_t>(ty * 4 + yy) * pw + tx * 2 + xx;
                    const float v = glow_[at];
                    if (v >= dot_threshold) {
                        pattern |= kBrailleMap[yy][xx];
                        cell.sub[static_cast<size_t>(yy * 2 + xx)] = static_cast<uint8_t>(std::lround(std::clamp(v, 0.0f, 1.0f) * 255.0f));
                        if (v >= level) { level = v; hue = hue_[at]; }
                    }
                }
            }
            cell.braille = pattern;
            cell.level = static_cast<uint8_t>(std::lround(std::clamp(level, 0.0f, 1.0f) * 255.0f));
            cell.hue = static_cast<uint8_t>(std::lround(std::clamp(hue, 0.0f, 1.0f) * 255.0f));
        }
    }
    return out;
}

// A true pixel picture: the same beam, drawn on a w x h grid, then a soft bloom around every lit pixel (a bright core,
// a dimmer rim, a faint halo) so the line has the look of a phosphor trace instead of a 1 pixel wire.
void RadioScope::render_image(int w, int h, const Params& params, std::vector<uint8_t>& level, std::vector<uint8_t>& hue) {
    level.assign(static_cast<size_t>(std::max(0, w)) * std::max(0, h), 0);
    hue.assign(level.size(), 0);
    if (w <= 0 || h <= 0) return;
    paint(w, h, params);
    std::vector<float> out(level.size(), 0.0f);
    std::vector<float> best(level.size(), 0.0f);   // strongest contribution so far, to pick the hue from
    constexpr int R = 2;
    const float gk = std::clamp(params.glow, 0.0f, 1.0f) / 0.60f;   // 0.60 = the original bloom
    static const float kW[2 * R + 1][2 * R + 1] = {
        {0.05f, 0.12f, 0.18f, 0.12f, 0.05f},
        {0.12f, 0.45f, 0.62f, 0.45f, 0.12f},
        {0.18f, 0.62f, 1.00f, 0.62f, 0.18f},
        {0.12f, 0.45f, 0.62f, 0.45f, 0.12f},
        {0.05f, 0.12f, 0.18f, 0.12f, 0.05f}};
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            const size_t at = static_cast<size_t>(y) * w + x;
            const float g = glow_[at];
            if (g < 0.02f) continue;
            for (int dy = -R; dy <= R; ++dy) {
                const int yy = y + dy;
                if (yy < 0 || yy >= h) continue;
                for (int dx = -R; dx <= R; ++dx) {
                    const int xx = x + dx;
                    if (xx < 0 || xx >= w) continue;
                    const size_t to = static_cast<size_t>(yy) * w + xx;
                    const float kw = (dx == 0 && dy == 0) ? 1.0f : std::min(1.0f, kW[dy + R][dx + R] * gk);
                    const float v = g * kw;
                    if (v > out[to]) out[to] = v;
                    if (v > best[to]) { best[to] = v; hue[to] = static_cast<uint8_t>(std::lround(std::clamp(hue_[at], 0.0f, 1.0f) * 255.0f)); }
                }
            }
        }
    }
    for (size_t i = 0; i < out.size(); ++i) level[i] = static_cast<uint8_t>(std::lround(std::clamp(out[i], 0.0f, 1.0f) * 255.0f));
}
