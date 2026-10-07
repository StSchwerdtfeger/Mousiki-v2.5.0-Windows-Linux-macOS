#pragma once
// The radio's equaliser overlay (SHIFT+E): the music player's 10-band EQ (src/equalizer.h) with the same presets, the
// same keys and the same look. The gains / custom presets live in RadioSettings (radio_config.txt); this file holds the
// overlay's own state and the preset logic (a copy of App::eq_* in app.cpp, which is tied to the player's settings).
#include <algorithm>
#include <string>
#include "radio_settings.h"
#include "radio_textedit.h"

namespace muisc::radio {

struct EqUi {
    int band = 0;                // selected band
    int last_preset = 0;         // unified preset index the cycle continues from (when the curve matches none)
    bool naming = false;         // the "Save as:" prompt
    std::string name;            // its text
    EditState name_edit;
    std::string status;          // feedback of the last action
    bool delete_armed = false;   // DEL / X pressed once on a custom preset
};

// Presets are addressed by one index: 0 .. kEqPresets.size()-1 the built-in ones, then the custom ones.
inline int eq_preset_count(const RadioSettings& c) { return static_cast<int>(kEqPresets.size() + c.eq_custom_presets.size()); }

inline const EqGains& eq_preset_gains(const RadioSettings& c, int index) {
    const int builtin = static_cast<int>(kEqPresets.size());
    if (index < builtin) return kEqPresets[static_cast<size_t>(std::clamp(index, 0, builtin - 1))].gains;
    const int k = std::clamp(index - builtin, 0, static_cast<int>(c.eq_custom_presets.size()) - 1);
    return c.eq_custom_presets[static_cast<size_t>(k)].gains;
}

inline std::string eq_preset_name(const RadioSettings& c, int index) {
    const int builtin = static_cast<int>(kEqPresets.size());
    if (index < builtin) return kEqPresets[static_cast<size_t>(std::clamp(index, 0, builtin - 1))].name;
    const int k = index - builtin;
    return k >= 0 && k < static_cast<int>(c.eq_custom_presets.size()) ? c.eq_custom_presets[static_cast<size_t>(k)].name : std::string("Custom");
}

// The preset the sliders currently equal, or -1 ("Custom").
inline int eq_current_preset(const RadioSettings& c, const EqUi& u) {
    const int n = eq_preset_count(c);
    if (u.last_preset >= 0 && u.last_preset < n && eq_gains_equal(eq_preset_gains(c, u.last_preset), c.eq_gains)) return u.last_preset;
    for (int i = 0; i < n; ++i) if (eq_gains_equal(eq_preset_gains(c, i), c.eq_gains)) return i;
    return -1;
}

inline void eq_open(const RadioSettings& c, EqUi& u) {
    const int m = eq_current_preset(c, u);
    if (m >= 0) u.last_preset = m;
    u.naming = false; u.name.clear(); u.name_edit.reset(); u.status.clear(); u.delete_armed = false;
}

inline void eq_set_gain(RadioSettings& c, int band, float db) {
    band = std::clamp(band, 0, kEqBands - 1);
    c.eq_gains[static_cast<size_t>(band)] = std::clamp(std::round(db), kEqMinDb, kEqMaxDb);
    c.eq_enabled = true;
}

inline void eq_select_preset(RadioSettings& c, EqUi& u, int dir) {
    const int n = eq_preset_count(c);
    const int cur = eq_current_preset(c, u);
    const int base = cur >= 0 ? cur : std::clamp(u.last_preset, 0, n - 1);
    const int next = ((base + dir) % n + n) % n;
    c.eq_gains = eq_preset_gains(c, next);
    c.eq_enabled = true;
    u.last_preset = next;
}

inline void eq_begin_naming(const RadioSettings& c, EqUi& u) {
    u.name.clear();
    const int builtin = static_cast<int>(kEqPresets.size());
    if (eq_current_preset(c, u) < 0 && u.last_preset >= builtin && u.last_preset < eq_preset_count(c)) u.name = eq_preset_name(c, u.last_preset);
    u.naming = true;
    u.name_edit.to_end(u.name);
}

// ENTER in the "Save as:" prompt. Returns true when the custom presets changed (the caller saves the config).
inline bool eq_commit_name(RadioSettings& c, EqUi& u) {
    std::string name;
    for (char ch : u.name) if (static_cast<unsigned char>(ch) >= 32 && ch != '|') name += ch;
    while (!name.empty() && name.front() == ' ') name.erase(name.begin());
    while (!name.empty() && name.back() == ' ') name.pop_back();
    u.naming = false; u.name.clear(); u.name_edit.reset();
    if (name.empty()) { u.status = "Nothing saved: the name is empty"; return false; }
    if (eq_name_reserved(name)) { u.status = "\"" + name + "\" is a built-in name, pick another"; return false; }
    const int builtin = static_cast<int>(kEqPresets.size());
    int existing = -1;
    for (size_t i = 0; i < c.eq_custom_presets.size(); ++i) if (eq_name_equal(c.eq_custom_presets[i].name, name)) { existing = static_cast<int>(i); break; }
    if (existing >= 0) {
        c.eq_custom_presets[static_cast<size_t>(existing)].name = name;
        c.eq_custom_presets[static_cast<size_t>(existing)].gains = c.eq_gains;
        u.last_preset = builtin + existing;
        u.status = "Updated \"" + name + "\"";
    } else if (c.eq_custom_presets.size() >= kEqMaxCustomPresets) {
        u.status = "Limit of " + std::to_string(kEqMaxCustomPresets) + " custom presets reached";
        return false;
    } else {
        EqCustomPreset cp;
        cp.name = name.substr(0, kEqNameMaxBytes);
        cp.gains = c.eq_gains;
        c.eq_custom_presets.push_back(std::move(cp));
        u.last_preset = builtin + static_cast<int>(c.eq_custom_presets.size()) - 1;
        u.status = "Saved \"" + name + "\"";
    }
    return true;
}

// DEL / X: removes the custom preset the sliders equal; the first press only asks. Returns true when one was deleted.
inline bool eq_delete_custom(RadioSettings& c, EqUi& u, bool confirmed) {
    const int builtin = static_cast<int>(kEqPresets.size());
    const int cur = eq_current_preset(c, u);
    if (cur < builtin) { u.status = cur < 0 ? "Select a custom preset to delete it" : "Built-in presets cannot be deleted"; return false; }
    const std::string name = eq_preset_name(c, cur);
    if (!confirmed) { u.delete_armed = true; u.status = "Press DEL / X again to delete \"" + name + "\""; return false; }
    c.eq_custom_presets.erase(c.eq_custom_presets.begin() + (cur - builtin));
    u.last_preset = cur - 1;
    u.status = "Deleted \"" + name + "\"";
    return true;
}

} // namespace muisc::radio
