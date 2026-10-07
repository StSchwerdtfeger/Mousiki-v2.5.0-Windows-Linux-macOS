#pragma once
// Radio mode -- "find this song on YouTube and download it" service behind the HISTORY tab's `y` overlay.
//
// It owns no search / download code of its own: the search is the music player's OnlineSource::search (fast InnerTube
// script first, yt-dlp as the fallback) and the download is YoutubeSource::resolve_by_id, the very function the player's
// online search uses (yt-dlp -> opus file in the download folder). This class only runs them off the UI thread and keeps
// the state the overlay draws.
#include <filesystem>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace muisc::radio {

struct YtItem {
    std::string video_id, title, uploader;
    double duration_sec = -1.0;
    int state = 0;                 // 0 = not downloaded, 1 = downloading, 2 = done, 3 = failed
    std::string note;              // file name when done, the error when failed
};

struct YtSnapshot {
    bool searching = false;
    bool searched = false;         // a search has finished at least once
    std::string query;             // the query of the last search
    std::string error;             // empty unless the last search found nothing / failed
    std::vector<YtItem> items;
};

class RadioYt {
public:
    RadioYt();
    ~RadioYt();                    // waits for running jobs (a download in progress is not interrupted)
    RadioYt(const RadioYt&) = delete;
    RadioYt& operator=(const RadioYt&) = delete;

    void set_download_dir(const std::filesystem::path& dir);
    void search(const std::string& query);     // async; a newer search supersedes an older one
    void download(size_t index);                // async; ignored when that item is already downloading / done
    YtSnapshot snapshot() const;
    void clear();                               // forgets results (the overlay opened for another entry)

private:
    struct State;
    std::shared_ptr<State> st_;
};

} // namespace muisc::radio
