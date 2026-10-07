# Mousiki radio mode (prototype)

Self-contained module with its **own audio path**. It shares only UI plumbing
(`settings`, `terminal_ui`, `process_util`) and the two visualizers
(`FftVisualizer`, `OscilloscopeVisualizer`) with the music player -- not `Player`,
not `StreamingPcm`.

| file | job |
|---|---|
| `radio_engine.*`   | ffmpeg child -> worker thread -> 10 s ring -> own miniaudio device; reconnect with back-off; ffprobe poll for ICY metadata |
| `radio_ui.*`       | the 120x30 screen, pure rendering, colours from `radio_config.txt` |
| `radio_settings.*` | the radio's own settings: colour schema, `radio_config.txt` loader / saver, colour helpers (no `src/settings.*`) |
| `radio_sign.h`     | the ON AIR braille art (edit here; keep rows equal width) |
| `radio_sign.h`     | the ON AIR sign art (braille, generated from `on_air_sign.txt`, blank edge columns trimmed) |
| `radio_stations.*` | `stations.txt` loader / writer + built-in defaults, presets file |
| `radio_browser.*`  | Radio Browser search client: `curl` child process on a worker thread, own JSON reader, query builder |
| `radio_main.cpp`   | standalone entry point, key handling, `--dump` / `--selftest` |

## Build
`cmake -B build && cmake --build build --target mousiki_radio` (option `MOUSIKI_BUILD_RADIO`, ON by default).

## Run
`mousiki_radio` -- interactive, needs a terminal of at least 120x30.
`mousiki_radio --dump` -- one idle frame as plain text.
`mousiki_radio --dump-cheat [scroll]` -- the cheatsheet as plain text.
`mousiki_radio --selftest [seconds]` -- headless run on the offline test signal (null audio device); prints the frame, checks 30x120 geometry.

## Keys
`Up/Down` move (`j/k` only work inside the menus) - `Enter` tune - `n` next channel in the list (always; `Left` / `Right` do nothing in the main UI any more) - `#` shuffle: a random next channel (always) - `b` previous channel, follows the mode: `S` the channel played before, `L` the one before it in the list - `m` switch the mode, `S` shuffle <-> `L` list (box next to SEARCH; it only affects `b`) -
`1 2 3 4 5 6 7 8 9 0 e r t d f g` recall slot 1-16 of the active preset - `Shift+Left/Right` previous / next preset - `Shift+K` open the PRESETS menu -
`/` search (`Enter` tunes the hovered hit, `Esc` clears; text-field keys below) - `p` mute / unmute (the small box between the buttons reads MUTE, or TUNE while muted; with no channel loaded it tunes the hovered station like `Enter`) - `+/-` volume - `R` reconnect - `Shift+S` open the RADIO BROWSER search menu - `Shift+P` open the STATION LISTS menu - `Shift+T` toggle the STATIONS sort (list order <-> name A-Z, also for search results) - `x` stop - `Shift+Z` sleep timer - `y` record the tuned stream - `h` open the LISTENING HISTORY - `L` open the big STATIONS overlay - `s` open the RADIO SETTINGS - `?` cheatsheet (all keys, categorized, scrollable) - `q` or `Ctrl+C` quit
(Reconnect is the capital letter `R`, because `r` is a preset key.)

## Presets
A **preset** is a named set of 16 station slots (keys `1234567890ertdfg`). You can keep as many as you like and switch
between them; the active one is shown in the title of the PRESETS pane (`PRESETS (Morning)`), in the main screen and in
the menu. `Shift+Left` / `Shift+Right` switch to the previous / next preset from the main screen (and from any pane of the menu).

## PRESETS menu (`Shift+K`)
Full-screen, laid out like the player's playlist menu, top to bottom:

1. SEARCH STATION / SEARCH PRESET -- one line. `s:` (default) filters stations, `p:` filters presets. Type `p:` into the empty box
   (or press `/` while in SELECT PRESET) to search presets, `s:` to go back (`/p:` / `/s:` still work).
2. SELECT PRESET -- as wide as the search bar, 4 columns x 2 rows of preset names (scrolls by rows). The active preset is
   highlighted like the playing station.
3. STATIONS (the full list, or the search results) and PRESETS (the 16 slots of the active preset) side by side, equal width.

A small `◀` behind a pane's title shows which pane has the keyboard. `Tab` cycles SEARCH -> SELECT PRESET -> STATIONS.

| key | where | does |
|---|---|---|
| `Tab` | anywhere | next pane |
| typing / `Backspace` | search | edit the search (stations: name, genre, country; presets: name) |
| `Up/Down` (`j/k`), `Left/Right` (`h/l`) | SELECT PRESET | move in the 4-column grid |
| `Up/Down` (`j/k`) | STATIONS | move in the list |
| `Enter` | STATIONS | tune the hovered station |
| `Enter` | SELECT PRESET | open the hovered preset (its slots appear in PRESETS and on the main screen) |
| `Enter` | search | tune the hovered station / open the hovered preset, then jump to that pane |
| `Left/Right`, `Shift+Left/Right`, `Home/End`, `Del`, `Ctrl+C/X/V` | search, name overlay | text editing, see below |
| `Shift+N` | SELECT PRESET | new preset: small name overlay (same look as the player's Oscilloscope overlay); `Enter` creates and opens it, `Esc` cancels |
| `Shift+C` | SELECT PRESET | rename the hovered preset (same overlay) |
| `1`..`0`, `e r t d f g` | STATIONS | **set the hovered station as that slot of the active preset** (same key again clears it; a station sits in one slot only, so it moves) |
| `Del` / `Backspace` | STATIONS | remove the hovered station from the active preset |
| `Shift+Left/Right` | anywhere | previous / next preset |
| `/` | STATIONS / SELECT PRESET | back to the search box (stations / presets) |
| `Esc` | search | clear the search; if it is already empty, close the menu |
| `Esc` | STATIONS / SELECT PRESET | close the menu |

Names are 1-24 characters, unique (case-insensitive); `[ ] |` are replaced by `( ) /` because they are file syntax.

Presets are saved on every change to `presets.txt` (`~/.config/mousiki/`, Windows `%USERPROFILE%\.config\mousiki\`):

```
@keys 1234567890ertdfg
@active Morning
[Default]
1 | https://stream.radioparadise.com/aac-128 | Radio Paradise
[Morning]
1 | https://st01.sslstream.dlf.de/dlf/01/128/mp3/stream.mp3 | Deutschlandfunk
```
The `@keys` line names the key letters the file was written with (slot 1 = first letter ... slot 16 = last). A `presets.txt` without it
comes from before the key change (`1234567890qwertz`) and is read with those letters, so old presets keep their slots; the next save
rewrites it with the new keys.
An older `presets.txt` without `[..]` headers is read as the preset "Default". Until the file exists, the stations marked `*`
in stations.txt fill the first slots of "Default" in file order.
`mousiki_radio --dump-menu [text] [p|new|rename]` prints the menu as plain text (`p`: `text` goes into the preset search;
`new` / `rename`: show the name overlay with `text` typed into it).

## RADIO BROWSER menu (`Shift+S`)
Searches the public station directory <https://www.radio-browser.info/> (API docs: <https://docs.radio-browser.info/>).
Six input panes (two per row) on top, **RESULTS** and **STATION INFO** (all details of the hovered result) side by side below:

| pane | what you type | sent as |
|---|---|---|
| NAME | part of the station name | `name` |
| TAGS | comma separated, **every** tag has to match: `rock, 80s` | `tagList` |
| COUNTRY | a name (`Germany`) or a 2-letter code (`DE`) | `country` / `countrycode` |
| STATE / REGION | part of the region name | `state` |
| LANGUAGE | as Radio Browser spells it: `german` | `language` |
| BITRATE | kbps: `128` = at least 128, `64-192` = range, `-192` = at most | `bitrateMin` / `bitrateMax` |

Every search also sends `hidebroken=true`, `order=clickcount&reverse=true` (most clicked first) and `limit=200`. Empty panes are left out.

| key | does |
|---|---|
| `Shift+S` | open the menu (main screen) |
| `Tab` | NAME -> TAGS -> COUNTRY -> STATE -> LANGUAGE -> BITRATE -> RESULTS -> NAME |
| `Enter` | in a pane: search (when the results arrive the focus jumps to RESULTS); in RESULTS: tune the hovered station |
| `Up/Down` | in a pane: jump one pane row up / down (down from the last row: RESULTS); in RESULTS: move the cursor |
| `a` | in RESULTS: add the hovered station to your station list (appended, so presets keep their slots; written to `stations.txt`) |
| `/` | in RESULTS: back to NAME |
| `Esc` | in a pane: clear it; empty pane or RESULTS: close the menu |
| `?` / `Ctrl+C` | cheatsheet (RESULTS only) / quit (in a pane Ctrl+C copies) |

A result is lit as tuned whatever way the station was tuned (this menu, the main STATIONS list, a preset ...). Streams are compared by
`same_stream_url()`: `http`/`https`, host case, a trailing `/` and tracking parameters (`?aggregator=web`, `utm_*`, ...) are
ignored, so `.../stream.mp3` and `.../stream.mp3?aggregator=web` are the same stream. The same rule decides the dot ("already in
your list"), the duplicate check of `a` and which list entry a tuned result belongs to.

The line under the results always shows what is tuned in right now (`Tuned in: ...`, with the track title when the stream sends one); a
result that is the tuned station is lit like in the main STATIONS pane.

A dot in front of a result means its stream is already in your station list; results whose last check failed are dimmed (and
hidden by default). Tuning counts one click at Radio Browser (`/json/url/<uuid>`), as the API asks. A station that is not in
your list can be tuned and reconnected (`R`) like any other.

Needs `curl` on the PATH (Windows 10+, macOS and most Linux have it). Servers tried in order: `de1`, `all`, `nl1`, `at1`
`.api.radio-browser.info`; the one that answered last goes first. `MOUSIKI_RADIO_API=<base url>` replaces the list (own mirror, tests).
Test the connection without the UI: `mousiki_radio --rb-search name=jazz tags="smooth, lounge" country=DE bitrate=128-320`;
`mousiki_radio --dump-browse` draws the menu with made-up results.
If `a` finds no `stations.txt` yet it creates `~/.config/mousiki/stations.txt` from the built-in list plus the new station.

## stations.txt
`name | genre | country | codec | kbps | url | [*] | [dial MHz]` -- see `radio/stations.txt`.
Put it in `~/.config/mousiki/` (Windows: `%USERPROFILE%\.config\mousiki\`) or next to the executable.
`lavfi:chords|tone|noise` URLs are offline test signals generated by ffmpeg.

## ON AIR sign
Diagonal gradient (top-left to bottom-right) of the visualizer colours, plus a travelling colour wave while live; dark otherwise.
Wave speed / crest count / strength are `SignWave` in `radio_ui.h` (`UiModel::wave`) -- ready to be fed from settings.

## Resizing
Minimum terminal 120x30, widths are clamped to 200. The frame is exactly as tall as the terminal (30 lines at 30 rows); its last
line is written without a trailing newline, so it cannot scroll. A wider terminal widens the top pane (only the oscilloscope grows),
the frequency band and the search box; a taller one only gives rows to STATIONS and PRESETS (50/50 split, 1 column gap). The PRESETS menu keeps its 16 rows. The band,
button pane and search keep their height; the band and the button pane are separated by one blank column. With only 4 list rows the 16 presets switch to 4 columns.
The hovered station row scrolls like a marquee when its name is too long (1.2 s hold, 4 columns/s -- same as the player's list).
`MOUSIKI_RADIO_SIZE=160x40 mousiki_radio --dump` renders a frame for any terminal size.

## ON AIR sign colours
The sign carries a diagonal (top-left -> bottom-right) gradient of the visualizer colours (`ColorVizLeft/Center/Right`),
with a travelling colour wave on top. The wave is `SignWave` in `radio_ui.h` (`speed`, `waves`, `amount`) -- meant to become settings.
The sign is grey while the stream is not live.

## STATION LISTS menu (`Shift+P`)
Named, ordered collections of stations, built and managed like the music player's playlist editor. Two tabs, switched with
`Alt+Left/Right` (`Option` on macOS) from every pane; the top pane STATION LISTS uses the same tab strip as the settings screen (`[CREATE / EDIT]` = current tab, no numbers, square indents) and has a single content row.

**Tab 1 - CREATE / EDIT.** The top pane's one row holds the list's `Name:` field: 25 characters at most (`kListNameMax`; longer names in an old file are cut to 25 on load), drawn as a 25-column field that turns red while you type, like the path fields in the settings. On tab 2 the row shows which list is open in tab 1. Below it SEARCH ALL STATIONS (name, genre, country),
then STATIONS (all stations, or the search results when the box is not empty) beside LIST CONTENTS (what you are building).

**Tab 2 - SAVED STATION LISTS.** The top pane (tab names only), SEARCH STATION LISTS and the STATION LISTS results.
`Enter` on a list moves its content into the editor on tab 1.

| key | where | does |
|---|---|---|
| `Alt+Left/Right` | anywhere | switch tab |
| `Tab` | anywhere | tab 1: name -> search -> STATIONS -> LIST CONTENTS; tab 2: search <-> list |
| `Enter` | name | on to the search |
| `Enter` | search, STATIONS | add the hovered station to the end of the list (a station is in a list once) |
| `Enter` | LIST CONTENTS | tune the hovered station (to audition the list) |
| `Shift+T` | STATIONS | sort: list order (the order the stations were added to `stations.txt`) <-> name A-Z; also sorts search results |
| `4` / `5`, `Del` / `Backspace` / `d` | LIST CONTENTS | move the hovered station up / down, remove it |
| `s` or `Home` | `s`: STATIONS / LIST CONTENTS; `Home`: every pane of tab 1 | save under the typed name, stay in the menu (same name overwrites; empty name: "enter a name first") |
| `Enter` | tab 2 search / list | to the list / load the hovered list into tab 1 |
| `Del` | tab 2 list | delete the hovered list after a Yes/No prompt |
| `/` | STATIONS, LIST CONTENTS, tab 2 list | back to the search box |
| `Esc` | search with text | clear it |
| `Esc` | otherwise | close; with unsaved changes on tab 1 it asks `Save changes to "x" before exiting? [Y]es [N]o [ESC] cancel` |

`s` is a plain letter, so inside the name and search boxes it is typed; `Home` saves from anywhere (it cannot appear in text).
Like the player, opening the menu starts a fresh list; an existing one comes back only through tab 2 -> `Enter`.
A dot behind a station in STATIONS means it is already in the list being built.
Lists are saved to `~/.config/mousiki/stationlists.txt` (Windows `%USERPROFILE%\.config\mousiki\`):
```
[Morning drive]
https://stream.radioparadise.com/aac-128 | Radio Paradise
```
Stations are matched by URL, then by name; one that has been removed from `stations.txt` is dropped when the list is read.
`mousiki_radio --dump-lists [2|prompt]` prints the menu with sample data (tab 1, tab 2, the unsaved-changes prompt).

### Station lists in the main search
Type `p:` into the empty search box (`/` first; `/p:` works too) to search the saved station lists: the box reads `/p:`, the
STATIONS pane turns into STATION LISTS (name, station count) and filters live while you type. `Up/Down` pick one, `Enter`
fills the STATIONS pane with that list's stations (in the list's order, the search box is emptied). While a list is shown the
pane title reads `STATIONS (LIST: name - ESC: all)` instead of the sort; searching and `Shift+T` still work inside the list.
`Esc` in the main screen clears a search that is still in the box, and the next `Esc` returns to all stations. `s:` in the empty
box goes back to searching stations. Search text is no longer repeated in the pane title (`filter: ...` is gone).
While a list is shown its rows are numbered by position in the list (1, 2, 3 ...), not by the station's number in the full list.
`Enter` in the search box with no hit keeps the focus in the box, so the query can be edited or cleared with `Esc`.

## Not done yet
- station lists can be shown in the main STATIONS pane (`p:`) but not yet used for `n` / `b` / `#` channel surfing or the presets
- not hooked into the main `App` (it would own a `RadioEngine` and call `render_radio_frame()`)
- hotkeys are fixed, not read from `settings_.hotkeys`
- the PRESETS menu has no mouse/page-jump keys (arrows only); presets cannot be deleted from the menu yet (edit presets.txt)
- the Windows build is written against the same defines as the main target but has not been compiled by me

## Text fields
The main SEARCH box, the menu's search box and the preset name overlay edit text like the music player's fields
(`radio_textedit.h` is a copy of the player's editor): `Left/Right` move the caret, `Shift+Left/Right` mark text,
`Home/End` jump to the ends, `Backspace` / `Del` delete the mark or one character, `Ctrl+C` copies the mark (nothing
marked: the whole field), `Ctrl+X` cuts it, `Ctrl+V` pastes (replacing the mark); typing replaces a mark too.
While a text field has the keyboard `Ctrl+C` copies instead of quitting, and `Shift+Left/Right` mark text instead of
switching the preset.


## Radio settings (`s`)

The radio has its own settings screen and its own file, `radio_config.txt` (`~/.config/mousiki/`, Windows `%USERPROFILE%\.config\mousiki\`, or next to the program). It never reads the music player's `config.txt`. Stopping the stream is `x` now.

The screen has the music player's square frame and tab strip (`[COLORS]` / `[ON/OFF]` / `[ANIMATION]` / `[PATHS]` / `[REFERENCE]` / `[ABOUT APP]`, `Tab` switches). Keys on COLORS: `Up/Down/Left/Right` move, `Enter` edits the cell (a palette number 0-255; empty or 0 = the terminal's own colour), `s` saves, `Esc` / `q` leaves (unsaved changes are saved on leaving and on quit). On ON/OFF, `Up/Down` pick a row and `Left/Right` / `Enter` toggle it.

| Row | Cells | Colours |
|---|---|---|
| BORDER_COLOR | TOP, BOTTOM | the frame lines (fade top to bottom) |
| ON_AIR | UPPER_LEFT, BOTTOM_RIGHT | the ON AIR sign's diagonal gradient wave (also needle, lamp, LIVE text) |
| METADATA | KEY, VAL | station info labels / values (and the channel name next to the needle) |
| VIZ | LEFT, RIGHT | the FFT spectrum under the station info (`ColorVizCenter` = optional middle stop, config file only) |
| OSCI | LEFT, RIGHT | the oscilloscope |
| FREQUENCY_BAR | LINE, MHz | the frequency line and ticks / the numbers and the small dots |
| VOLUME | CURRENT, POSSIBLE | `#` / `-` of the volume bar |
| LIST | INACTIVE, PLAYING, CURSOR (FG, BG) | STATIONS rows (also the station names in PRESETS) |
| PRESETS | INACTIVE FG/BG, KEY FG | unassigned preset slots / the short-cut key letters (main pane and PRESETS menu) |
| HEADER | TEXT | captions (tuning / buffering / reconnecting), empty-pane messages |
| LEGEND | TEXT | key command hint lines, their errors, cheatsheet titles |
| TAB_NAMES | CURRENT, OTHER | the settings screen's tab names: the current `[tab]` (and the `< ↔ >` hint) / the others |

### ANIMATION tab

| Row | Effect |
|---|---|
| Vis. Fluidity | 1-10: how smoothly the spectrum bars rise |
| Gradient wave speed | cycles per second of the ON AIR sign's colour wave (default 0.25) |
| Pulse wave speed | loops per second of the circle burst, independent of the colour wave (default 0.125 = the old half of the colour wave) |
| Playback mode | list / shuffle for the station surf (`m`, the S/L box next to the search) |
| Vis. Degradation | 1-10: how fast bars fall |
| Vis. Viscosity | 0-10: how strongly neighbouring bars are blended |

The player's Waveform Style (seek-bar waveform), Lyrics Alignment and Lyrics Animation do not exist in the radio. Both speeds use the steps 0.01 .. 1.00 (config file: any value in that range).

### ON/OFF tab

| Row | Effect |
|---|---|
| On air ascii | the ON AIR sign and its colour wave |
| Pulse wave | the faint circle burst behind the sign |
| Dummy buttons | the `<<< MUTE >>>` boxes next to the frequency band (the volume bar stays) |
| Osci/sphere | right of the station info: oscilloscope, the player's audio-reactive sphere, or off |
| Visualizer | the FFT spectrum under the station info |
| Stereo sound | off = left and right are folded to mono (the footer of the band then says MONO) |
| Normalize volume | loudness normalisation (below) |
| Osci style | `braille` (the oscilloscope as braille dots) or `image` (a real pixel picture drawn by the terminal itself, see below). Config: `OsciStyle=` |
| Tuning noise | off by default. On: radio static fades in (about 0.1 s) when another station is tuned, stays while the new stream connects (at least 0.45 s, at most 6 s) and fades out (about 1 s) once it plays. It is synthesised in the audio callback (pink noise with a slight wobble and rare pops), follows volume, mute and the sleep-timer fade, and is not recorded. Config: `TuneNoise=` |

With the sign on, the scope takes all free width; with the scope off the sign and the station info are centred in the pane. With On air ascii off (like the player's disk off) the station info moves to the left edge and the scope is centred in the rest of the pane; the pulse wave lives behind the sign, so it goes with it. There is no emoji row and no metadata row (the player's one switches file name / tag title, which the radio does not have): the radio always draws emoji as `?`.

`SHIFT+O` opens the oscilloscope overlay (afterglow, dot threshold, tail; `Up/Down` pick, `Left/Right` change, `R` reset, `Esc` / `SHIFT+O` close and save). `v` toggles loudness normalisation, `SHIFT+V` opens its overlay (on/off, target level -40..0 LUFS, max boost 0..24 dB, a live line with the stream's measured loudness and applied gain; `Space` on/off, `R` reset). The loudness is measured over the stream since it was tuned (BS.1770 gating, like the player) and the gain glides in over about a second; it restarts on tune and when Stereo sound changes.

Previously the code called these roles the other way round (hints used `header`, captions used `legend`); the names now say what they colour. Defaults reproduce the old look. Colours from the player's `config.txt` are not carried over. `VisualizerFluidity` / `DegradationSpeed` / `Viscosity`, `Osci*` (also the SHIFT+O overlay), `Box*` and `ListSeparator` are config-file-only for now.


### PATHS tab

Where the radio keeps its files. Every path is a *base*; the radio creates its own sub-folders below it (nothing is moved or deleted, files that do not exist at the new place yet are copied there).

| Row | Result |
|---|---|
| STATION LISTS PATH | `<path>/stations/stations.txt` (the overall list) and `<path>/station_lists/stationlists.txt` (the lists made in `Shift+P`) |
| PRESETS PATH | `<path>/presets/presets.txt` |
| DOWNLOAD PATH | `<path>/radio_downloads/` -- recordings (`y`) and YouTube downloads from the history |
| Same folder as music player | ON = use the music player's `DownloadFolder` (from `~/.config/mousiki/config.txt`; without one the player's cache folder, `~/.cache/mousiki`) instead of DOWNLOAD PATH |
| HISTORY PATH | `<path>/radio_history/` (`history.txt`, `archive.txt`) |

Empty = the default: the lists, presets and `radio_config.txt` sit directly in `~/.config/mousiki/` (Windows `%USERPROFILE%\.config\mousiki\`; **not** in `.cache`), downloads and recordings follow the music player (the toggle is ON by default; OFF with an empty DOWNLOAD PATH = `~/Music/radio_downloads`), history to `~/.config/mousiki/radio_history`. `Enter` edits a path (a normal text field, `~` = home; `Enter` applies, `Esc` cancels), the bool row toggles. The screen shows the resulting locations underneath. When the stations / presets / lists files change the playback is stopped and everything is reloaded from the new place.

## Recording (`y`)
`y` starts recording what the tuned station delivers (before volume, mute and the mono fold, so it is the stream as it is). The footer of the frequency band shows `REC mm:ss`. `y` again stops; the recording is a WAV while it runs and is converted to `<YYYY-MM-DD_HH-MM-SS>.mp3` (libmp3lame, ID3 title = the current "artist - title" or the station) in the download folder by ffmpeg in the background; the WAV is removed afterwards (kept if the conversion fails). Tuning another station or `x` ends the recording too. A recording shorter than half a second is discarded.

## Listening history (`h`)
One fused pane: `LISTENING HISTORY` rests on the top border line, followed by the settings-style tab strip (no numbers), and the bottom border carries the info (`14 lines (max 10000)`, or the sort direction on TOP CHANNELS). Three tabs (keys `1 2 3`, `Left/Right`, `Tab`, `Up/Down`, `Home/End`, `Esc` / `q` / `h` close):

* **HISTORY** -- WHEN / CHANNEL / ARTIST / TITLE / HEARD. A new line starts whenever the channel, the artist or the title changes (lines heard for less than 3 s are dropped; a line that only has the channel name is completed in place when the first title arrives within 20 s). Hover a line and press `y`: a small overlay searches YouTube for "artist title" (the player's own `OnlineSource::search`: the fast `scripts/fast_yt_search.py` first, yt-dlp as the fallback), `Enter` downloads the hovered result with the player's `YoutubeSource::resolve_by_id` (yt-dlp, opus) into the download folder; `[..]` downloading, `[ok]` done, `[!!]` failed. `Tab` edits the query, `Esc` closes the overlay.
* **TOP CHANNELS** -- channels by time listened (`r` flips most / least first; the header shows `LISTENED ▼` / `▲`, a rule separates it from the rows).
* **HABITS** -- sessions (a 30 min gap starts a new one), time per day, channels, listening by hour of the day and by weekday.

The history is light-weight like the player's: the newest 10 000 lines are kept in memory and in `history.txt` (one tab-separated line appended when a line ends, no rewrite per song). Beyond 10 000 the oldest line leaves the list; before that it is folded into `archive.txt` (per-channel totals, per-day / hour / weekday seconds), so TOP CHANNELS and HABITS stay lifetime figures.


### REFERENCE tab
The keys that are not hard coded, grouped like the player's REFERENCE tab (a note at the top points to the cheatsheet `?` for the full list). `Enter` edits the hovered key: type one character (case matters, `S` and `s` are different keys) or `SPACE` / `TAB` / `BACKSPACE`; `Enter` applies, `Esc` cancels, `Del` restores the default. A key that is already used is refused with `KEY "n" ALREADY USED BY NEXT CHANNEL -- try another key` (the fixed keys count too: the preset slot keys `1234567890ertdfg` and `j` / `k`). Enter, Esc and the arrows keep their jobs and cannot be bound. The cheatsheet always lists the default keys. Stored as `HKey<Action>=` lines in `radio_config.txt`.

### ABOUT APP tab
The same text as the music player's ABOUT tab (`Up/Down` scroll).

## Sleep timer (`Shift+Z`)
15 / 30 / 60 / 90 / 120 minutes, then the stream is **stopped** (there is no song to wait for in a radio, so the player's "stop after current song" does not exist here). The search box shows `[SLEEP 24:10]` while it runs; picking `Off` cancels it. **Fade out** (on by default, saved as `SleepFade`): over the last 10 % of the time (at least 30 s, at most 10 min: 15 min -> 90 s, 30 -> 3 min, 60 -> 6 min, 90 -> 9 min, 120 -> 10 min) the volume glides down with gain = t^2 (t = fade time left / fade length: -12 dB half-way, -20 dB at 30 %), so the first half is hardly noticed and the end is a gentle slide into silence. The fade multiplies your volume setting and is undone when the timer is cancelled.

## OFF AIR
While nothing is live (idle or failed) the OFF AIR sign takes the place of the ON AIR sign with the same colour wave (no pulse wave), there is no caption under it, "Currently No Station Tuned" uses the list colour, and the pane on the right (where the oscilloscope / sphere lives) shows a satellite that swings +-25 degrees (a sine, so it slows down towards both ends). The signs live in `radio_sign.h`.


## Big STATIONS overlay (`L`)
The STATIONS pane with the whole screen for its rows: the same search box and state as the main screen (`/` searches, `p:` searches the station lists and `Enter` opens one, `s:` back, `Shift+T` sorts, `Esc` clears the search, leaves a list, then closes), plus a PRESET NAME column. `Enter` tunes the hovered station; `n` `b` `#` `p` `x` `+` `-` work as in the main screen. Keys that would open another menu are ignored while it is open.

* `a` adds a station by its stream URL (`TAB` switches URL <-> NAME; an empty NAME becomes the host name). It is appended to the end of the list (so the preset slots stay valid) and to `stations.txt`; a stream that is already in the list is refused.
* `Shift+C` gives the hovered station a **preset name**: a shorter second name that is shown in the PRESETS pane instead of the station name (empty = the station's own name again). Stored in `preset_names.txt` (`url<TAB>name`) in the same folder as `stations.txt`, so it moves with it when the PATHS tab changes the folder. The search also looks at it.
* The tuned entry of the PRESETS pane scrolls (marquee, like the hovered rows of the lists) when its name does not fit.

## Search
All searches (stations, station lists, presets, the STATION LISTS menu) are typo tolerant like the music player's: an exact substring ranks first (earlier hit first), then close fuzzy matches (every query word needs a word within edit distance, quality >= 0.55); `-` `_` `.` `/` count as spaces. With a query the best match comes first; `Shift+T` (A-Z) overrides that order. The scoring is `radio_fuzzy.h`, a copy of the player's `fuzzy_score()` (that one sits in `app.cpp`'s anonymous namespace). The Radio Browser menu searches on the server and is not fuzzy.

### REFERENCE: reset and undo
The last row of the tab, `Reset All Keys To Default`, puts every key back (`Enter`). `Ctrl+Shift+U` undoes the last key change (a new key, `Del`, or a reset), up to the last 5, newest first; it works from any settings tab, switches to REFERENCE and moves the cursor to the key that came back (after an undone reset: the first key that changed). Needs a terminal that reports `Ctrl+Shift+U` (kitty keyboard protocol / xterm modifyOtherKeys, like the player's `Ctrl+Shift+S`).

## ENTER tunes in
`Enter` on a station list (after `p:`, in the main screen and in the big overlay) opens the list and tunes its first station. `Enter` on a preset (PRESETS menu, SELECT PRESET) opens it and tunes its first filled slot.

## Oscilloscope overlay (`Shift+O`)
Besides decay, dot threshold and tail the overlay has: **Line/Vec. Interpol.** (on = the samples are joined by lines; off = only the sample dots), **Z-Axis (XYZ Mode)** on/off with **Z Depth** (0-1) and **Z Source** (`speed`: the faster the beam moves the dimmer it is, like a CRT; `level`: the further from the centre the brighter), **Trace Length** (128-1024 samples per frame: short = sharper, long = more complete figure), **Rotate 45 deg (M/S)** (mono signals stand upright, stereo width spreads sideways) and **Mono Phase Portrait** (near-mono signals draw a circle-like phase portrait instead of a diagonal line) and **Color** (see below). `Up/Down` pick, `Left/Right` change (on/off rows: left = off, right = on), `R` resets. The braille grid is coarse, so Z depth, trace length and interpolation show most clearly with a low dot threshold and a long afterglow. The scope code is the radio's own copy (`radio_scope.*`); the player's is untouched.

## Lists scroll like the player's
The window of a list only moves when the cursor leaves it: walking back up after a long way down moves the cursor up inside the window first.

## Tuning noise (changed)
One continuous bed of static (pink noise plus a little hiss, no pulsing) with short glitches in it: crackle bursts, sample-and-hold stutters, dropouts and digital crunch, 2-16 ms long every 25-250 ms.

## Oscilloscope image style (real pixels in the terminal)
ON/OFF -> `Osci style: image` draws the scope as a pixel image through the terminal's graphics protocol, like yazi's previews: the picture is as many pixels as the scope block has (e.g. 360 x 280 at 10 x 20 pixel cells), with a phosphor bloom around the beam, a white-hot core on the brightest parts and a transparent background (your terminal background shows through). The same controls, colours and temperature mapping apply. Supported: **Kitty graphics** (Kitty, WezTerm, Ghostty, Konsole, ...: the image sits under the text and is replaced in place, zlib-compressed, about 60 kB per frame) and **Sixel** (Windows Terminal 1.22+, foot, mlterm, xterm with sixel on, ...: 16 colours x 15 brightness steps). Without either, `image` falls back to braille. The terminal is asked once at start (a Kitty query plus the device attributes, at most 0.25 s); `OsciImageProtocol=auto|kitty|sixel|off` in `radio_config.txt` (or the environment variable `MOUSIKI_RADIO_GFX`) forces a choice. The pixel size of a cell is asked from the terminal (10 x 20 when it does not report it; the environment variable `MOUSIKI_RADIO_CELLPX=WxH` can override). A small overlay (Shift+O etc.) that reaches into the scope block hides the covered left part of the picture. Other menus take the whole screen and remove the picture. On Windows the protocol is chosen from the environment (Windows Terminal = Sixel); that path is untested here.

## Oscilloscope overlay, second round
The overlay (`Shift+O`) now also holds the shared rows **Display** (osci / sphere), **Osci style** (braille / image), **Frame rate** (30 / 45 / 60 / 90 fps), and, in the image style, **Image protocol** (auto / kitty / sixel / off, asked again right away). The rest of the rows are **per style**: braille and image each keep their own values (braille `Osci<name>=` in `radio_config.txt`, image `OsciImage<name>=`), and the overlay shows and resets the set of the style in use (`R`). Image-only rows: **Glow** (size / strength of the bloom). Braille-only: **Dot threshold**.

`o` switches the scope block between the oscilloscope and the sphere (a rebindable action, REFERENCE tab: "Switch Osci / Sphere"; `+` is the volume key and stays that).

**Color** (per style): `gradient` = the OSCI colours of the COLORS tab from left to right (the default); `settings` = the same two colours but by beam speed (slow = left colour); `temperature` = deep red / orange / yellow-white / blue; `aurora`, `magma`, `ice`, `neon`, `spectrum` = fixed designs. All but `gradient` follow the beam speed (about the pitch), stretched over the range the signal really uses, so a pure tone and a busy mix both sweep through the palette. Config: `OsciPalette=` / `OsciImagePalette=` (the old `TemperatureColor=true` selects `temperature`).

**Frame rate**: the afterglow is scaled by the real frame time, so the look stays the same at any rate. The terminal is sent at most 60 pictures per second with Kitty (compressed), 20 without zlib, 15 with Sixel; at 90 fps the text still refreshes at 90 but the image every other frame. Config: `FrameRate=`.


## Notes of the latest revision
- The radio's own listening history files are `history_radio.txt` / `archive_radio.txt` (the player keeps `history.json`); old names are migrated once.
- REFERENCE tab: *Reset all keys* is the first line (under the cheat-sheet note); keys read `SHIFT+t`; the cheat sheet wraps descriptions at 120 columns and aligns them to the longest key.
- OSCI decay / dot threshold / tail move in steps of 0.01 (also in the image style); the preset menu no longer shows the selected preset in its bottom line.
- The image style's pictures are compressed with the vendored `third_party/miniz` (MIT), so no system zlib is needed.
