#pragma once
// Single-line text editing for the radio's text fields (main SEARCH box, the PRESETS menu's search box,
// the preset name overlay): caret, Shift+arrow marking, Home/End, Delete, Ctrl+C/X/V.
//
// This is the music player's own editor (edit_text_key() / paint_edit_field() in app.cpp) -- same keys,
// same behaviour, same look (selection in reverse video, caret as a block) -- copied here because those
// two live in app.cpp's anonymous namespace. When radio mode moves into the main app, delete this file
// and call the player's versions instead.
//
// A field is a std::string plus an EditState: two BYTE offsets, the caret and the other end of the
// selection (anchor). caret == anchor means nothing is marked. Every move / delete is UTF-8 aware.
#include <algorithm>
#include <string>
#include <vector>
#include "terminal_ui.h"

namespace muisc::radio {

struct EditState {
    size_t caret = 0;
    size_t anchor = 0;
    void to_end(const std::string& s) { caret = anchor = s.size(); }
    void reset() { caret = anchor = 0; }
};

namespace textedit_detail {

// Every byte offset at which `s` may be split without breaking UTF-8: 0, the start of each
// codepoint, then s.size().
inline std::vector<size_t> utf8_cuts(const std::string& s) {
    std::vector<size_t> cuts;
    cuts.reserve(s.size() + 1);
    cuts.push_back(0);
    size_t i = 0;
    while (i < s.size()) {
        const unsigned char c = static_cast<unsigned char>(s[i]);
        size_t n = 1;
        if ((c & 0xE0) == 0xC0) n = 2;
        else if ((c & 0xF0) == 0xE0) n = 3;
        else if ((c & 0xF8) == 0xF0) n = 4;
        i += n;
        if (i > s.size()) i = s.size();
        cuts.push_back(i);
    }
    return cuts;
}

inline size_t prev_cut(const std::string& s, size_t off) {
    if (off == 0) return 0;
    size_t best = 0;
    for (size_t c : utf8_cuts(s)) { if (c >= off) break; best = c; }
    return best;
}

inline size_t next_cut(const std::string& s, size_t off) {
    if (off >= s.size()) return s.size();
    for (size_t c : utf8_cuts(s)) if (c > off) return c;
    return s.size();
}

inline void clamp(const std::string& s, EditState& e) {
    e.caret = std::min(e.caret, s.size());
    e.anchor = std::min(e.anchor, s.size());
}

inline void range(const EditState& e, size_t& a, size_t& b) {
    a = std::min(e.caret, e.anchor);
    b = std::max(e.caret, e.anchor);
}

inline void erase_selection(std::string& s, EditState& e) {
    size_t a, b;
    range(e, a, b);
    if (a == b) return;
    s.erase(a, b - a);
    e.caret = e.anchor = a;
}

inline void move(const std::string& s, EditState& e, int dir, bool shift) {
    clamp(s, e);
    e.caret = dir < 0 ? prev_cut(s, e.caret) : next_cut(s, e.caret);
    if (!shift) e.anchor = e.caret;
}

inline void backspace(std::string& s, EditState& e) {
    clamp(s, e);
    if (e.caret != e.anchor) { erase_selection(s, e); return; }
    const size_t p = prev_cut(s, e.caret);
    s.erase(p, e.caret - p);
    e.caret = e.anchor = p;
}

inline void delete_forward(std::string& s, EditState& e) {
    clamp(s, e);
    if (e.caret != e.anchor) { erase_selection(s, e); return; }
    const size_t n = next_cut(s, e.caret);
    s.erase(e.caret, n - e.caret);
    e.anchor = e.caret;
}

inline void insert(std::string& s, EditState& e, char c, size_t limit) {
    clamp(s, e);
    erase_selection(s, e);
    if (s.size() >= limit) return;
    s.insert(e.caret, 1, c);
    e.caret = e.anchor = e.caret + 1;
}

// Control characters are dropped (a newline would corrupt the single-line row); what still does not
// fit is trimmed, codepoint-aligned, against `limit`.
inline void paste(std::string& s, EditState& e, const std::string& text, size_t limit) {
    clamp(s, e);
    erase_selection(s, e);
    std::string t;
    for (unsigned char c : text) if (c >= 32 && c != 127) t.push_back(static_cast<char>(c));
    if (t.empty()) return;
    const size_t room = s.size() >= limit ? 0 : limit - s.size();
    while (t.size() > room && !t.empty()) t.erase(prev_cut(t, t.size()), std::string::npos);
    if (t.empty()) return;
    s.insert(e.caret, t);
    e.caret = e.anchor = e.caret + t.size();
}

inline bool is_text_key(int key) {
    return (key >= 32 && key < 127) || (key >= 0x80 && key <= 0xFF);
}

} // namespace textedit_detail

// One keystroke into one field. Returns true only when the TEXT changed (so the caller re-runs its
// filter); caret moves, marking, copy and a cut with nothing marked return false.
// `status` (nullable) receives "COPIED" / "CUT" / "PASTED" / "nothing selected".
// The caller must NOT hand it Up/Down arrows it wants for something else -- they are ignored here.
inline bool edit_text_key(std::string& buf, EditState& e, int key, size_t limit, std::string* status = nullptr) {
    using namespace textedit_detail;
    auto say = [&](const char* t) { if (status) *status = t; };
    if (key == kKeyHome) { e.caret = e.anchor = 0; return false; }
    if (key == kKeyEnd) { e.caret = e.anchor = buf.size(); return false; }
    if (key == kKeyShiftLeft) { move(buf, e, -1, true); return false; }
    if (key == kKeyShiftRight) { move(buf, e, +1, true); return false; }
    if (key == kKeyDelete) { delete_forward(buf, e); return true; }
    if (key == kKeyCtrlC) {
        size_t a, b;
        range(e, a, b);
        const std::string t = a == b ? buf : buf.substr(a, b - a);   // nothing marked: copy the whole field
        clipboard_set(t);
        say(t.empty() ? "nothing selected" : "COPIED");
        return false;
    }
    if (key == kKeyCtrlX) {
        size_t a, b;
        range(e, a, b);
        if (a == b) return false;   // nothing marked: cut must never empty the field
        clipboard_set(buf.substr(a, b - a));
        erase_selection(buf, e);
        say("CUT");
        return true;
    }
    if (key == kKeyCtrlV) { paste(buf, e, clipboard_get(), limit); say("PASTED"); return true; }
    if (key == 127 || key == 8) { backspace(buf, e); return true; }
    // Arrows collapse to the letters A-D, so a typed capital A-D is told apart by last_key_was_arrow().
    if (last_key_was_arrow() && (key == 'A' || key == 'B')) return false;
    if (last_key_was_arrow() && (key == 'C' || key == 'D')) { move(buf, e, key == 'D' ? -1 : +1, false); return false; }
    if (is_text_key(key)) { insert(buf, e, static_cast<char>(key), limit); return true; }
    return false;
}

// What a field looks like on screen.
struct EditPaint {
    std::string s;   // windowed text, marked range in reverse video, caret as a block
    int cols = 0;    // display columns `s` really occupies (ANSI not counted)
};

// Paints a field into at most `w` columns: a window scrolled so the caret stays visible, the marked
// range in reverse video and, with `block`, the caret drawn as a block. `restore` is the SGR to re-emit
// after the reverse-video run so the field keeps its own colour.
inline EditPaint paint_edit_field(const std::string& s, EditState e, int w, const std::string& restore, bool block = true) {
    using namespace textedit_detail;
    EditPaint out;
    if (w < 1) w = 1;
    clamp(s, e);
    size_t caret = e.caret, anchor = e.anchor;
    if (anchor > caret) std::swap(anchor, caret);

    const std::vector<size_t> cuts = utf8_cuts(s);
    std::vector<int> col(cuts.size(), 0);   // col[i] = display column of cuts[i]
    for (size_t i = 1; i < cuts.size(); ++i)
        col[i] = col[i - 1] + display_width(s.substr(cuts[i - 1], cuts[i] - cuts[i - 1]));
    auto idx_of = [&](size_t off) -> size_t {
        for (size_t i = 0; i < cuts.size(); ++i) if (cuts[i] == off) return i;
        return cuts.size() - 1;
    };
    const size_t caret_i = idx_of(caret);
    const size_t anchor_i = idx_of(anchor);

    // leftmost window that still keeps the caret inside it (one column is reserved for the block)
    const int target = col[caret_i] - (w - 1);
    size_t lo_i = 0;
    for (size_t i = 0; i <= caret_i; ++i) {
        if (col[i] >= target) { lo_i = i; break; }
        lo_i = caret_i;
    }
    size_t hi_i = lo_i;
    while (hi_i + 1 < cuts.size() && col[hi_i + 1] - col[lo_i] <= w) ++hi_i;
    if (hi_i < caret_i) hi_i = caret_i;

    auto emit = [&](size_t from, size_t to, bool sel, const char* glyph) {
        const std::string body = glyph ? std::string(glyph) : s.substr(from, to - from);
        if (body.empty()) return;
        if (sel) out.s += "\x1b[7m" + body + "\x1b[0m" + restore;
        else out.s += body;
    };
    for (size_t i = lo_i; i < hi_i; ++i) {
        const bool sel = (i >= anchor_i && i < caret_i);
        if (block && i == caret_i) emit(cuts[i], cuts[i + 1], sel, "\u2588");
        else emit(cuts[i], cuts[i + 1], sel, nullptr);
    }
    if (block && caret_i >= hi_i) emit(0, 0, false, "\u2588");

    out.cols = col[hi_i] - col[lo_i];
    if (block && caret_i >= hi_i) out.cols += 1;
    return out;
}

} // namespace muisc::radio
