#include "radio_gfx.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>
#if !defined(_WIN32)
#include <poll.h>
#include <sys/ioctl.h>
#include <unistd.h>
#endif
#include "miniz.h"

namespace {

constexpr int kImageId = 7201;

std::string base64(const uint8_t* d, size_t n) {
    static const char* T = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string o;
    o.reserve((n + 2) / 3 * 4);
    size_t i = 0;
    for (; i + 2 < n; i += 3) {
        const uint32_t v = (static_cast<uint32_t>(d[i]) << 16) | (static_cast<uint32_t>(d[i + 1]) << 8) | d[i + 2];
        o += T[(v >> 18) & 63]; o += T[(v >> 12) & 63]; o += T[(v >> 6) & 63]; o += T[v & 63];
    }
    if (i + 1 == n) {
        const uint32_t v = static_cast<uint32_t>(d[i]) << 16;
        o += T[(v >> 18) & 63]; o += T[(v >> 12) & 63]; o += "==";
    } else if (i + 2 == n) {
        const uint32_t v = (static_cast<uint32_t>(d[i]) << 16) | (static_cast<uint32_t>(d[i + 1]) << 8);
        o += T[(v >> 18) & 63]; o += T[(v >> 12) & 63]; o += T[(v >> 6) & 63]; o += '=';
    }
    return o;
}

std::string at_cell(int row, int col) { return "\x1b[" + std::to_string(row + 1) + ";" + std::to_string(col + 1) + "H"; }

// brightness curve shared by both protocols: faint stays faint, the core turns white-hot
float curve(float l) { return std::pow(std::clamp(l, 0.0f, 1.0f), 0.85f); }
void shade(const GfxFrame& f, uint8_t level, uint8_t hue, float& r, float& g, float& b, float& a) {
    const float l = static_cast<float>(level) / 255.0f;
    a = curve(l);
    const float hot = std::clamp((l - 0.80f) / 0.20f, 0.0f, 1.0f) * 0.40f;   // a white core on the brightest pixels
    r = static_cast<float>(f.pal[hue][0]) * (1.0f - hot) + 255.0f * hot;
    g = static_cast<float>(f.pal[hue][1]) * (1.0f - hot) + 255.0f * hot;
    b = static_cast<float>(f.pal[hue][2]) * (1.0f - hot) + 255.0f * hot;
}

std::string kitty_emit(const GfxFrame& f) {
    const int cw = f.w / std::max(1, f.cols);
    const int cropx = std::clamp(f.crop, 0, f.cols) * cw;
    const int vis_cols = f.cols - std::clamp(f.crop, 0, f.cols);
    if (vis_cols <= 0) return gfx_clear(GfxProto::Kitty);
    std::vector<uint8_t> rgba(static_cast<size_t>(f.w) * f.h * 4);
    for (size_t i = 0; i < static_cast<size_t>(f.w) * f.h; ++i) {
        if (f.level[i] < 8) { rgba[i * 4] = rgba[i * 4 + 1] = rgba[i * 4 + 2] = rgba[i * 4 + 3] = 0; continue; }
        float r, g, b, a;
        shade(f, f.level[i], f.hue[i], r, g, b, a);
        // 5 bits per channel: invisible on screen, and it halves what has to be sent (the bloom is full of near-equal values)
        rgba[i * 4] = static_cast<uint8_t>(r) & 0xF8; rgba[i * 4 + 1] = static_cast<uint8_t>(g) & 0xF8; rgba[i * 4 + 2] = static_cast<uint8_t>(b) & 0xF8;
        rgba[i * 4 + 3] = static_cast<uint8_t>(std::clamp(a * 255.0f, 0.0f, 255.0f)) & 0xF8;
    }
    std::string payload;
    std::string comp = "";
    {   // zlib stream (miniz, vendored in third_party/miniz): level 1 is fast and the picture is mostly empty
        mz_ulong n = mz_compressBound(static_cast<mz_ulong>(rgba.size()));
        std::vector<uint8_t> z(n);
        if (mz_compress2(z.data(), &n, rgba.data(), static_cast<mz_ulong>(rgba.size()), 1) == MZ_OK) { payload = base64(z.data(), n); comp = ",o=z"; }
    }
    if (comp.empty()) payload = base64(rgba.data(), rgba.size());
    std::string keys = "a=T,f=32,s=" + std::to_string(f.w) + ",v=" + std::to_string(f.h) + ",i=" + std::to_string(kImageId) + ",p=1,z=-1,C=1,q=2"
                     + comp + ",c=" + std::to_string(vis_cols) + ",r=" + std::to_string(f.rows);
    if (cropx > 0) keys += ",x=" + std::to_string(cropx) + ",y=0,w=" + std::to_string(f.w - cropx) + ",h=" + std::to_string(f.h);
    std::string o = at_cell(f.row, f.col + (f.cols - vis_cols));
    constexpr size_t kChunk = 4096;
    for (size_t off = 0; off < payload.size() || off == 0; off += kChunk) {
        const bool more = off + kChunk < payload.size();
        o += "\x1b_G";
        if (off == 0) o += keys + ",";
        o += std::string("m=") + (more ? "1" : "0") + ";" + payload.substr(off, kChunk) + "\x1b\\";
        if (!more) break;
    }
    return o;
}

// ---- Sixel: 16 colours (from the hue) x 15 brightness steps; pixels below the threshold stay transparent (P2 = 1) ----
std::string sixel_emit(const GfxFrame& f) {
    static std::string cache;
    const int cw = f.w / std::max(1, f.cols);
    const int cropx = std::clamp(f.crop, 0, f.cols) * cw;
    const int vis_cols = f.cols - std::clamp(f.crop, 0, f.cols);
    if (vis_cols <= 0) return "";
    if (!f.fresh) return cache;   // no new picture this frame (the cells were written over): draw the last one again
    const int W = f.w - cropx, H = f.h;
    std::vector<uint8_t> idx(static_cast<size_t>(W) * H, 0);   // 0 = transparent, else 1 + hue16 * 15 + (step - 1)
    for (int y = 0; y < H; ++y)
        for (int x = 0; x < W; ++x) {
            const size_t at = static_cast<size_t>(y) * f.w + cropx + x;
            const int step = static_cast<int>(std::lround(curve(static_cast<float>(f.level[at]) / 255.0f) * 15.0f));
            if (step < 1) continue;
            idx[static_cast<size_t>(y) * W + x] = static_cast<uint8_t>(1 + (f.hue[at] >> 4) * 15 + (step - 1));
        }
    std::string o = at_cell(f.row, f.col + (f.cols - vis_cols));
    o += "\x1bP0;1;0q\"1;1;" + std::to_string(W) + ";" + std::to_string(H);
    for (int hb = 0; hb < 16; ++hb)
        for (int st = 1; st <= 15; ++st) {
            const float l = static_cast<float>(st) / 15.0f;
            const float hot = std::clamp((l - 0.80f) / 0.20f, 0.0f, 1.0f) * 0.40f;
            const auto& p = f.pal[static_cast<size_t>(hb * 16 + 8)];
            auto ch = [&](int c) { return static_cast<int>(std::lround(((static_cast<float>(p[static_cast<size_t>(c)]) * (1.0f - hot) + 255.0f * hot) * l) / 255.0f * 100.0f)); };
            o += "#" + std::to_string(1 + hb * 15 + st - 1) + ";2;" + std::to_string(ch(0)) + ";" + std::to_string(ch(1)) + ";" + std::to_string(ch(2));
        }
    for (int by = 0; by < H; by += 6) {
        const int bh = std::min(6, H - by);
        std::vector<char> used(256, 0);
        for (int y = 0; y < bh; ++y)
            for (int x = 0; x < W; ++x) used[idx[static_cast<size_t>(by + y) * W + x]] = 1;
        for (int c = 1; c < 256; ++c) {
            if (!used[static_cast<size_t>(c)]) continue;
            o += "#" + std::to_string(c);
            int run = 0; char prev = 0;
            auto flush = [&]() {
                if (run <= 0) return;
                if (run > 3) o += "!" + std::to_string(run) + prev;
                else o.append(static_cast<size_t>(run), prev);
                run = 0;
            };
            for (int x = 0; x < W; ++x) {
                int bits = 0;
                for (int y = 0; y < bh; ++y) if (idx[static_cast<size_t>(by + y) * W + x] == c) bits |= 1 << y;
                const char ch = static_cast<char>(63 + bits);
                if (run > 0 && ch == prev) ++run; else { flush(); prev = ch; run = 1; }
            }
            flush();
            o += "$";
        }
        o += "-";
    }
    o += "\x1b\\";
    cache = o;
    return o;
}

} // namespace

std::string gfx_emit(GfxProto proto, const GfxFrame& f) {
    if (proto == GfxProto::None || !f.active || f.cols <= 0 || f.rows <= 0) return "";
    if (proto == GfxProto::Kitty) return f.fresh ? kitty_emit(f) : std::string();
    return sixel_emit(f);
}

std::string gfx_clear(GfxProto proto) {
    if (proto == GfxProto::Kitty) return "\x1b_Ga=d,d=I,i=" + std::to_string(kImageId) + ",q=2\x1b\\";
    return "";   // Sixel pictures vanish when the cells are written over (every frame does that)
}

const char* gfx_name(GfxProto p) { return p == GfxProto::Kitty ? "kitty" : p == GfxProto::Sixel ? "sixel" : "none"; }

void gfx_cell_pixels(const std::string& override_wxh, int& w, int& h) {
    w = 0; h = 0;
    const char* e = std::getenv("MOUSIKI_RADIO_CELLPX");
    const std::string ov = e && *e ? e : override_wxh;
    int a = 0, b = 0;
    if (std::sscanf(ov.c_str(), "%dx%d", &a, &b) == 2 && a >= 4 && b >= 8) { w = a; h = b; return; }
#if !defined(_WIN32)
    struct winsize ws {};
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0 && ws.ws_row > 0 && ws.ws_xpixel > 0 && ws.ws_ypixel > 0) {
        w = ws.ws_xpixel / ws.ws_col; h = ws.ws_ypixel / ws.ws_row;
    }
#endif
    if (w < 4 || h < 8) { w = 10; h = 20; }
}

GfxProto gfx_probe(const std::string& pref_in) {
    std::string pref = pref_in;
    if (const char* e = std::getenv("MOUSIKI_RADIO_GFX")) if (*e) pref = e;
    for (auto& c : pref) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    if (pref == "off" || pref == "none" || pref == "0" || pref == "false") return GfxProto::None;
    if (pref == "kitty") return GfxProto::Kitty;
    if (pref == "sixel") return GfxProto::Sixel;
    const char* term = std::getenv("TERM");
    const char* prog = std::getenv("TERM_PROGRAM");
    const std::string t = term ? term : "", p = prog ? prog : "";
    const bool env_kitty = std::getenv("KITTY_WINDOW_ID") || t == "xterm-kitty" || t == "xterm-ghostty" || p == "ghostty" || p == "WezTerm";
    const bool env_sixel = std::getenv("WT_SESSION") || t.rfind("foot", 0) == 0 || t.find("sixel") != std::string::npos || t.rfind("mlterm", 0) == 0;
#if !defined(_WIN32)
    if (!isatty(STDIN_FILENO) || !isatty(STDOUT_FILENO)) return GfxProto::None;
    // Ask: a 1x1 Kitty graphics query, then the primary device attributes (every terminal answers those, so the wait ends).
    const char q[] = "\x1b_Gi=31,s=1,v=1,a=q,t=d,f=24;AAAA\x1b\\\x1b[c";
    if (::write(STDOUT_FILENO, q, sizeof q - 1) < 0) return env_kitty ? GfxProto::Kitty : env_sixel ? GfxProto::Sixel : GfxProto::None;
    std::string reply;
    for (int waited = 0; waited < 250;) {
        struct pollfd pfd = {STDIN_FILENO, POLLIN, 0};
        if (::poll(&pfd, 1, 25) <= 0) { waited += 25; continue; }
        char buf[256];
        const ssize_t n = ::read(STDIN_FILENO, buf, sizeof buf);
        if (n <= 0) break;
        reply.append(buf, static_cast<size_t>(n));
        const size_t da = reply.find("\x1b[?");
        if (da != std::string::npos && reply.find('c', da) != std::string::npos) break;   // the DA reply closes the exchange
    }
    if (reply.find("_Gi=31;OK") != std::string::npos) return GfxProto::Kitty;
    const size_t da = reply.find("\x1b[?");
    if (da != std::string::npos) {
        const size_t end = reply.find('c', da);
        const std::string attrs = ";" + reply.substr(da + 3, end == std::string::npos ? std::string::npos : end - da - 3) + ";";
        if (attrs.find(";4;") != std::string::npos) return GfxProto::Sixel;
    }
    return env_kitty ? GfxProto::Kitty : env_sixel ? GfxProto::Sixel : GfxProto::None;
#else
    return env_kitty ? GfxProto::Kitty : env_sixel ? GfxProto::Sixel : GfxProto::None;
#endif
}

bool gfx_compressed() { return true; }
