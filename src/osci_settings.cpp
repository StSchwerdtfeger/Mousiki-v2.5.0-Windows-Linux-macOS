#include "settings.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <ostream>
#include <sstream>

// The oscilloscope's settings: the rows of the SHIFT+O overlay (labels, values, stepping), the per-style parameter sets and
// their config.txt keys. Shared in spirit with the radio mode (radio_settings.cpp), which has the same rows.

namespace muisc {

const char* const kOsciPaletteNames[kOsciPaletteCount] = {"gradient", "settings", "temperature", "aurora", "magma", "ice", "neon", "spectrum"};

namespace {
const int kFpsSteps[4] = {30, 45, 60, 90};
const char* const kCellSteps[7] = {"", "8x16", "9x18", "10x20", "12x24", "14x28", "16x32"};
const char* const kProtoSteps[4] = {"auto", "kitty", "sixel", "off"};
template <class T, size_t N> int index_of(const T (&arr)[N], const std::string& v, int def) {
    for (size_t i = 0; i < N; ++i) if (v == arr[i]) return static_cast<int>(i);
    return def;
}
int step_in(int idx, int n, int dir) { return std::clamp(idx + (dir > 0 ? 1 : -1), 0, n - 1); }
std::string lower(std::string s) { for (char& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c))); return s; }
}

std::vector<int> osci_visible_rows(const Settings& s) {
    std::vector<int> r = {kOrDisplay, kOrStyle, kOrFps};
    if (s.osci_style == 1) { r.push_back(kOrProtocol); }
    r.push_back(kOrDecay);
    if (s.osci_style != 1) r.push_back(kOrDot);
    r.push_back(kOrTail);
    for (int id : {kOrInterp, kOrZ, kOrZDepth, kOrZSource, kOrTrace, kOrRotate, kOrMono, kOrPalette}) r.push_back(id);
    if (s.osci_style == 1) r.push_back(kOrGlow);
    return r;
}

std::string osci_row_label(int row) {
    static const char* const L[kOsciRowCount] = {"Decay", "Dot threshold", "Tail", "Line/Vec. Interpol.", "Z-Axis (XYZ Mode)",
        "Z Depth", "Z Source", "Trace Length", "Rotate 45 deg (M/S)", "Mono Phase Portrait", "Color", "Glow",
        "Osci style", "Osci cell pixels", "Image protocol", "Frame rate", "Display"};
    return row >= 0 && row < kOsciRowCount ? L[row] : "";
}

std::string osci_row_value(const Settings& s, int row) {
    char b[24];
    const OsciSet& o = s.osci();
    auto on = [](bool v) { return std::string(v ? "on" : "off"); };
    auto f2 = [&](double v) { std::snprintf(b, sizeof b, "%.2f", v); return std::string(b); };
    switch (row) {
        case kOrDecay: return f2(o.decay);
        case kOrDot: return f2(o.dot_threshold);
        case kOrTail: return f2(o.tail);
        case kOrInterp: return on(o.interp);
        case kOrZ: return on(o.z);
        case kOrZDepth: return f2(o.z_depth);
        case kOrZSource: return o.z_source == 1 ? "level" : "speed";
        case kOrTrace: return std::to_string(o.trace);
        case kOrRotate: return on(o.rotate);
        case kOrMono: return on(o.mono_phase);
        case kOrPalette: return kOsciPaletteNames[std::clamp(o.palette, 0, kOsciPaletteCount - 1)];
        case kOrGlow: return f2(o.glow);
        case kOrStyle: return s.osci_style == 1 ? "image" : "braille";
        case kOrCells: return s.cell_pixels.empty() ? "auto" : s.cell_pixels;
        case kOrProtocol: return s.gfx_protocol;
        case kOrFps: return std::to_string(s.frame_rate) + " fps";
        case kOrDisplay: return s.lyric_viz == 1 ? "osci" : "sphere";
    }
    return "";
}

void osci_adjust(Settings& s, int row, int dir) {
    OsciSet& o = s.osci();
    auto snap = [](double v, double step, double lo, double hi) { return static_cast<float>(std::clamp(std::round(v / step) * step, lo, hi)); };
    switch (row) {
        case kOrDecay: o.decay = snap(o.decay + dir * 0.01, 0.01, 0.0, 0.99); break;
        case kOrDot: o.dot_threshold = snap(o.dot_threshold + dir * 0.01, 0.01, 0.01, 1.0); break;
        case kOrTail: o.tail = snap(o.tail + dir * 0.01, 0.01, 0.0, 1.0); break;
        case kOrInterp: o.interp = dir > 0; break;
        case kOrZ: o.z = dir > 0; break;
        case kOrZDepth: o.z_depth = snap(o.z_depth + dir * 0.05, 0.05, 0.0, 1.0); break;
        case kOrZSource: o.z_source = dir > 0 ? 1 : 0; break;
        case kOrTrace: o.trace = std::clamp(o.trace + dir * 128, 128, 1024); break;
        case kOrRotate: o.rotate = dir > 0; break;
        case kOrMono: o.mono_phase = dir > 0; break;
        case kOrPalette: o.palette = step_in(o.palette, kOsciPaletteCount, dir); break;
        case kOrGlow: o.glow = snap(o.glow + dir * 0.05, 0.05, 0.0, 1.0); break;
        case kOrStyle: s.osci_style = dir > 0 ? 1 : 0; break;
        case kOrCells: s.cell_pixels = kCellSteps[step_in(index_of(kCellSteps, s.cell_pixels, 0), 7, dir)]; break;
        case kOrProtocol: s.gfx_protocol = kProtoSteps[step_in(index_of(kProtoSteps, s.gfx_protocol, 0), 4, dir)]; break;
        case kOrFps: {
            int i = 0;
            for (int k = 0; k < 4; ++k) if (kFpsSteps[k] == s.frame_rate) i = k;
            s.frame_rate = kFpsSteps[step_in(i, 4, dir)];
            break;
        }
        case kOrDisplay: s.lyric_viz = dir > 0 ? 0 : 1; break;
    }
}

void osci_reset(Settings& s) {   // the parameters of the style in use; the shared rows (style, protocol, cells, fps, display) stay
    s.osci() = OsciSet{};
    if (s.osci_style == 1) s.osci().glow = 0.60f;
}

namespace {
bool as_bool(const std::string& v, bool def) {
    const std::string l = lower(v);
    if (l == "true" || l == "1" || l == "yes" || l == "on") return true;
    if (l == "false" || l == "0" || l == "no" || l == "off") return false;
    return def;
}
float as_float(const std::string& v, float def, float lo, float hi) {
    try { return std::clamp(std::stof(v), lo, hi); } catch (...) { return def; }
}
int as_int(const std::string& v, int def, int lo, int hi) {
    try { return std::clamp(std::stoi(v), lo, hi); } catch (...) { return def; }
}
}

bool osci_config_key(Settings& s, const std::string& key, const std::string& value) {
    // Per style: braille = Osci<name> (the keys the player always had), image = OsciImage<name>.
    for (int st = 0; st < 2; ++st) {
        const std::string P = st == 0 ? "Osci" : "OsciImage";
        if (key.compare(0, P.size(), P) != 0) continue;
        const std::string n = key.substr(P.size());
        OsciSet& o = s.osci_set[st];
        if (n == "Decay") o.decay = as_float(value, o.decay, 0.0f, 0.99f);
        else if (n == "DotThreshold") o.dot_threshold = as_float(value, o.dot_threshold, 0.01f, 1.0f);
        else if (n == "TailBrightness") o.tail = as_float(value, o.tail, 0.0f, 1.0f);
        else if (n == "Interpolation") o.interp = as_bool(value, o.interp);
        else if (n == "ZAxis") o.z = as_bool(value, o.z);
        else if (n == "ZDepth") o.z_depth = as_float(value, o.z_depth, 0.0f, 1.0f);
        else if (n == "ZSource") o.z_source = lower(value) == "level" ? 1 : 0;
        else if (n == "TraceLength") o.trace = as_int(value, o.trace, 128, 1024);
        else if (n == "Rotate") o.rotate = as_bool(value, o.rotate);
        else if (n == "MonoPhase") o.mono_phase = as_bool(value, o.mono_phase);
        else if (n == "Glow") o.glow = as_float(value, o.glow, 0.0f, 1.0f);
        else if (n == "Palette") { const std::string l = lower(value); for (int i = 0; i < kOsciPaletteCount; ++i) if (l == kOsciPaletteNames[i]) o.palette = i; }
        else continue;
        return true;
    }
    if (key == "OsciStyle") { s.osci_style = lower(value) == "image" ? 1 : 0; return true; }
    if (key == "OsciImageProtocol") { s.gfx_protocol = lower(value); return true; }
    if (key == "FrameRate") { const int fr = as_int(value, s.frame_rate, 30, 90); s.frame_rate = fr >= 75 ? 90 : fr >= 52 ? 60 : fr >= 38 ? 45 : 30; return true; }
    return false;
}

void osci_config_write(std::ostream& out, const Settings& s) {
    auto num = [](float v) { std::ostringstream os; os.precision(2); os << std::fixed << v; return os.str(); };
    auto b = [](bool v) { return std::string(v ? "true" : "false"); };
    auto block = [&](const std::string& P, const OsciSet& o) {
        out << P << "Decay=" << num(o.decay) << "\n" << P << "DotThreshold=" << num(o.dot_threshold) << "\n"
            << P << "TailBrightness=" << num(o.tail) << "\n" << P << "Interpolation=" << b(o.interp) << "\n"
            << P << "ZAxis=" << b(o.z) << "\n" << P << "ZDepth=" << num(o.z_depth) << "\n"
            << P << "ZSource=" << (o.z_source == 1 ? "level" : "speed") << "\n" << P << "TraceLength=" << o.trace << "\n"
            << P << "Rotate=" << b(o.rotate) << "\n" << P << "MonoPhase=" << b(o.mono_phase) << "\n"
            << P << "Palette=" << kOsciPaletteNames[std::clamp(o.palette, 0, kOsciPaletteCount - 1)] << "\n"
            << P << "Glow=" << num(o.glow) << "\n";
    };
    out << "## Oscilloscope (SHIFT+O overlay), one block per style: braille = Osci<name>, image = OsciImage<name>. Each style keeps its own values.\n";
    out << "## Decay = afterglow 0.00-0.99 | DotThreshold 0.01-1.00 (braille: lower = thicker line) | TailBrightness 0.00-1.00 | Interpolation = connect the\n";
    out << "## samples with lines | ZAxis = beam intensity (ZDepth 0-1, ZSource speed|level) | TraceLength 128-1024 samples | Rotate = 45 degrees (mid vertical) |\n";
    out << "## MonoPhase = phase portrait for near-mono signals | Palette gradient|settings|temperature|aurora|magma|ice|neon|spectrum | Glow 0-1 (image bloom)\n";
    block("Osci", s.osci_set[0]);
    block("OsciImage", s.osci_set[1]);
    out << "## braille | image (a real pixel picture drawn by the terminal: Kitty graphics or Sixel; braille where there is neither)\n";
    out << "OsciStyle=" << (s.osci_style == 1 ? "image" : "braille") << "\n";
    out << "## image style: auto | kitty | sixel | off, and the pixels of one terminal cell (WxH, empty = ask the terminal)\n";
    out << "OsciImageProtocol=" << s.gfx_protocol << "\n";
    out << "## screen refresh: 30 | 45 | 60 | 90 frames per second (higher = more CPU and, for the image, more data for the terminal)\n";
    out << "FrameRate=" << s.frame_rate << "\n";
}

} // namespace muisc
