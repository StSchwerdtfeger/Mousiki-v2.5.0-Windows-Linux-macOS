#pragma once
// Terminal graphics for the oscilloscope's "image" style: the picture is a real pixel image drawn by the terminal itself
// (the way yazi shows previews). Two protocols are spoken:
//   Kitty graphics  Kitty, WezTerm, Ghostty, Konsole, ...   RGBA with alpha, drawn UNDER the text, replaced in place
//   Sixel           Windows Terminal (1.22+), foot, xterm -ti vt340, mlterm, WezTerm, ...   palette image over the cells
// Without either, the UI keeps drawing the braille scope.
#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace muisc {

enum class GfxProto { None, Kitty, Sixel };

// The finished picture of one frame, filled by the UI (app.cpp) and turned into escape codes by gfx_emit().
struct GfxFrame {
    bool active = false;            // an image belongs on screen this frame (false: remove it)
    bool fresh = false;             // `level` / `hue` hold new pixels (false: the picture of the last frame stays)
    int col = 0, row = 0;           // top-left cell, 0-based
    int cols = 0, rows = 0;         // size in cells
    int crop = 0;                   // cells at the left that an overlay covers and that stay free
    int w = 0, h = 0;               // size of the picture in pixels (with scale > 1 smaller than the screen area)
    int scale = 1;                  // the picture has 1/scale of the screen resolution and is enlarged on the way out:
                                    // Kitty scales it itself (c= / r=), Sixel repeats the pixels (vertically via the pixel
                                    // aspect ratio where the terminal honours it, i.e. Windows Terminal)
    int out_cw = 0, out_ch = 0;     // Sixel: screen pixels of one cell (0 = w / cols, h / rows)
    std::vector<uint8_t> level;     // brightness 0..255, w * h
    std::vector<uint8_t> hue;       // colour index 0..255, w * h (see `pal`)
    std::array<std::array<uint8_t, 3>, 256> pal{};   // hue -> colour at full brightness
};

// Which protocol does this terminal speak? `pref` = "auto" | "kitty" | "sixel" | "off" (the environment variable
// MOUSIKI_GFX overrides it). "auto" asks the terminal (a Kitty graphics query plus the device attributes, 250 ms at
// most) and looks at the environment. Call once at start, with the terminal already in raw mode.
GfxProto gfx_probe(const std::string& pref);
// Pixels per cell: the terminal's report, `override_wxh` ("10x20"), or a guess.
void gfx_cell_pixels(const std::string& override_wxh, int& w, int& h);
// Escape codes that put the picture on screen (cursor moves included; the cursor position is not kept) / take it away.
std::string gfx_emit(GfxProto proto, const GfxFrame& f);
std::string gfx_clear(GfxProto proto);
const char* gfx_name(GfxProto proto);
bool gfx_compressed();
// Placeholder cells for the picture in a text frame (n columns) and their replacement before the frame is written:
// skip = true moves the cursor over them (Sixel: the picture on screen stays), false writes spaces.
std::string gfx_hole(int n);
void gfx_fill_holes(std::string& s, bool skip);
// The most pictures per second sent to the terminal (Kitty 60; Sixel 30 at full resolution, 60 at half / third).
int gfx_picture_cap(GfxProto proto, int scale);   // Kitty pictures are always zlib-compressed (miniz): cheap enough to send every frame

} // namespace muisc
