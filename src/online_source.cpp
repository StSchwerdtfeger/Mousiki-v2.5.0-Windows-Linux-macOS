#include "online_source.h"
#include "process_util.h"
#include <cctype>
#include <sstream>
#if defined(_WIN32)
#include "win_compat.h"
#endif

namespace muisc {

// Same tiny flat-JSON string extractor used in lyrics_fetcher.cpp — kept
// local here since yt-dlp's per-line JSON objects are the only thing that
// needs it in this file, not worth a shared JSON dependency for two spots.
static bool json_get_string(const std::string& json, const std::string& key, std::string& out) {
    std::string needle = "\"" + key + "\"";
    size_t kpos = json.find(needle);
    if (kpos == std::string::npos) return false;
    size_t colon = json.find(':', kpos + needle.size());
    if (colon == std::string::npos) return false;
    size_t qstart = json.find('"', colon);
    if (qstart == std::string::npos) return false;
    size_t i = qstart + 1;
    std::string raw;
    while (i < json.size()) {
        if (json[i] == '\\' && i + 1 < json.size()) { raw += json[i]; raw += json[i + 1]; i += 2; continue; }
        if (json[i] == '"') break;
        raw += json[i];
        ++i;
    }
    // unescape the common cases
    std::string clean;
    for (size_t j = 0; j < raw.size(); ++j) {
        if (raw[j] == '\\' && j + 1 < raw.size()) {
            char n = raw[j + 1];
            if (n == 'n') { clean += ' '; ++j; continue; }
            if (n == '"' || n == '\\' || n == '/') { clean += n; ++j; continue; }
        }
        clean += raw[j];
    }
    out = clean;
    return true;
}

// Bare (unquoted) numeric field, e.g. "duration": 213.0 or "duration": 213.
// yt-dlp's flat-playlist listings include this for most extractors, but
// not all -- false/untouched `out` just means "unknown", not an error.
static bool json_get_number(const std::string& json, const std::string& key, double& out) {
    std::string needle = "\"" + key + "\"";
    size_t kpos = json.find(needle);
    if (kpos == std::string::npos) return false;
    size_t colon = json.find(':', kpos + needle.size());
    if (colon == std::string::npos) return false;
    size_t i = colon + 1;
    while (i < json.size() && (json[i] == ' ' || json[i] == '\t')) ++i;
    if (i >= json.size() || json[i] == 'n') return false; // null / nothing there
    size_t start = i;
    while (i < json.size() && (std::isdigit(static_cast<unsigned char>(json[i])) || json[i] == '-' ||
                                json[i] == '+' || json[i] == '.' || json[i] == 'e' || json[i] == 'E')) ++i;
    if (i == start) return false;
    try {
        out = std::stod(json.substr(start, i - start));
    } catch (...) {
        return false;
    }
    return true;
}

// Shared by both search paths -- yt-dlp's flat-playlist -j output and
// fast_yt_search.py's output are both one JSON object per line, and
// fast_yt_search.py deliberately mirrors yt-dlp's field names for exactly
// this reason (see its own comment).
static std::vector<OnlineResult> parse_json_lines(const std::string& out) {
    std::vector<OnlineResult> results;
    std::istringstream stream(out);
    std::string line;
    while (std::getline(stream, line)) {
        if (line.empty() || line[0] != '{') continue;
        OnlineResult item;
        std::string uploader;
        json_get_string(line, "id", item.video_id);
        json_get_string(line, "title", item.title);
        if (!json_get_string(line, "uploader", uploader)) {
            json_get_string(line, "channel", uploader);
        }
        item.uploader = uploader;
        json_get_number(line, "duration", item.duration_sec);
        if (!item.video_id.empty() && !item.title.empty()) results.push_back(std::move(item));
    }
    return results;
}

static std::vector<OnlineResult> search_via_ytdlp(const std::string& query, int count) {
    // The query is user-typed text, so it goes through shell_quote() like every
    // other interpolated argument. Wrapped in plain double quotes (as before) a
    // POSIX shell would still expand $VAR, $(...), backticks and backslashes
    // inside it -- a search for `cost $5` or `"Heroes" (live)` ran a different
    // query than the one typed, and a crafted one could run a command.
    std::string cmd = "yt-dlp -4 --no-warnings --match-filters \"categories *= 'Music' & duration >= 90\" --flat-playlist -j " +
                       shell_quote("ytsearch" + std::to_string(count) + ":" + query);
    ProcResult r = run_capture(cmd);
    return parse_json_lines(r.out);
}

static std::vector<OnlineResult> search_via_fast_script(const std::string& query, int count,
                                                          const std::string& script_path) {
#if defined(_WIN32)
    const std::string& python = win_python_command();
    if (python.empty()) return {};
#else
    const std::string python = "python3";
#endif
    std::string cmd = python + " " + shell_quote(script_path) +
                       " " + shell_quote(query) + " " + std::to_string(count);
    ProcResult r = run_capture(cmd);
    return parse_json_lines(r.out);
}

std::vector<OnlineResult> OnlineSource::search(const std::string& query, int count,
                                                const std::string& fast_script_path) {
    if (!fast_script_path.empty()) {
        auto results = search_via_fast_script(query, count, fast_script_path);
        if (!results.empty()) return results;
        // Empty covers every failure mode uniformly (script missing,
        // Python missing, network error, or a genuine zero-result
        // query) -- yt-dlp gets a full attempt in every one of them.
    }
    return search_via_ytdlp(query, count);
}

// One JSON object per line with a page URL instead of a video id (SoundCloud via yt-dlp, Bandcamp via the script):
// the URL becomes the result's id (see OnlineResult).
static std::vector<OnlineResult> parse_url_lines(const std::string& out) {
    std::vector<OnlineResult> results;
    std::istringstream stream(out);
    std::string line;
    while (std::getline(stream, line)) {
        if (line.empty() || line[0] != '{') continue;
        OnlineResult item;
        std::string url, uploader;
        if (!json_get_string(line, "webpage_url", url) || url.find("://") == std::string::npos) json_get_string(line, "url", url);
        json_get_string(line, "title", item.title);
        if (!json_get_string(line, "uploader", uploader)) json_get_string(line, "artist", uploader);
        item.uploader = uploader;
        json_get_number(line, "duration", item.duration_sec);
        if (url.find("://") == std::string::npos || item.title.empty()) continue;
        item.video_id = url;
        results.push_back(std::move(item));
    }
    return results;
}

// A SoundCloud track whose every stream is a preview (the 30-second "snippet" SoundCloud plays for Go+ tracks and
// some label releases): yt-dlp marks those streams with "preview" in their format id. No stream at all (DRM, blocked
// in this country) cannot be played either.
static bool soundcloud_preview_only(const std::string& line) {
    const std::string key = "\"format_id\": \"";
    size_t at = 0;
    int all = 0, preview = 0;
    while ((at = line.find(key, at)) != std::string::npos) {
        at += key.size();
        const size_t end = line.find('"', at);
        if (end == std::string::npos) break;
        ++all;
        if (line.substr(at, end - at).find("preview") != std::string::npos) ++preview;
        at = end;
    }
    return all == 0 || preview == all;
}

std::vector<OnlineResult> OnlineSource::search_soundcloud(const std::string& query, int count) {
    // Not --flat-playlist: only the full listing has the streams, and only they tell a 30-second preview from the
    // whole track (the duration shown is the full one either way). Somewhat slower; a few more results are asked
    // for, since previews are dropped.
    const int ask = count + count / 2;
    std::string cmd = "yt-dlp --no-warnings --ignore-errors -j " + shell_quote("scsearch" + std::to_string(ask) + ":" + query);
    ProcResult r = run_capture(cmd);
    std::string kept;
    std::istringstream stream(r.out);
    std::string line;
    while (std::getline(stream, line))
        if (!line.empty() && line[0] == '{' && !soundcloud_preview_only(line)) kept += line + "\n";
    auto results = parse_url_lines(kept);
    if (static_cast<int>(results.size()) > count) results.resize(static_cast<size_t>(count));
    return results;
}

std::vector<OnlineResult> OnlineSource::search_bandcamp(const std::string& query, int count, const std::string& script_path) {
    if (script_path.empty()) return {};
#if defined(_WIN32)
    const std::string& python = win_python_command();
    if (python.empty()) return {};
#else
    const std::string python = "python3";
#endif
    std::string cmd = python + " " + shell_quote(script_path) + " " + shell_quote(query) + " " + std::to_string(count);
    ProcResult r = run_capture(cmd);
    return parse_url_lines(r.out);
}

std::vector<OnlineResult> OnlineSource::list_playlist(const std::string& url, std::string* error_out) {
    std::vector<OnlineResult> results;
    std::string trimmed = url;
    // Trim incidental whitespace a paste often carries.
    while (!trimmed.empty() && (trimmed.front() == ' ' || trimmed.front() == '\t')) trimmed.erase(trimmed.begin());
    while (!trimmed.empty() && (trimmed.back() == ' ' || trimmed.back() == '\t' || trimmed.back() == '\r' || trimmed.back() == '\n')) trimmed.pop_back();
    if (trimmed.empty()) {
        if (error_out) *error_out = "empty link";
        return results;
    }
    if (trimmed.find("http://") != 0 && trimmed.find("https://") != 0) {
        if (error_out) *error_out = "not a URL -- paste a youtube.com/playlist?list=... link";
        return results;
    }

    // --flat-playlist -j lists every entry (id/title/uploader) without
    // resolving/downloading any of them -- same fast enumeration
    // approach as search() above. Works for a playlist URL (many
    // entries) and degrades gracefully to a single entry for a plain
    // video URL.
    std::string cmd = "yt-dlp -4 --no-warnings --flat-playlist -j " + shell_quote(trimmed);
    ProcResult r = run_capture(cmd);
    if (!r.ok() || r.out.empty()) {
        if (error_out) *error_out = "yt-dlp couldn't list that link (exit " + std::to_string(r.exit_code) + ")";
        return results;
    }

    std::istringstream stream(r.out);
    std::string line;
    while (std::getline(stream, line)) {
        if (line.empty() || line[0] != '{') continue;
        OnlineResult item;
        std::string uploader;
        json_get_string(line, "id", item.video_id);
        json_get_string(line, "title", item.title);
        if (!json_get_string(line, "uploader", uploader)) {
            json_get_string(line, "channel", uploader);
        }
        item.uploader = uploader;
        json_get_number(line, "duration", item.duration_sec);
        if (!item.video_id.empty()) {
            if (item.title.empty()) item.title = item.video_id;
            results.push_back(std::move(item));
        }
    }
    if (results.empty() && error_out) *error_out = "no videos found in that playlist";
    return results;
}

} // namespace muisc
