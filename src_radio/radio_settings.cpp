#include "radio_settings.h"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <map>
#include <sstream>

namespace muisc::radio {

namespace fs = std::filesystem;

// ===========================================================================================
// COLORS tab schema
// ===========================================================================================
const ColorRowSpec kColorRows[] = {
    {"BORDER_COLOR", "TOP",          &RadioSettings::border_top,         "BOTTOM",         &RadioSettings::border_bottom},
    {"ON_AIR",       "UPPER_LEFT",   &RadioSettings::on_air_upper_left,  "BOTTOM_RIGHT",   &RadioSettings::on_air_bottom_right},
    {"METADATA",     "KEY",          &RadioSettings::meta_key,           "VAL",            &RadioSettings::meta_val},
    {"VIZ",          "LEFT",         &RadioSettings::viz_left,           "RIGHT",          &RadioSettings::viz_right},
    {"OSCI",         "LEFT",         &RadioSettings::osci_left,          "RIGHT",          &RadioSettings::osci_right},
    {"FREQUENCY_BAR","LINE",         &RadioSettings::freq_line,          "MHz",            &RadioSettings::freq_mhz},
    {"VOLUME",       "CURRENT",      &RadioSettings::volume_current,     "POSSIBLE",       &RadioSettings::volume_possible},
    // ---- divider ----
    {"LIST",         "INACTIVE  FG", &RadioSettings::list_fg,            "BG",             &RadioSettings::list_bg},
    {"",             "PLAYING   FG", &RadioSettings::list_playing_fg,    "BG",             &RadioSettings::list_playing_bg},
    {"",             "CURSOR    FG", &RadioSettings::list_cursor_fg,     "BG",             &RadioSettings::list_cursor_bg},
    {"PRESETS",      "INACTIVE  FG", &RadioSettings::preset_inactive_fg, "BG",             &RadioSettings::preset_inactive_bg},
    {"",             "KEY       FG", &RadioSettings::preset_key_fg,      nullptr,          nullptr},
    {"HEADER",       "TEXT",         &RadioSettings::header,             nullptr,          nullptr},
    {"LEGEND",       "TEXT",         &RadioSettings::legend,             nullptr,          nullptr},
    {"TAB_NAMES",    "CURRENT",      &RadioSettings::tab_current,        "OTHER",          &RadioSettings::tab_other},
};
const int kColorRowCount = static_cast<int>(sizeof(kColorRows) / sizeof(kColorRows[0]));

const OnOffRowSpec kOnOffRows[] = {
    {"On air ascii",     &RadioSettings::on_air_ascii,    nullptr},
    {"Pulse wave",       &RadioSettings::pulse_wave,      nullptr},
    {"Dummy buttons",    &RadioSettings::dummy_buttons,   nullptr},
    {"Osci/sphere",      nullptr,                         &RadioSettings::scope_mode},
    {"Visualizer",       &RadioSettings::element_visualizer, nullptr},
    {"Stereo sound",     &RadioSettings::stereo,          nullptr},
    {"Normalize volume", &RadioSettings::normalize,       nullptr},
    {"Tuning noise",     &RadioSettings::tune_noise,      nullptr},
    {"Osci style",       nullptr,                         &RadioSettings::osci_style},
};
const int kOnOffRowCount = static_cast<int>(sizeof(kOnOffRows) / sizeof(kOnOffRows[0]));

std::string on_off_value(const RadioSettings& s, int row) {
    if (row < 0 || row >= kOnOffRowCount) return "";
    const OnOffRowSpec& r = kOnOffRows[row];
    if (r.flag) return (s.*(r.flag)) ? "true" : "false";
    const int v = s.*(r.choice);
    if (r.choice == &RadioSettings::osci_style) return v == 1 ? "image" : "braille";
    return v == 1 ? "sphere" : v == 2 ? "off" : "osci";
}

void on_off_change(RadioSettings& s, int row, int dir) {
    if (row < 0 || row >= kOnOffRowCount) return;
    const OnOffRowSpec& r = kOnOffRows[row];
    if (r.flag) {
        bool& b = s.*(r.flag);
        b = dir == 0 ? !b : dir > 0;
    } else {
        int& v = s.*(r.choice);
        const int n = r.choice == &RadioSettings::osci_style ? 2 : 3;
        v = dir == 0 ? (v + 1) % n : (v + (dir > 0 ? 1 : n - 1)) % n;
    }
}

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
}

std::vector<int> osci_visible_rows(const RadioSettings& s) {
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
std::string osci_row_value(const RadioSettings& s, int row) {
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
        case kOrDisplay: return s.scope_mode == 1 ? "sphere" : "osci";
    }
    return "";
}
void osci_adjust(RadioSettings& s, int row, int dir) {
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
        case kOrDisplay: s.scope_mode = dir > 0 ? 1 : 0; break;
    }
}
void osci_reset(RadioSettings& s) {   // the parameters of the style in use; the shared rows (style, protocol, cells, fps, display) stay
    s.osci() = OsciSet{};
    if (s.osci_style == 1) s.osci().glow = 0.60f;
}
void norm_adjust(RadioSettings& s, int row, int dir) {
    if (row == 0) { s.normalize = dir > 0; return; }
    double* v = row == 1 ? &s.normalize_target_lufs : &s.normalize_max_boost_db;
    const Knob& k = kNormKnobs[std::clamp(row, 0, 2)];
    *v = std::clamp(std::round((*v + dir * k.step) / k.step) * k.step, k.lo, k.hi);
}
void norm_reset(RadioSettings& s) {
    const RadioSettings d;
    s.normalize_target_lufs = d.normalize_target_lufs; s.normalize_max_boost_db = d.normalize_max_boost_db;
}

const char* const kAnimRows[] = {"Vis. Fluidity", "Gradient wave speed", "Pulse wave speed", "Playback mode",
                                 "Vis. Degradation", "Vis. Viscosity"};
const int kAnimRowCount = 6;

namespace {
// One scale for both wave speeds (cycles / loops per second). Contains both defaults (0.25 colour wave, 0.125 pulse).
constexpr double kSpeedSteps[] = {0.01, 0.03, 0.05, 0.10, 0.125, 0.17, 0.25, 0.35, 0.50, 0.75, 1.00};
constexpr int kSpeedStepCount = static_cast<int>(sizeof(kSpeedSteps) / sizeof(kSpeedSteps[0]));
int nearest_step(double v) {
    int best = 0;
    for (int i = 1; i < kSpeedStepCount; ++i)
        if (std::fabs(kSpeedSteps[i] - v) < std::fabs(kSpeedSteps[best] - v)) best = i;
    return best;
}
void cycle_int(int& v, int lo, int hi, int dir) {
    const int n = hi - lo + 1;
    v = lo + ((v - lo + (dir == 0 ? 1 : dir) + n) % n + n) % n;
}
void cycle_speed(double& v, int dir) {
    const int i = nearest_step(v);
    v = kSpeedSteps[((i + (dir == 0 ? 1 : dir)) % kSpeedStepCount + kSpeedStepCount) % kSpeedStepCount];
}
std::string fmt_speed(double v) {
    std::ostringstream os;
    os << v;
    return os.str();
}
} // namespace

std::string anim_value(const RadioSettings& s, int row) {
    switch (row) {
        case 0: return std::to_string(s.visualizer_fluidity);
        case 1: return fmt_speed(kSpeedSteps[nearest_step(s.gradient_wave_speed)]);
        case 2: return fmt_speed(kSpeedSteps[nearest_step(s.pulse_wave_speed)]);
        case 3: return s.playback_shuffle ? "shuffle" : "list";
        case 4: return std::to_string(s.visualizer_degradation_speed);
        case 5: return std::to_string(s.visualizer_viscosity);
    }
    return "";
}

void anim_change(RadioSettings& s, int row, int dir) {
    switch (row) {
        case 0: cycle_int(s.visualizer_fluidity, 1, 10, dir); break;
        case 1: cycle_speed(s.gradient_wave_speed, dir); break;
        case 2: cycle_speed(s.pulse_wave_speed, dir); break;
        case 3: s.playback_shuffle = !s.playback_shuffle; break;
        case 4: cycle_int(s.visualizer_degradation_speed, 1, 10, dir); break;
        case 5: cycle_int(s.visualizer_viscosity, 0, 10, dir); break;
    }
}

const int kPathRowCount = 5;
const char* const kPathRows[] = {"STATION LISTS PATH", "PRESETS PATH", "DOWNLOAD PATH", "Same folder as music player", "HISTORY PATH"};

std::string* path_row_text(RadioSettings& s, int row) {
    switch (row) {
        case 0: return &s.stations_path;
        case 1: return &s.presets_path;
        case 2: return &s.download_path;
        case 4: return &s.history_path;
    }
    return nullptr;
}
const std::string* path_row_text(const RadioSettings& s, int row) { return path_row_text(const_cast<RadioSettings&>(s), row); }

std::string* color_field(RadioSettings& s, int row, int col) {
    if (row < 0 || row >= kColorRowCount) return nullptr;
    const ColorRowSpec& r = kColorRows[row];
    if (col == 0) return &(s.*(r.field1));
    if (col == 1 && r.field2) return &(s.*(r.field2));
    return nullptr;
}
const std::string* color_field(const RadioSettings& s, int row, int col) {
    return color_field(const_cast<RadioSettings&>(s), row, col);
}

// ===========================================================================================
// colour helpers
// ===========================================================================================
namespace {

int color_index(const std::string& v) {
    if (v.empty()) return 0;
    try { return std::stoi(v); } catch (...) { return 0; }
}

struct RGB { int r, g, b; };

RGB palette_rgb(int idx) {
    static const RGB basic16[16] = {
        {0,0,0}, {128,0,0}, {0,128,0}, {128,128,0}, {0,0,128}, {128,0,128}, {0,128,128}, {192,192,192},
        {128,128,128}, {255,0,0}, {0,255,0}, {255,255,0}, {0,0,255}, {255,0,255}, {0,255,255}, {255,255,255}};
    if (idx >= 0 && idx < 16) return basic16[idx];
    if (idx >= 16 && idx <= 231) {                       // 6x6x6 cube
        const int ci = idx - 16;
        auto v = [](int c) { return c == 0 ? 0 : 55 + 40 * c; };
        return {v(ci / 36), v((ci / 6) % 6), v(ci % 6)};
    }
    if (idx >= 232 && idx <= 255) { const int g = 8 + (idx - 232) * 10; return {g, g, g}; }   // greys
    return {255, 255, 255};
}

bool parse_rgb(const std::string& v, RGB& out) {
    const int idx = color_index(v);
    if (idx <= 0 || idx > 255) return false;             // none has no RGB
    out = palette_rgb(idx);
    return true;
}

} // namespace

std::string ansi_fg(const std::string& color) {
    const int v = color_index(color);
    if (v <= 0) return "";
    if ((v >= 30 && v <= 47) || (v >= 90 && v <= 107)) return "\x1b[" + std::to_string(v) + "m";
    return "\x1b[38;5;" + std::to_string(v) + "m";
}

std::string ansi_bg(const std::string& color) {
    const int v = color_index(color);
    if (v <= 0) return "";
    if ((v >= 30 && v <= 47) || (v >= 90 && v <= 107)) return "\x1b[" + std::to_string(v) + "m";   // direct codes ignore fg/bg, like the player
    return "\x1b[48;5;" + std::to_string(v) + "m";
}

std::string sgr_params(const std::string& color) {
    const int v = color_index(color);
    if (v <= 0) return "";
    if ((v >= 30 && v <= 47) || (v >= 90 && v <= 107)) return std::to_string(v);
    return "38;5;" + std::to_string(v);
}

std::string gradient_fg(const std::string& start, const std::string& end, float t) {
    if (end.empty()) return ansi_fg(start);
    RGB a, b;
    if (!parse_rgb(start, a) || !parse_rgb(end, b)) return ansi_fg(start);
    t = std::clamp(t, 0.0f, 1.0f);
    const int r = static_cast<int>(a.r + (b.r - a.r) * t);
    const int g = static_cast<int>(a.g + (b.g - a.g) * t);
    const int bl = static_cast<int>(a.b + (b.b - a.b) * t);
    return "\x1b[38;2;" + std::to_string(r) + ";" + std::to_string(g) + ";" + std::to_string(bl) + "m";
}

std::string gradient3_fg(const std::string& left, const std::string& center, const std::string& right, float t) {
    if (center.empty()) return gradient_fg(left, right, t);
    t = std::clamp(t, 0.0f, 1.0f);
    return t <= 0.5f ? gradient_fg(left, center, t * 2.0f) : gradient_fg(center, right, (t - 0.5f) * 2.0f);
}

std::string gradient_bar(const std::string& start, const std::string& end, int width) {
    std::string out;
    for (int i = 0; i < width; ++i)
        out += gradient_fg(start, end, width > 1 ? static_cast<float>(i) / static_cast<float>(width - 1) : 0.0f) + "█";
    return out + "\x1b[0m";
}

bool valid_color_value(const std::string& v) {
    if (v.empty()) return true;
    for (unsigned char c : v) if (!std::isdigit(c)) return false;
    if (v.size() > 3) return false;
    return std::stoi(v) <= 255;
}

// ===========================================================================================
// config file
// ===========================================================================================
namespace {

std::string trim(const std::string& s) {
    const size_t a = s.find_first_not_of(" \t\r\n");
    if (a == std::string::npos) return "";
    return s.substr(a, s.find_last_not_of(" \t\r\n") - a + 1);
}

std::string lower(std::string s) {
    for (auto& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return s;
}

fs::path home_config_path() {
    const char* home = std::getenv("HOME");
    if (!home || !*home) home = std::getenv("USERPROFILE");
    if (home && *home) return fs::path(home) / ".config" / "mousiki" / "radio_config.txt";
    return fs::path("radio_config.txt");
}

bool as_bool(const std::string& v, bool def) {
    const std::string l = lower(v);
    if (l == "true" || l == "1" || l == "on" || l == "yes") return true;
    if (l == "false" || l == "0" || l == "off" || l == "no") return false;
    return def;
}

int as_int(const std::string& v, int def, int lo, int hi) {
    try { return std::clamp(std::stoi(v), lo, hi); } catch (...) { return def; }
}

float as_float(const std::string& v, float def, float lo, float hi) {
    try { return std::clamp(std::stof(v), lo, hi); } catch (...) { return def; }
}

} // namespace

fs::path radio_default_dir() {
    const char* home = std::getenv("HOME");
    if (!home || !*home) home = std::getenv("USERPROFILE");
    if (home && *home) return fs::path(home) / ".config" / "mousiki";
    return fs::path(".");
}

std::string player_download_folder() {
    // Read-only peek at the player's own config.txt; the radio has no other connection to the player's settings.
    std::ifstream in(radio_default_dir() / "config.txt");
    std::string line;
    while (std::getline(in, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;
        const size_t eq = line.find('=');
        if (eq == std::string::npos) continue;
        const std::string key = lower(trim(line.substr(0, eq)));
        if (key == "downloadfolder" || key == "download_folder") return trim(line.substr(eq + 1));
    }
    return "";
}


// ---- REFERENCE ----------------------------------------------------------------------------------------------
const KeyAction kKeyActions[] = {
    {"PLAYBACK", "Next", "Next Channel", "n"},
    {nullptr, "Shuffle", "Random Channel", "#"},
    {nullptr, "Previous", "Previous Channel", "b"},
    {nullptr, "Mode", "Surf Mode S / L", "m"},
    {nullptr, "Mute", "Mute / Unmute", "p"},
    {nullptr, "Stop", "Stop Stream", "x"},
    {nullptr, "Reconnect", "Reconnect", "R"},
    {nullptr, "VolumeUp", "Volume Up", "+"},
    {nullptr, "VolumeDown", "Volume Down", "-"},
    {nullptr, "Normalize", "Toggle Normalization", "v"},
    {nullptr, "Record", "Record Stream", "y"},
    {nullptr, "SleepTimer", "Sleep Timer", "Z"},
    {"MENUS", "Search", "Search Stations", "/"},
    {nullptr, "Sort", "Toggle Sort A-Z", "T"},
    {nullptr, "Presets", "Open Presets Menu", "K"},
    {nullptr, "Browser", "Open Radio Browser", "S"},
    {nullptr, "Lists", "Open Station Lists", "P"},
    {nullptr, "History", "Open History", "h"},
    {nullptr, "Stations", "Big Stations Overlay", "L"},
    {nullptr, "OsciMenu", "Oscilloscope Overlay", "O"},
    {nullptr, "NormMenu", "Normalization Overlay", "V"},
    {nullptr, "EqMenu", "Equalizer Overlay", "E"},
    {nullptr, "ScopeToggle", "Switch Osci / Sphere", "o"},
    {"SYSTEM", "Settings", "Open Settings", "s"},
    {nullptr, "Cheatsheet", "Cheatsheet", "?"},
    {nullptr, "Quit", "Quit Application", "q"},
};
const int kKeyActionCount = static_cast<int>(sizeof(kKeyActions) / sizeof(kKeyActions[0]));

std::string key_binding(const RadioSettings& s, int a) {
    if (a < 0 || a >= kKeyActionCount) return "";
    const auto it = s.keys.find(kKeyActions[a].id);
    return it != s.keys.end() && !it->second.empty() ? it->second : kKeyActions[a].def;
}

int key_code(const std::string& k) {
    if (k == "SPACE") return ' ';
    if (k == "TAB") return 9;
    if (k == "BACKSPACE") return 127;
    if (k.size() == 1 && static_cast<unsigned char>(k[0]) > 32 && static_cast<unsigned char>(k[0]) < 127) return static_cast<unsigned char>(k[0]);
    return 0;
}

std::string key_conflict(const RadioSettings& s, const std::string& key, int except) {
    const int code = key_code(key);
    if (code == 0) return "";
    for (int i = 0; i < kKeyActionCount; ++i)
        if (i != except && key_code(key_binding(s, i)) == code) {
            std::string up = kKeyActions[i].label;
            for (auto& c : up) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
            return up;
        }
    for (const char* p = "1234567890ertdfg"; *p; ++p) if (code == *p) return "PRESET SLOT KEY";
    if (code == 'j' || code == 'k') return "CURSOR DOWN / UP";
    if (code == '=' || code == '_') return "VOLUME (= and _ are the unshifted + and -)";
    return "";
}

int key_translate(const RadioSettings& s, int typed) {
    if (typed <= 32 && typed != 9) return typed;
    if (typed >= 127) return typed;
    // the old aliases of + and - follow the volume actions while those keep their defaults
    if (typed == '=' && key_binding(s, 7) == "+") return '+';
    if (typed == '_' && key_binding(s, 8) == "-") return '-';
    for (int i = 0; i < kKeyActionCount; ++i)
        if (key_code(key_binding(s, i)) == typed) return static_cast<unsigned char>(kKeyActions[i].def[0]);
    for (int i = 0; i < kKeyActionCount; ++i)
        if (typed == static_cast<unsigned char>(kKeyActions[i].def[0])) return 0;   // a default key whose action moved elsewhere
    return typed;
}

int key_of(const RadioSettings& s, const char* id) {
    for (int i = 0; i < kKeyActionCount; ++i)
        if (std::string(kKeyActions[i].id) == id) return key_code(key_binding(s, i));
    return 0;
}

const char* const kAboutLines[] = {
    "Mousiki",
    "",
    "A terminal music player built for people who prefer control.",
    "Zero external UI bloat. Mousiki is designed around a fast, ",
    "focused TUI with no unnecessary interface layers. A terminal ",
    "music player built for people who prefer control.",
    "",
    "Devloper : ender                         Github   : itzender5820",
    "Email    : itz.ender5820@gmail.com",
    "Version  : original and final v1.0       Licence  : Apache licence 2.0",
    "",
    "Windows port : Steffen Schwerdtfeger     Github   : StSchwerdtfeger",
    "Version      : v2.5.0                    Licence  : Apache licence 2.0",
    "",
    "Adjusted to also run on Windows. Several features and modifications were",
    "added, the general design remained. Additions: playlist menu, meta data ",
    "editor incl. fetch meta data function via AcoustID, listening history, ",
    "big list/queue overlays, equalizer, YX mode oscillator several new hot ",
    "keys and then some... See Readme.md for full list of additions and changes ",
    "along the win native port. Done with the help of AI tools.",
};
const int kAboutLineCount = static_cast<int>(sizeof(kAboutLines) / sizeof(kAboutLines[0]));

StoragePaths resolve_storage_paths(const RadioSettings& s) {
    StoragePaths o;
    const fs::path def = radio_default_dir();
    // Empty = the layout the radio always had: the files sit directly in ~/.config/mousiki.
    if (s.stations_path.empty()) { o.stations_file = def / "stations.txt"; o.lists_file = def / "stationlists.txt"; }
    else {
        o.stations_file = fs::path(s.stations_path) / "stations" / "stations.txt";
        o.lists_file = fs::path(s.stations_path) / "station_lists" / "stationlists.txt";
    }
    o.presets_file = s.presets_path.empty() ? def / "presets.txt" : fs::path(s.presets_path) / "presets" / "presets.txt";
    o.history_dir = (s.history_path.empty() ? def : fs::path(s.history_path)) / "radio_history";
    if (s.download_same_as_player) {
        const std::string p = player_download_folder();
        o.player_download = true;
        if (!p.empty()) o.download_dir = p;
        else {
            const char* home = std::getenv("HOME");
            if (!home || !*home) home = std::getenv("USERPROFILE");
            o.download_dir = fs::path(home ? home : ".") / ".cache" / "mousiki";
            o.download_note = "player has no DownloadFolder: its cache";
        }
    } else {
        fs::path base = s.download_path;
        if (base.empty()) {
            const char* home = std::getenv("HOME");
            if (!home || !*home) home = std::getenv("USERPROFILE");
            base = fs::path(home ? home : ".") / "Music";
        }
        o.download_dir = base / "radio_downloads";
    }
    return o;
}

fs::path radio_config_path() {
    // Same search order as stations.txt: the user's folder first, then the working directory.
    const fs::path home = home_config_path();
    std::error_code ec;
    if (fs::exists(home, ec)) return home;
    if (fs::exists("radio_config.txt", ec)) return fs::path("radio_config.txt");
    return home;   // nothing yet: this is where a save creates it
}

RadioSettings load_radio_settings(std::string* source_out) {
    RadioSettings s;
    const fs::path p = radio_config_path();
    std::ifstream in(p);
    if (!in) { if (source_out) *source_out = "built-in defaults"; return s; }
    if (source_out) *source_out = p.string();
    std::map<std::string, std::string> kv;
    std::vector<std::string> eq_preset_lines;
    std::string line;
    while (std::getline(in, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;
        const size_t eq = line.find('=');
        if (eq == std::string::npos) continue;
        kv[lower(trim(line.substr(0, eq)))] = trim(line.substr(eq + 1));
        if (lower(trim(line.substr(0, eq))) == "equalizerpreset") eq_preset_lines.push_back(trim(line.substr(eq + 1)));   // repeated key
    }
    auto col = [&](const char* key, std::string& field) {
        const auto it = kv.find(lower(key));
        if (it != kv.end() && valid_color_value(it->second)) field = it->second;   // a bad value keeps the default
    };
    col("ColorBorderTop", s.border_top);          col("ColorBorderBottom", s.border_bottom);
    col("ColorOnAirUpperLeft", s.on_air_upper_left); col("ColorOnAirBottomRight", s.on_air_bottom_right);
    col("ColorMetadataKey", s.meta_key);          col("ColorMetadataVal", s.meta_val);
    col("ColorVizLeft", s.viz_left);              col("ColorVizCenter", s.viz_center);   col("ColorVizRight", s.viz_right);
    col("ColorOsciLeft", s.osci_left);            col("ColorOsciRight", s.osci_right);
    col("ColorFrequencyLine", s.freq_line);       col("ColorFrequencyMHz", s.freq_mhz);
    col("ColorPresetInactiveFg", s.preset_inactive_fg); col("ColorPresetInactiveBg", s.preset_inactive_bg);
    col("ColorPresetKeyFg", s.preset_key_fg);
    col("ColorVolumeCurrent", s.volume_current);  col("ColorVolumePossible", s.volume_possible);
    col("ColorListInactiveFg", s.list_fg);        col("ColorListInactiveBg", s.list_bg);
    col("ColorListPlayingFg", s.list_playing_fg); col("ColorListPlayingBg", s.list_playing_bg);
    col("ColorListCursorFg", s.list_cursor_fg);   col("ColorListCursorBg", s.list_cursor_bg);
    col("ColorHeader", s.header);                 col("ColorLegend", s.legend);
    col("ColorTabCurrent", s.tab_current);        col("ColorTabOther", s.tab_other);

    auto get = [&](const char* key) -> const std::string* {
        const auto it = kv.find(lower(key));
        return it == kv.end() ? nullptr : &it->second;
    };
    if (auto v = get("StationListsPath")) s.stations_path = *v;
    if (auto v = get("PresetsPath")) s.presets_path = *v;
    if (auto v = get("DownloadPath")) s.download_path = *v;
    if (auto v = get("DownloadSameAsMusicPlayer")) s.download_same_as_player = as_bool(*v, s.download_same_as_player);
    if (auto v = get("HistoryPath")) s.history_path = *v;
    if (auto v = get("SleepFade")) s.sleep_fade = as_bool(*v, s.sleep_fade);
    for (int i = 0; i < kKeyActionCount; ++i) {
        if (auto v = get((std::string("HKey") + kKeyActions[i].id).c_str())) {
            std::string val = *v;
            if (key_code(val) != 0 && val != kKeyActions[i].def) s.keys[kKeyActions[i].id] = val;
        }
    }
    if (auto v = get("OnAirAscii")) s.on_air_ascii = as_bool(*v, s.on_air_ascii);
    if (auto v = get("PulseWave")) s.pulse_wave = as_bool(*v, s.pulse_wave);
    if (auto v = get("DummyButtons")) s.dummy_buttons = as_bool(*v, s.dummy_buttons);
    if (auto v = get("OsciSphere")) { const std::string l = lower(*v); s.scope_mode = l == "sphere" ? 1 : l == "off" ? 2 : 0; }
    if (auto v = get("TuneNoise")) s.tune_noise = as_bool(*v, s.tune_noise);
    if (auto v = get("StereoSound")) s.stereo = as_bool(*v, s.stereo);
    if (auto v = get("NormalizeVolume")) s.normalize = as_bool(*v, s.normalize);
    if (auto v = get("NormalizeTargetLufs")) s.normalize_target_lufs = as_int(*v, static_cast<int>(s.normalize_target_lufs), -40, 0);
    if (auto v = get("NormalizeMaxBoostDb")) s.normalize_max_boost_db = as_int(*v, static_cast<int>(s.normalize_max_boost_db), 0, 24);
    if (auto v = get("EqualizerEnabled")) s.eq_enabled = as_bool(*v, s.eq_enabled);
    auto parse_gains = [](const std::string& str, EqGains& out) {   // ten comma separated dB values; anything malformed is ignored as a whole
        EqGains g{};
        size_t pos = 0; int n = 0;
        while (pos <= str.size() && n < kEqBands) {
            const size_t comma = str.find(',', pos);
            const std::string tok = str.substr(pos, comma == std::string::npos ? std::string::npos : comma - pos);
            try { g[static_cast<size_t>(n++)] = std::clamp(std::stof(tok), kEqMinDb, kEqMaxDb); } catch (...) { return false; }
            if (comma == std::string::npos) break;
            pos = comma + 1;
        }
        if (n != kEqBands) return false;
        out = g;
        return true;
    };
    if (auto v = get("EqualizerBands")) parse_gains(*v, s.eq_gains);
    for (const std::string& val : eq_preset_lines) {   // "<ten gains>|<name>"
        const size_t bar = val.find('|');
        if (bar == std::string::npos) continue;
        const std::string name = trim(val.substr(bar + 1));
        if (name.empty() || eq_name_reserved(name) || s.eq_custom_presets.size() >= kEqMaxCustomPresets) continue;
        bool dup = false;
        for (const auto& cp : s.eq_custom_presets) if (eq_name_equal(cp.name, name)) { dup = true; break; }
        EqCustomPreset cp;
        if (dup || !parse_gains(val.substr(0, bar), cp.gains)) continue;
        cp.name = name.substr(0, kEqNameMaxBytes);
        s.eq_custom_presets.push_back(std::move(cp));
    }
    if (auto v = get("GradientWaveSpeed")) try { s.gradient_wave_speed = std::clamp(std::stod(*v), 0.01, 1.0); } catch (...) {}
    if (auto v = get("PulseWaveSpeed")) try { s.pulse_wave_speed = std::clamp(std::stod(*v), 0.01, 1.0); } catch (...) {}
    if (auto v = get("PlaybackMode")) s.playback_shuffle = lower(*v) != "list";
    if (auto v = get("Visualizer")) s.element_visualizer = as_bool(*v, s.element_visualizer);
    if (auto v = get("VisualizerFluidity")) s.visualizer_fluidity = as_int(*v, s.visualizer_fluidity, 1, 10);
    if (auto v = get("VisualizerDegradationSpeed")) s.visualizer_degradation_speed = as_int(*v, s.visualizer_degradation_speed, 1, 10);
    if (auto v = get("VisualizerViscosity")) s.visualizer_viscosity = as_int(*v, s.visualizer_viscosity, 0, 10);
    // The scope's parameters, once per style: braille = Osci<name>, image = OsciImage<name>.
    for (int st = 0; st < 2; ++st) {
        const std::string P = st == 0 ? "Osci" : "OsciImage";
        OsciSet& o = s.osci_set[st];
        auto key = [&](const char* n) { return P + n; };
        if (auto v = get(key("Decay").c_str())) o.decay = as_float(*v, o.decay, 0.0f, 0.99f);
        if (auto v = get(key("DotThreshold").c_str())) o.dot_threshold = as_float(*v, o.dot_threshold, 0.01f, 1.0f);
        if (auto v = get(key("TailBrightness").c_str())) o.tail = as_float(*v, o.tail, 0.0f, 1.0f);
        if (auto v = get(key("Interpolation").c_str())) o.interp = as_bool(*v, o.interp);
        if (auto v = get(key("ZAxis").c_str())) o.z = as_bool(*v, o.z);
        if (auto v = get(key("ZDepth").c_str())) o.z_depth = as_float(*v, o.z_depth, 0.0f, 1.0f);
        if (auto v = get(key("ZSource").c_str())) o.z_source = lower(*v) == "level" ? 1 : 0;
        if (auto v = get(key("TraceLength").c_str())) o.trace = as_int(*v, o.trace, 128, 1024);
        if (auto v = get(key("Rotate").c_str())) o.rotate = as_bool(*v, o.rotate);
        if (auto v = get(key("MonoPhase").c_str())) o.mono_phase = as_bool(*v, o.mono_phase);
        if (auto v = get(key("Glow").c_str())) o.glow = as_float(*v, o.glow, 0.0f, 1.0f);
        if (auto v = get(key("Palette").c_str())) {
            const std::string l = lower(*v);
            for (int i = 0; i < kOsciPaletteCount; ++i) if (l == kOsciPaletteNames[i]) o.palette = i;
        }
    }
    if (auto v = get("TemperatureColor")) if (as_bool(*v, false)) s.osci_set[0].palette = s.osci_set[1].palette = 2;   // the old on/off option
    if (auto v = get("OsciBraille")) s.osci_style = as_bool(*v, true) ? 0 : 1;   // the old name: false meant "not braille"
    if (auto v = get("OsciStyle")) s.osci_style = lower(*v) == "image" ? 1 : 0;
    if (auto v = get("OsciImageProtocol")) s.gfx_protocol = lower(*v);
    if (auto v = get("FrameRate")) { const int fr = as_int(*v, s.frame_rate, 30, 90); s.frame_rate = fr >= 75 ? 90 : fr >= 52 ? 60 : fr >= 38 ? 45 : 30; }
    auto glyph = [&](const char* key, std::string& field) { if (auto v = get(key)) if (!v->empty()) field = *v; };
    glyph("BoxUpperLeft", s.box_upper_left);   glyph("BoxUpperRight", s.box_upper_right);
    glyph("BoxLowerLeft", s.box_lower_left);   glyph("BoxLowerRight", s.box_lower_right);
    glyph("BoxVertical", s.box_vertical);      glyph("BoxHorizontal", s.box_horizontal);
    glyph("ListSeparator", s.list_separator);
    return s;
}

namespace {
std::string eq_bands_text(const EqGains& g) {
    std::ostringstream o;
    for (int b = 0; b < kEqBands; ++b) o << (b ? "," : "") << g[static_cast<size_t>(b)];
    return o.str();
}
std::string eq_presets_text(const RadioSettings& s) {
    std::ostringstream o;
    for (const auto& cp : s.eq_custom_presets) o << "EqualizerPreset=" << eq_bands_text(cp.gains) << "|" << cp.name << "\n";
    o << "# EqualizerPreset=<ten gains>|<name> is one custom preset (made in the overlay with S), up to " << kEqMaxCustomPresets << "\n";
    return o.str();
}
} // namespace

bool save_radio_settings(const RadioSettings& s, std::string* err) {
    const fs::path p = home_config_path();
    // Written where it is read from: a ./radio_config.txt that is in use stays the one that is saved.
    const fs::path target = fs::exists(fs::path("radio_config.txt")) && !fs::exists(p) ? fs::path("radio_config.txt") : p;
    std::error_code ec;
    if (target.has_parent_path()) fs::create_directories(target.parent_path(), ec);
    std::ofstream o(target, std::ios::trunc);
    if (!o) { if (err) *err = "cannot write " + target.string(); return false; }
    auto num = [](float v) { std::ostringstream os; os.precision(2); os << std::fixed << v; return os.str(); };
    auto osci_block = [&](const std::string& P, const OsciSet& o) {
        auto b = [](bool v) { return std::string(v ? "true" : "false"); };
        return P + "Decay=" + num(o.decay) + "\n" + P + "DotThreshold=" + num(o.dot_threshold) + "\n" + P + "TailBrightness=" + num(o.tail) + "\n"
             + P + "Interpolation=" + b(o.interp) + "\n" + P + "ZAxis=" + b(o.z) + "\n" + P + "ZDepth=" + num(o.z_depth) + "\n"
             + P + "ZSource=" + (o.z_source == 1 ? "level" : "speed") + "\n" + P + "TraceLength=" + std::to_string(o.trace) + "\n"
             + P + "Rotate=" + b(o.rotate) + "\n" + P + "MonoPhase=" + b(o.mono_phase) + "\n"
             + P + "Palette=" + kOsciPaletteNames[std::clamp(o.palette, 0, kOsciPaletteCount - 1)] + "\n" + P + "Glow=" + num(o.glow) + "\n";
    };
    o << "# Mousiki radio mode -- settings (separate from the music player's config.txt)\n"
         "# Location (Windows): %USERPROFILE%\\.config\\mousiki\\radio_config.txt\n"
         "# Location (Linux/macOS): $HOME/.config/mousiki/radio_config.txt  (or ./radio_config.txt next to the program)\n"
         "# Written by the settings screen (press s in the radio); edit by hand if you like.\n"
         "#\n"
         "# Colours are numbers of the 256-colour terminal palette (1-255). 0 or empty = no colour (the terminal's own).\n"
         "# 30-47 and 90-107 are sent as direct terminal colour codes.\n\n"
         "##-------------------------------------------\n"
         "##              COLORS\n"
         "##-------------------------------------------\n\n"
         "# Frame lines: fade from top to bottom\n"
         "ColorBorderTop=" << s.border_top << "\nColorBorderBottom=" << s.border_bottom << "\n\n"
         "# ON AIR sign: diagonal gradient (upper left -> bottom right); also the needle, the lamp and the LIVE text\n"
         "ColorOnAirUpperLeft=" << s.on_air_upper_left << "\nColorOnAirBottomRight=" << s.on_air_bottom_right << "\n\n"
         "# Station info: the labels (Station, Title ...) and their values\n"
         "ColorMetadataKey=" << s.meta_key << "\nColorMetadataVal=" << s.meta_val << "\n\n"
         "# FFT spectrum under the station info (ColorVizCenter = optional middle stop of the gradient)\n"
         "ColorVizLeft=" << s.viz_left << "\nColorVizCenter=" << s.viz_center << "\nColorVizRight=" << s.viz_right << "\n\n"
         "# Oscilloscope (top right)\n"
         "ColorOsciLeft=" << s.osci_left << "\nColorOsciRight=" << s.osci_right << "\n\n"
         "# Frequency band: the line + ticks / the numbers and the small dots\n"
         "ColorFrequencyLine=" << s.freq_line << "\nColorFrequencyMHz=" << s.freq_mhz << "\n\n"
         "# Volume bar: the # / the -\n"
         "ColorVolumeCurrent=" << s.volume_current << "\nColorVolumePossible=" << s.volume_possible << "\n\n"
         "# STATIONS list rows (the station names in the PRESETS pane use the same colours)\n"
         "ColorListInactiveFg=" << s.list_fg << "\nColorListInactiveBg=" << s.list_bg << "\n"
         "ColorListPlayingFg=" << s.list_playing_fg << "\nColorListPlayingBg=" << s.list_playing_bg << "\n"
         "ColorListCursorFg=" << s.list_cursor_fg << "\nColorListCursorBg=" << s.list_cursor_bg << "\n\n"
         "# PRESETS pane: key slots without a station / the short-cut key letters\n"
         "ColorPresetInactiveFg=" << s.preset_inactive_fg << "\nColorPresetInactiveBg=" << s.preset_inactive_bg << "\n"
         "ColorPresetKeyFg=" << s.preset_key_fg << "\n\n"
         "# Secondary text: captions (off air, no signal), the ON AIR sign while nothing is live, empty-pane\n"
         "# messages, status lines, dimmed entries\n"
         "ColorHeader=" << s.header << "\n\n"
         "# Key command legends: the hint lines of the menus ([TAB] Switch pane | ...), their errors, the cheatsheet titles\n"
         "ColorLegend=" << s.legend << "\n\n"
         "# Settings screen: the current tab name (and the < \u2194 > hint) / the other tab names\n"
         "ColorTabCurrent=" << s.tab_current << "\nColorTabOther=" << s.tab_other << "\n\n"
         "##-------------------------------------------\n"
         "##              ON/OFF\n"
         "##-------------------------------------------\n\n"
         "OnAirAscii=" << (s.on_air_ascii ? "true" : "false") << "\n"
         "PulseWave=" << (s.pulse_wave ? "true" : "false") << "\n"
         "DummyButtons=" << (s.dummy_buttons ? "true" : "false") << "\n"
         "# right of the station info: osci | sphere | off\n"
         "OsciSphere=" << on_off_value(s, 3) << "\n"
         "Visualizer=" << (s.element_visualizer ? "true" : "false") << "\n"
         "StereoSound=" << (s.stereo ? "true" : "false") << "\n"
         "# static that fades in when another station is tuned and fades out when the stream plays\n"
         "TuneNoise=" << (s.tune_noise ? "true" : "false") << "\n"
         "# loudness normalisation (SHIFT+v overlay, v toggles): target -40..0 LUFS, max boost 0..24 dB\n"
         "NormalizeVolume=" << (s.normalize ? "true" : "false") << "\n"
         "NormalizeTargetLufs=" << static_cast<int>(s.normalize_target_lufs) << "\n"
         "NormalizeMaxBoostDb=" << static_cast<int>(s.normalize_max_boost_db) << "\n"
         "# equaliser (SHIFT+e overlay): ten gains in dB (-12..12) for 31, 62, 125, 250, 500 Hz, 1, 2, 4, 8, 16 kHz\n"
         "EqualizerEnabled=" << (s.eq_enabled ? "true" : "false") << "\n"
         "EqualizerBands=" << eq_bands_text(s.eq_gains) << "\n"
         << eq_presets_text(s) <<
         "\n"
         "##-------------------------------------------\n"
         "##              ANIMATION\n"
         "##-------------------------------------------\n\n"
         "# speeds: 0.01 - 1.00 (colour wave of the ON AIR sign in cycles per second | pulse wave behind it in loops per second)\n"
         "GradientWaveSpeed=" << fmt_speed(s.gradient_wave_speed) << "\n"
         "PulseWaveSpeed=" << fmt_speed(s.pulse_wave_speed) << "\n"
         "# station surf mode (key m): list | shuffle\n"
         "PlaybackMode=" << (s.playback_shuffle ? "shuffle" : "list") << "\n\n"
         "##-------------------------------------------\n"
         "##              PATHS (empty = default)\n"
         "##-------------------------------------------\n\n"
         "# StationListsPath: <path>/stations/stations.txt and <path>/station_lists/stationlists.txt (empty: the files sit in ~/.config/mousiki)\n"
         "StationListsPath=" << s.stations_path << "\n"
         "# PresetsPath: <path>/presets/presets.txt\n"
         "PresetsPath=" << s.presets_path << "\n"
         "# Recordings (key y) and YouTube downloads: <DownloadPath>/radio_downloads, or the music player's download folder\n"
         "DownloadPath=" << s.download_path << "\n"
         "DownloadSameAsMusicPlayer=" << (s.download_same_as_player ? "true" : "false") << "\n"
         "# HistoryPath: <path>/radio_history/history_radio.txt (empty: ~/.config/mousiki)\n"
         "HistoryPath=" << s.history_path << "\n\n"
         "##-------------------------------------------\n"
         "##              SLEEP TIMER\n"
         "##-------------------------------------------\n\n"
         "# the sleep timer (SHIFT+z) fades the volume out before it stops the stream\n"
         "SleepFade=" << (s.sleep_fade ? "true" : "false") << "\n\n"
         "##-------------------------------------------\n"
         "##              KEYS (REFERENCE tab): one character, or SPACE / TAB / BACKSPACE\n"
         "##-------------------------------------------\n\n";
    for (int i = 0; i < kKeyActionCount; ++i) o << "HKey" << kKeyActions[i].id << "=" << key_binding(s, i) << "\n";
    o << "\n"
         "##-------------------------------------------\n"
         "##              LOOK / TUNING (edit here)\n"
         "##-------------------------------------------\n\n"
         "# FFT: 1-10 (higher = smoother / slower rise) | 1-10 (higher = bars fall faster) | 0-10 (neighbour blending)\n"
         "VisualizerFluidity=" << s.visualizer_fluidity << "\n"
         "VisualizerDegradationSpeed=" << s.visualizer_degradation_speed << "\n"
         "VisualizerViscosity=" << s.visualizer_viscosity << "\n"
         "# Oscilloscope (SHIFT+o overlay), one block per style: braille = Osci<name>, image = OsciImage<name>. Each style keeps its own values.\n"
         "# Decay = afterglow 0.00-0.99 | DotThreshold 0.01-1.00 (braille: lower = thicker line) | TailBrightness 0.00-1.00 | Interpolation = connect the\n"
         "# samples with lines | ZAxis = beam intensity (ZDepth 0-1, ZSource speed|level) | TraceLength 128-1024 samples | Rotate = 45 degrees (mid vertical) |\n"
         "# MonoPhase = phase portrait for near-mono signals | Palette gradient|settings|temperature|aurora|magma|ice|neon|spectrum | Glow 0-1 (image bloom)\n"
         << osci_block("Osci", s.osci_set[0]) << osci_block("OsciImage", s.osci_set[1]) <<
         "# ON/OFF tab: braille | image (a real pixel picture drawn by the terminal: Kitty graphics or Sixel; braille where there is neither)\n"
         "OsciStyle=" << (s.osci_style == 1 ? "image" : "braille") << "\n"
         "# image style: auto | kitty | sixel | off\n"
         "OsciImageProtocol=" << s.gfx_protocol << "\n"
         "# screen refresh: 30 | 45 | 60 | 90 frames per second (higher = more CPU and, for the image, more data for the terminal)\n"
         "FrameRate=" << s.frame_rate << "\n\n"
         "BoxUpperLeft=" << s.box_upper_left << "\nBoxUpperRight=" << s.box_upper_right << "\n"
         "BoxLowerLeft=" << s.box_lower_left << "\nBoxLowerRight=" << s.box_lower_right << "\n"
         "BoxVertical=" << s.box_vertical << "\nBoxHorizontal=" << s.box_horizontal << "\n"
         "ListSeparator=" << s.list_separator << "\n";
    if (!o) { if (err) *err = "cannot write " + target.string(); return false; }
    return true;
}

} // namespace muisc::radio
