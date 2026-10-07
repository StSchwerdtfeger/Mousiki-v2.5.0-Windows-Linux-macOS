#pragma once
// Radio mode -- Radio Browser (https://www.radio-browser.info/) station search.
//
// Self-contained like the rest of radio/: it knows nothing about the music player. The search runs on a
// worker thread, so the UI never waits for the network; every HTTP request is a `curl` child process
// (curl ships with Windows 10+, macOS and practically every Linux), because the project has no HTTP
// client library and adding one just for this is not worth it.
//
// API used (docs: https://docs.radio-browser.info/):
//   GET /json/stations/search?name=&tagList=&countrycode=|country=&state=&language=&bitrateMin=&bitrateMax=
//                            &hidebroken=true&order=clickcount&reverse=true&limit=N
//   GET /json/url/<stationuuid>      -- the "click counter": one call per tune, as the API asks
// Servers: the mirrors de1 / all / nl1 / at1 of api.radio-browser.info are tried in order; the one that
// answered last is tried first next time. MOUSIKI_RADIO_API=<base url> overrides the list (self-hosted
// mirror, or a test server).
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include "radio_stations.h"

namespace muisc::radio {

// What the six input panes hold, as typed.
struct BrowseQuery {
    std::string name;
    std::string tags;       // comma separated, every tag has to match
    std::string country;    // "Germany" (name) or "DE" (2-letter code)
    std::string state;
    std::string language;   // "german"
    std::string bitrate;    // "128" (at least), "64-192" (range), "-192" (at most)
};

// One station as Radio Browser describes it (the fields the UI shows).
struct BrowseStation {
    std::string uuid;
    std::string name;
    std::string url;            // the address the owner registered
    std::string url_resolved;   // playlists / redirects already followed -- what we tune to
    std::string homepage;
    std::string tags;           // comma separated
    std::string country;        // full name
    std::string countrycode;    // "DE"
    std::string state;
    std::string language;
    std::string codec;
    int bitrate = 0;            // kbps
    int votes = 0;
    int clickcount = 0;         // clicks in the last 24 h
    bool online = true;         // lastcheckok
    bool hls = false;
    std::string lastchecktime;  // "YYYY-MM-DD HH:MM:SS" (UTC)

    const std::string& stream_url() const { return url_resolved.empty() ? url : url_resolved; }
};

enum class BrowseState { Idle, Searching, Done, Error };

struct BrowseSnapshot {
    BrowseState state = BrowseState::Idle;
    std::string message;                                   // error text, or a short status
    std::shared_ptr<const std::vector<BrowseStation>> results;   // immutable once published; may be null
    unsigned generation = 0;                               // grows with every search() -- lets the UI notice new results
    std::string server;                                    // host that answered
};

// How many stations one search asks for.
constexpr int kBrowseLimit = 200;

// Query -> API parameters (empty fields are left out). Returns false and fills `error` for a bitrate
// that cannot be read. Public so the --rb-search / dump modes and tests can show what is sent.
bool build_browse_args(const BrowseQuery& q, std::vector<std::pair<std::string, std::string>>& args, std::string* error);

// Radio Browser JSON array -> stations (never throws; garbage gives an empty list and `ok` = false).
std::vector<BrowseStation> parse_browse_json(const std::string& json, bool* ok = nullptr);

// A result as a station-list entry (what `a` adds): genre = first tags, country = country code.
Station browse_to_station(const BrowseStation& b);

class RadioBrowser {
public:
    RadioBrowser();
    ~RadioBrowser();
    RadioBrowser(const RadioBrowser&) = delete;
    RadioBrowser& operator=(const RadioBrowser&) = delete;

    // Starts a search and returns at once. A search that is still running is superseded: its result is
    // dropped. An unreadable bitrate puts the state at Error immediately.
    void search(const BrowseQuery& q);

    // Counts a click for the station (fire and forget, the API asks for it every time a user starts a stream).
    void count_click(const std::string& uuid);

    BrowseSnapshot snapshot() const;

    // Blocks until the running search has finished (used by --rb-search; the UI never calls it).
    void wait();

private:
    struct Shared;
    std::shared_ptr<Shared> sh_;
};

} // namespace muisc::radio
