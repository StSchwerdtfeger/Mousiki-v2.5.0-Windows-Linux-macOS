<div align="center">
    
# 📻 Mousiki v3.0.0 - Music and Radio Player 🎵 
(Windows · Linux · macOS)  

<p align="center">
  <a href="https://opensource.org/" target="_blank">
    <img src="https://i0.wp.com/opensource.org/wp-content/uploads/2023/03/cropped-OSI-horizontal-large.png?fit=640%2C229&quality=80&ssl=1" alt="OSI" height="52" /></a>
&nbsp;
  <a href="https://www.apache.org/" target="_blank">
    <img src="https://www.apache.org/images/oakleaf.svg" alt="Apache" height="52" /></a>
</p>


> [!NOTE]
> **Developer note:** Mousiki is released under the Apache License 2.0.
> You are free to use, modify, fork, re-distribute, and sell the software,
> subject to the terms of the license.

[![License](https://img.shields.io/badge/License-Apache_2.0-blue.svg)](LICENSE)
[![Language](https://img.shields.io/badge/Language-C++17-orange.svg)](https://github.com/StSchwerdtfeger/Mousiki-Windows-Native-Port/tree/main)
[![Windows](https://img.shields.io/badge/Windows-x64-0078D4.svg?logo=windows&logoColor=white)](https://github.com/StSchwerdtfeger/Mousiki-Windows-Native-Port/tree/main) [![Linux](https://img.shields.io/badge/Linux-x64-FCC624.svg?logo=linux&logoColor=black)](https://github.com/StSchwerdtfeger/Mousiki-Windows-Native-Port/tree/main) [![macOS](https://img.shields.io/badge/macOS-x64-000000.svg?logo=apple&logoColor=white)](https://github.com/StSchwerdtfeger/Mousiki-Windows-Native-Port/tree/main)
</div>

Mousiki is a terminal music player that was originally built for macOS/Linux by the amazing [itzender5820/mousiki, v1.0](https://github.com/itzender5820/mousiki) for people who prefer control, simplicity, and a keyboard (mousi-**key**). This Mousiki fork lets it also run natively on Windows (including quite a bunch of modifications and additions; design maintained for obvious reasons). All credits for the design, main feature set, and the vast majority of the code goes to the original author. Since v2.5.0 the same code base builds natively on **Linux and macOS** again (see [Quick start (Linux / macOS)](#quick-start-linux--macos) or use [setup packages or portable version for all three platforms in latest release of v2.5.0 (OUTDATED)](https://github.com/StSchwerdtfeger/Mousiki-v2.5.0-Windows-Linux-macOS/releases/tag/v2.5.0)). Windows remains the primary and most tested platform (only tested Linux via WSL and works fine so far, a few bugs were recently fixed; not tested for macOS and non-WSL Linux systems). Feel free to give feedback in the discussions and report issues you might experience using this modified port. 

Build yourself (see [prerequisites](#prerequisites-windows) below) **or use the installer/portable (x64) version** that is included in the latest release (since v2.1.0). A full **[user manual](Mousiki_User_Manual/Mousiki_User_Guide.md)** (also as [PDF](Mousiki_User_Manual/Mousiki_User_Guide.pdf)) walks through every entry of the in-app cheat sheet, every settings tab and every overlay.

This fork exists because the original targets POSIX (Linux/macOS/Termux) and had no Windows build path at all. The Windows build uses no WSL, no MSYS runtime, no POSIX emulation layer, just a plain `mousiki.exe` built against the Win32 API and WASAPI. All Windows-specific code is guarded by `_WIN32`, so the very same sources also build on Linux (PulseAudio/PipeWire/ALSA) and macOS (CoreAudio) with the included `setup.sh`. Porting the original repository surfaced a long list of platform differences beyond the obvious ones (see [What had to change](#what-had-to-change-for-the-windows-port), below), plus a small number of pre-existing bugs in the original codebase that had nothing to do with Windows and got fixed along the way.

Along the Win32 port, **a lot of minor and major additions were made too**. The major ones are a e.g. **radio player mode**, **playlist menu** to create playlists from local (or downloaded) tracks, a **meta data editor** including **fetching artist/title via AcoustID** (audio fingerprinting done via chromaprint), a **listening history** (incl. the ability to add top tracks to the playback queue), an **XY mode ASCII-Braille / image oscilloscope** as an alternative to the lyrics ball and lyrics, a **10 band EQ with 12 presets** and the ability to create and save custom presets, enlarged **list/queue overlays**, **sleep timer**, a **user manual** (.md and .pdf)... Minor changes/additions are e.g. a general key to shuffle to a next title (before only the next title in the list was possible or switching to shuffle mode), stereo audio playback and loudness normalization, adding paths via the settings menu, toggling the lyrics on/off via a hotkey command, an optimized search engine for Windows (searching metadata was very slow and only available 2-3 min. after starting the app), fuzzy search (e.g. "X-Files" didn't show up when searching "X Files" without the dash), a categorized cheat sheet... As mentioned, the design remained the same for obvious reasons; the design asset added is a Braille-ASCII music cassette, shown when no track is loaded... See section [added features beyond the port](#added-features-beyond-the-port) for a full detailed list of modifications and added features.  

The latest addition is the mentioned **[radio player mode](#default-keybinding-radio-player)** (online radio with a tuning-dial look, presets, Radio Browser search, recording option and its own listening history) that is part of the very same program: press `SHIFT` and `+` (the `*` character) to switch between the music player and the radio; the radio is closed completely when you leave it, while the player only waits in the background (playback paused), so coming back to it is instant.

- **Original:** [github.com/itzender5820/mousiki](https://github.com/itzender5820/mousiki) — ender ([itzender5820](https://github.com/itzender5820))
- **License:** Apache 2.0 — see [LICENSE](LICENSE)
- **Windows port:** Steffen Schwerdtfeger ([StSchwerdtfeger](https://github.com/StSchwerdtfeger)), ported and adjusted with the help of AI tools (only free versions, mostly MiMo V2.6 and Sonnet 5 set on medium). Therefore take some of the below with a grain of salt, since I am not a developer for applications like this and I do not fully understand how the porting and the original C++ code actually works. Still took me >80h in the last weeks to perform porting to Win, modify (feature design) and debug this version... The repo code could also be optimized in that respect, but apparently is supposed to be done quite well (from feedback I got so far and evaluated myself, as far as I am capable to do so). Even though I am not that big fan of using AI for scientific applications (which I usually do), e.g. in the context of data science - since someone has to understand how sh** works and understanding is beautiful and mind blowing - I still liked this music player way too much the first time I saw it on social media to not want to use it on my Windows setup... Sooooo, I went this path and vibe coded a lot to create a port for Windows and still learned a lot as well (especially on UI and feature design).

*In general, a huge shout out for the great work by itzender5820 for this beautiful music player.* <3 It's the best and most fun music player I ever found. Makes me want to listen to music all the time :D 

<p align="center"><img width="800" alt="preview" src="preview.gif" /></p>

<p align="center"><img width="800" alt="preview" src="mousiki_xy_mode_osci.gif" /></p>

My current setup looks like the last .gif and the below below. The current default config.txt uses the below theme too. It is adjusted to fit my cyber-cat themed **MeowerShell** terminal setup [(see Gihub repository for config files)](https://github.com/StSchwerdtfeger/Meower-Shell), which includes FastFetch, Oh-My-Posh and loads of recommendations for lovers of the terminal, e.g. helpful apps like yazi, fzf fuzzy search ...

<p align="center"><img width="779" height="392" alt="grafik" src="https://github.com/user-attachments/assets/a16c6728-37e1-4124-86a4-591677656f00" /></p>

The new main UI of the radio mode looks like the below:

<p align="center"><img width="850" height="412" alt="image" src="https://github.com/user-attachments/assets/986ce2b5-45b6-4a5d-95e6-f45683e6c665" /></p>

## Current Status of the Port and Modification (v3.0.0)

For now the Mousiki port works well and also includes everything I at least wanted and made sense to me for a music player, so there might be no further major releases that add new features, except of bug-fixes that might appear to me or others in the future (feel free to start discussions or report issues!!). I might adjust the code to be more polished / robust and might optimize the setup release (Win version results currently in ~250MB size, setup itself ~80MB, portable .zip ~100MB)... Since v2.5.0 my version of Mousiki also builds on macOS/Linux again (`setup.sh`), which is a first step towards making it potentially integratable into the main branch of the original project (which still seems way too hard after dozens of comments in the last two weeks, at least from my perspective). The Linux/macOS builds are less tested than the Windows one, so reports from users on those platforms would help finalize it more. 
Concerning potential new features: further below you'll find a list of [current Ideas on features and modifications](#current-ideas-on-features-and-modifications). The online-radio function is now included (see [radio mode](#default-keybinding-radio-player)); a mixtape creator would be cool, but I'll see. Again, feel free to give feedback in the discussions, report issues you might experience using this modified port or contributing in any other way...

## Quick start (Windows)

Setup/portable (x64) version is included in the latest release (since v2.1.0 and since v2.5.0 for all platforms) or build yourself via the commands below. **On Linux or macOS?** Jump to [Quick start (Linux / macOS)](#quick-start-linux--macos).

However you install it, the app starts with either of two commands: **`mousiki`** or **`lala`**. The Windows installer puts both on the `PATH` (task *Add Mousiki to PATH*), the `.deb` installs `/usr/bin/mousiki` and `/usr/bin/lala`, the macOS `.pkg` `/usr/local/bin/mousiki` and `/usr/local/bin/lala`, the portable zips contain both launchers, and `setup.ps1` / `setup.sh` create both when you build yourself.

```powershell
# from the repo root
.\setup.ps1
```
> [!IMPORTANT]
> `setup.ps1` itself **requires PowerShell 7 or newer** (`#Requires -Version 7.0`); it will refuse to run in Windows PowerShell 5.1. The finished `mousiki.exe` was tested in PowerShell 7.6.6 *and* 5.1. If you only have 5.1, build manually (see below).

Optional parameters: `-SkipDeps` (configure and build only, don't touch winget/pip), `-BuildType Debug` (default is `Release`) and `-NoInstall` (see below).

After the build, `setup.ps1` creates the two commands **`mousiki`** and **`lala`** (both start the app) as small launchers in `%LOCALAPPDATA%\Mousiki\bin` and adds that folder to your user `PATH`; open a new terminal and type either one. `-NoInstall` skips this.

If PowerShell blocks the script, the following tFemporarily disables script blocking and warning prompts for the current PowerShell session only:

```powershell
Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
.\setup.ps1
```

The setup.ps1 already performs the build. If something didn't work and you want to build manually after debugging (this also works in PowerShell 5.1), use CMake directly:

```powershell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel
```

The build copies `scripts\` (including the freshly built `fpcalc.exe`) next to `mousiki.exe`. Keep that `scripts\` folder next to the exe if you move it, otherwise lyrics, fast online search and the AcoustID fetch stop working.

Run app e.g. via the command below (adjust username in the path before executing!!).
Note that you have to add your local files path via the config file `C:\Users\YOURNAME\.config\mousiki\config.txt` or via Settings (`s`) inside the app (more details further below). The `config.txt` in the repo root is a documented template of that file.
See the [user manual](Mousiki_User_Manual/Mousiki_User_Guide.md) or the original repo by itzender5820 for an introduction on how to use Mousiki.
```powershell
& 'C:\Users\YOURNAME\mousiki\build\Release\mousiki.exe'
```
Personally, I recommend writing a function in your PowerShell profile.ps1 in order to be able to run the app via a command (in my case I set the command to be "lala"):
To do so, open your profile file via:

```powershell
notepad $PROFILE
```

Then add the following (as said, I called the function lala but you can choose whatever you want; there is certainly a bunch of already existing commands, such as e.g. python, but you should be safe for most of the cases). 
Again, add your Username in the path!

```powershell
function lala {
    & 'C:\Users\YOURNAME\mousiki\build\Release\mousiki.exe'
}
```
Save your profile.ps1 via Ctrl + S and open a new terminal in order to be able to test your new function.
Voilà, you can now open Mousiki from any folder you're at using the command "lala", or whatever you set as command respectively...

## Prerequisites (Windows)

*(This section describes the Windows setup; for Linux/macOS see [Quick start (Linux / macOS)](#quick-start-linux--macos) above.)*

Note, I had a bunch of the below already installed, so I am not sure how smooth setup.ps1 runs installing the below for the first time using setup.ps1 (such as installing Visual Studio 2022 Build Tools...).

`setup.ps1` (PowerShell 7+) installs these via winget (and pip for `requests`), except the compiler "VS 2022 Build tools":

| Tool | Why | Install |
|---|---|---|
| Visual Studio 2022 Build Tools, "Desktop development with C++" | compiles the app (and the `fpcalc` helper) | `winget install --id Microsoft.VisualStudio.2022.BuildTools`, then tick the C++ workload interactively — winget's silent mode won't pick it. `setup.ps1` only warns if no compiler is found. |
| CMake ≥ 3.16 | build system | `winget install Kitware.CMake` |
| FFmpeg | decodes Opus (miniaudio can't), `ffprobe` supplies metadata, decodes audio for the AcoustID fingerprint and writes tags in the meta editor (`ffmpeg -c copy`) | `winget install Gyan.FFmpeg` |
| curl | radio mode only: the Radio Browser station search (`SHIFT+s` in the radio) | **nothing to install** — ships with Windows 10+ |
| yt-dlp | online search fallback, YouTube playlists (bulk add), streaming, downloads | `winget install yt-dlp.yt-dlp` |
| Python 3 | runs the helper scripts in `scripts/`: fast online search (`fast_yt_search.py`), AcoustID fetch (`fetch_meta.py`) and lyrics (`fetch_lyrics.py`) — the first two use the standard library only | `winget install Python.Python.3.12` |
| Python package `requests` | lyrics only (`fetch_lyrics.py` / `lrc.py`) | `py -3 -m pip install requests` (done by `setup.ps1`) |
| `fpcalc` (Chromaprint) | audio fingerprint for the AcoustID fetch | **nothing to install** — built by CMake from `third_party/chromaprint/` and copied to `scripts\` |
| `sidplayfp` (optional) | Commodore 64 SID tunes (`.sid`); tracker modules (`.mod/.xm/.it/.s3m`…) and console music (`.nsf/.spc/.vgm`…) only need an FFmpeg built with libopenmpt / libgme, which the usual Windows builds are | from a [sidplayfp release](https://github.com/libsidplayfp/sidplayfp/releases), put on the `PATH` |

All of the runtime tools (FFmpeg, yt-dlp, Python) are independent of each other and of the core player. With none of them installed, local playback of MP3/FLAC/WAV etc. still works (Opus/some other formats need FFmpeg); the radio mode cannot play anything without FFmpeg.  

MinGW-w64 (MSYS2 UCRT64) also builds this — configure with `-G "MinGW Makefiles"`. The code guards on `_WIN32`, not on `_MSC_VER`, except where MSVC genuinely differs (noted inline where it matters).

`third_party/` (miniaudio v0.11.25, kissfft, chromaprint 1.6.1 for the AcoustID meta data fetch, miniz 3.0.2 — MIT — which compresses the pictures of the oscilloscope's image style) is vendored in this repo, so configuring and building needs no internet connection — `CMakeLists.txt` no longer downloads anything, it just stops with a clear error if one of them is missing. miniaudio is public domain / MIT-0, kissfft is BSD-3-Clause, Chromaprint is MIT but — because it bundles some FFmpeg code — is to be treated as LGPL-2.1 as a whole (see `third_party/chromaprint/LICENSE.md` and the headers in `third_party/`). To update either, replace the files in `third_party/` with a newer upstream copy (not tested if it is that ease now that chromaprint is also included).

## Use Windows Terminal

The entire UI is ANSI escape sequences. `mousiki.exe` enables `ENABLE_VIRTUAL_TERMINAL_PROCESSING` at startup and exits with a clear message if that fails, rather than rendering garbage. Windows Terminal (`wt.exe`) works; the legacy conhost window on pre-1511 Windows builds does not.

## Where your files go (Windows)

`$HOME` doesn't exist on Windows, and the original codebase looks it up in seven different places to find its directories. Rather than rewrite all seven call sites to be platform-aware, this port points `HOME` at `%USERPROFILE%` for its own process at startup, so every one of those paths resolves exactly the way it does on Linux/macOS:

|| | Path |
|---|---|
| Config | `%USERPROFILE%\.config\mousiki\config.txt` |
| Cache (downloaded/streamed tracks) | `%USERPROFILE%\.cache\mousiki\` |
| Log | `%USERPROFILE%\.cache\mousiki\logs\console.log` |
| Session snapshot | `%USERPROFILE%\.cache\mousiki\snapshot\snapshot.json` |
| Listening history | `%USERPROFILE%\.cache\mousiki\history\history.json` (one folder, changeable: `HistoryPath=` or Settings → PATHS → HISTORY PATH) |
| Meta editor session (pending edits) | `%USERPROFILE%\.cache\mousiki\meta_session\session.json` |
| Playlists (default) | `<first LocalMusicPath>\playlists\`; falls back to `%USERPROFILE%\.cache\mousiki\playlists\` if no `LocalMusicPath` is set |
| Fetched lyrics | `<folder of the track>\lyrics\` |
| Radio settings | `%USERPROFILE%\.config\mousiki\radio_config.txt` (its own file; the radio never reads `config.txt`) |
| Radio stations, presets, station lists | `%USERPROFILE%\.config\mousiki\` — `stations.txt`, `presets.txt`, `stationlists.txt`, `preset_names.txt` (defaults, changeable in the radio's Settings → PATHS) |
| Radio listening history | `%USERPROFILE%\.config\mousiki\radio_history\` — `history_radio.txt`, `archive_radio.txt` |
| Radio recordings / downloads | the player's download folder by default, otherwise `<path>\radio_downloads\` |

`LocalMusicPath=`, `PlaylistsPath=` and `DownloadFolder=` entries accept Windows paths, both slash directions should work (`std::filesystem` normalizes them). A leading `~` is expanded to your user profile; **environment variables such as `%USERPROFILE%` are *not* expanded**, so use `~` or a full path:

```
LocalMusicPath=C:\Users\you\Music
LocalMusicPath=~/Music
```
Playlists folder (optional, **one** `PlaylistsPath=` line; further lines are ignored; changing it in the settings copies the existing playlists there). The listening history folder works the same way (`HistoryPath=`: an existing `history.json` in the new folder is used, otherwise the current one is copied). Without it, playlists live in `<first LocalMusicPath>\playlists`. Listing, loading, saving and deleting all use that single folder.
You can also add folders in the settings (`s`). Same goes for the (single) download folder (`DownloadFolder=`, default is `%USERPROFILE%\.cache\mousiki`), which is automatically added to the local music paths.

```
PlaylistsPath=C:\Users\YOUR NAME !!!!!!!\Music\playlists
DownloadFolder=D:\Downloads\mousiki
```

## Quick start (Linux / macOS)

Since v2.5.0 the code builds again on Linux and macOS as well (same sources, platform selected at compile time by `CMakeLists.txt`; audio via PulseAudio/PipeWire-pulse or ALSA on Linux and CoreAudio on macOS). Portable and setup packages can be found under latest release (included since v2.5.0). Windows remains the primary, most tested platform. `setup.sh` is the counterpart of `setup.ps1`:

```bash
# from the repo root
chmod +x setup.sh   # only needed once, if the executable bit got lost (e.g. after unzipping)
./setup.sh
```

It detects your package manager (apt, dnf, pacman, zypper, apk, or Homebrew on macOS), installs only what is missing (see the table below), then configures and builds with CMake. The binary ends up in `build/mousiki`, with `scripts/` (including the freshly built `fpcalc`) copied next to it. Finally it creates the two commands `mousiki` and `lala` in `~/.local/bin` (both start the app; skip with `--no-install`).

Options:

| Option | Effect |
|---|---|
| `--skip-deps` | configure and build only, don't touch the package manager / pip |
| `--debug`, `--build-type Release\|Debug` | build type (default `Release`) |
| `--no-install` | don't create the launchers. By default `setup.sh` creates `~/.local/bin/mousiki` and `~/.local/bin/lala`, so you can start the app from anywhere by typing `mousiki` or `lala` (`--install` is still accepted and does the same as the default) |
| `-y`, `--yes` | don't ask before installing packages |
| `-h`, `--help` | show the options |

> [!NOTE]
> Package installation uses `sudo` on Linux when you are not root. On macOS the script needs [Homebrew](https://brew.sh) and the Xcode Command Line Tools (`xcode-select --install`, they provide the C++ compiler); if one is missing, the script tells you and stops. The launcher from `--install` is a small wrapper script on purpose and not a symlink: macOS does not resolve symlinks when the binary looks up its `scripts/` folder.

Run it:

```bash
./build/mousiki
```

The config file is created at `~/.config/mousiki/config.txt` (the `config.txt` in the repo root is a documented template; Linux/macOS paths such as `LocalMusicPath=~/Music` work, the Windows examples in it do not apply). Any UTF-8 terminal works.

| Tool | Why | Installed by `setup.sh` as |
|---|---|---|
| C++17 compiler, CMake ≥ 3.16 | builds the app and the `fpcalc` helper | `build-essential` / `gcc-c++` / `base-devel` / `build-base` + `cmake`; Xcode CLT on macOS |
| curl | radio mode: Radio Browser station search | `curl` (macOS has it built in) |
| ALSA + PulseAudio libraries (Linux) | audio output (miniaudio loads them at runtime; PipeWire works through `pipewire-pulse` / `pipewire-alsa`) | `libasound2-dev libpulse-dev` (apt), `alsa-lib-devel pulseaudio-libs-devel` (dnf), … |
| FFmpeg (incl. `ffprobe`) | Opus decoding, metadata, AcoustID decoding, tag writing; radio streams (needed) and MP3 recordings (`libmp3lame`; `setup.sh` warns if it is missing) | `ffmpeg` (on Fedora from RPM Fusion; `setup.sh` falls back to `ffmpeg-free` and prints a hint) |
| yt-dlp | online search fallback, playlists, streaming, downloads | `yt-dlp` from the package manager, otherwise `pip install --user yt-dlp` (then `~/.local/bin` has to be on your `PATH`) |
| Python 3 + `requests` | lyrics, fast online search, AcoustID fetch (`requests` is only needed for lyrics) | `python3`, `python3-requests` (or `pip install --user requests`) |
| `xclip` / `wl-clipboard` (Linux, optional) | pasting into the search field; macOS uses the built-in `pbpaste` | `xclip` on X11, `wl-clipboard` on Wayland |
| `fpcalc` (Chromaprint) | audio fingerprint for the AcoustID fetch | **nothing to install** — built by CMake from `third_party/chromaprint/` and copied to `scripts\` |
| `sidplayfp` (optional) | Commodore 64 SID tunes (`.sid`); tracker modules and console music go through FFmpeg (libopenmpt / libgme, included in the distro builds) | `sidplayfp` (apt/dnf/pacman), `brew install sidplayfp` |

Just like on Windows, FFmpeg, yt-dlp and Python are independent of each other and of the core player: without them, local playback of MP3/FLAC/WAV etc. still works (the radio mode needs FFmpeg). The radio is built into the same binary (sources in `src_radio/`), so there is nothing extra to build or run; like the player it has only been tested on Linux so far. If you prefer to install everything yourself, build manually:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

Keep the `scripts/` folder next to the binary if you move it, otherwise lyrics, fast online search and the AcoustID fetch stop working.

## Where your files go (Linux/macOS)

| | Path |
|---|---|
| Config | `~/.config/mousiki/config.txt` |
| Cache (downloaded/streamed tracks) | `~/.cache/mousiki/` |
| Log | `~/.cache/mousiki/logs/console.log` |
| Session snapshot | `~/.cache/mousiki/snapshot/snapshot.json` |
| Listening history | `~/.cache/mousiki/history/history.json` (one folder, changeable: `HistoryPath=` or Settings → PATHS → HISTORY PATH) |
| Meta editor session (pending edits) | `~/.cache/mousiki/meta_session/session.json` |
| Playlists (default) | `<first LocalMusicPath>/playlists/`; falls back to `~/.cache/mousiki/playlists/` if no `LocalMusicPath` is set |
| Fetched lyrics | `<folder of the track>/lyrics/` |
| Radio settings | `~/.config/mousiki/radio_config.txt` (its own file; the radio never reads `config.txt`) |
| Radio stations, presets, station lists | `~/.config/mousiki/` — `stations.txt`, `presets.txt`, `stationlists.txt`, `preset_names.txt` (defaults, changeable in the radio's Settings → PATHS) |
| Radio listening history | `~/.config/mousiki/radio_history/` — `history_radio.txt`, `archive_radio.txt` |
| Radio recordings / downloads | the player's download folder by default, otherwise `<path>/radio_downloads/` (`~/Music/radio_downloads` if the toggle is off and the path empty) |

## Default Keybindings (Music Player)

IN GENERAL: See manual for a full detailed overview of the functionality of the current version of Mousiki. 

Rebindable in `C:\Users\USER\.config\mousiki\config.txt` or in Settings → Reference (`s`). Keys written as `SHIFT+x` are the **uppercase letter** (e.g. `HKeyCycleSortMode="T"`), because the plain lowercase letter already does something else. Some commands are fixed (not rebindable): `ESC`, `Y`/`N` in prompts, `SHIFT+B`, `SHIFT+↑/↓` in the overlays, and the keys inside the playlist and meta editors including `CTRL+S` (save) and `CTRL+SHIFT+X` (discard).
See the **[user manual](Mousiki_User_Manual/Mousiki_User_Guide.md)** for every command in detail or use `?` for the cheat sheet inside the app (it always shows the keys you actually have bound).

Note that macOS uses the Option key or Ctrl as substitute for ALT! When installing on macOS all key command legends and the cheat sheet should be adjusted accordingly. 

### System
| Action | Keybinding | Description |
| :--- | :--- | :--- |
| **Switch Player ↔ Radio** | `SHIFT + +` | Switch to the radio mode (and back from there). It types the `*` character, so on a German keyboard it is `SHIFT` and `+`; the cheat sheet shows the combination that fits your keyboard layout (detected automatically, `MOUSIKI_KEYBOARD=us\|de\|fr\|…` overrides it). The player is paused and waits in the background, the radio is closed completely when you leave it. Fixed key, not rebindable; `SHIFT + m` stays the meta data editor. `+` on its own is volume up |
| **Cheat Sheet** | `?` | List of all key commands (on German keyboards it is `SHIFT + ß`) |
| **Settings** | `s` | Open Settings; inside Settings `s` again saves to `config.txt` and returns |
| **Exit Settings** | `ESC` / `q` | Discard & close: changes made on the screen are reverted; while there are unsaved changes a green ``Unsaved changes! Save with `s` or discard with `ESC`.`` note is shown. While a field (colour, path, value, key) is being edited, `←/→` move the caret and `TAB` stays in the field; only `ENTER` (apply) or `ESC` (cancel) leave it |
| **Console / Logs** | `t` | Show console logs for debugging |
| **Quit** | `q` | Quit (from the main UI) |
| **Rescan library** | `SHIFT + r` | Rescan the music folders (e.g. after new files were copied or downloaded); also works in the meta data editor while the library pane or the fetch list has the focus. Fixed key |
| **Home view** | `ESC` | On the main screen: back to the full local library, no search/folder filter, scrolled to top (sort mode stays) |
| **Confirm prompt** | `Y` / `N` | Answer Yes/No prompts (clear queue, AcoustID disclaimer, ...) |

### Playback
| Action | Keybinding | Description |
| :--- | :--- | :--- |
| **Play / reload selected track** | `Enter` | Play or reload selected track (on a playlist row: queue all of its tracks) |
| **Play / Pause** | `p` | Toggle playback |
| **Next / Previous Track** | `n` / `b` | Skip between songs (`n` takes the queue first) |
| **Shuffle Next** | `#` | Jump to a random track of the current list (ignores the queue) |
| **Change Play-/Cycle mode** | `m` | list → repeat → shuffle → stop → queue then stop (shown as `L` `R` `S` `O` `Q` next to the search bar) |
| **Seek** | `ARROW_LEFT` / `ARROW_RIGHT` | Seek backward / forward (5 s) |
| **Volume** | `+` / `-` | Increase / Decrease in-app volume (steps of 5); the same keys as in the radio (until v3.0.0: `1` / `2`) |
| **Mute** | `x` | Set volume to 0 without pausing; press again to restore |
| **Normalize** | `v` | Toggle loudness normalization, edit in overlay menu, use `SHIFT + v` |

### Navigation & View
| Action | Keybinding | Description |
| :--- | :--- | :--- |
| **Navigate** | `ARROW_UP` / `ARROW_DOWN` | Move selection |
| **Switch Tabs/Cards** | `TAB` | Switch focus between list and queue |
| **Filter by Folder** | `f` | Show only the folder of the hovering track |
| **Clear Filter** | `c` | Reset the folder filter |
| **Cycle Sort Mode** | `SHIFT + t` | folder order → title A-Z → artist A-Z |
| **Filename / Meta Data** | `SHIFT + n` | Toggle between filename + meta data and meta data only in lists |
| **Cycle Lyrics View** | `.` | Cycle the lyrics area: lyrics → sphere → oscilloscope (until v3.0.0: `+`) |
| **Retry Lyrics** | `l` | Form to fetch lyrics again with a manual title/artist (only while the lyrics engine is on) |
| **Waveform style** | `w` | Toggle waveform raw / smooth |
| **Refresh UI** | `r` | Force a full redraw (e.g. after a terminal resize); until v3.0.0 this was `k` |
| **Karaoke overlay** | `k` | The lyrics of the playing track over the whole screen, centred between two karaoke pictures in the disk colours with a moving colour wave; the sung words highlighted; `SHIFT` and `+` / `SHIFT` and `-` change the lyrics size 1-5 (big sizes drawn with a built-in pixel font); the layout follows terminal resizes; playback keys keep working, `ESC` or `k` closes |
| **Track list overlay** | `SHIFT + l` | Enlarged overlay of the track list pane; `SHIFT + ↑/↓` pages, `ESC` closes |
| **Queue list overlay** | `SHIFT + k` | Enlarged overlay of the queue pane (same paging, `ESC` closes) |
| **Oscilloscope tuning** | `SHIFT + o` | Overlay to tune the oscilloscope live: display (sphere / osci), style (braille / image), frame rate, decay, dot threshold, tail, line interpolation, Z axis, trace length, 45° rotation, mono phase portrait, colour palette, glow (image style) and image protocol. Each style keeps its own values (`R` resets the style in use, saved on close) |
| **Normalization tuning** | `SHIFT + v` | Overlay to adjust the loudness normalization parameters |
| **Lyrics timing** | `ALT + l` | Overlay to shift the lyrics of the playing track earlier / later (`←`/`→` ±0.1 s, `↑`/`↓` ±0.5 s, `R` reset, `ENTER` saves the offset into the track's `.lrc`, `ESC` cancels). |
| **Equalizer** | `SHIFT + e` | Open Equalizer overlay menu, includes a 10 band EQ with 13 presets. | 
| **Sleep timer** | `SHIFT + z` | Small overlay: pause playback after 15 / 30 / 60 / 90 / 120 minutes, or stop after the current song (or switch it off). **Fade out** (on/off, `SleepFade` in config.txt) lowers the volume over the last 10 % of the time (30 s to 10 min). Independent of the Stop play mode |

### Search
| Action | Keybinding | Description |
| :--- | :--- | :--- |
| **Local Search** | `/` | Filter and search local library (fuzzy, live as you type) |
| **Online Stream Search** | `/s: <query>` | Search and stream music online (runs on `Enter`) |
| **Search Playlists** | `/p: <query>` | Search saved playlists; `Enter` on a playlist queues all of its tracks |
| **Search Folders by Name** | `/f: <query>` | Search folder; hit `Enter` to open content in local audio pane (similar using `f`) |

### Queue
| Action | Keybinding | Description |
| :--- | :--- | :--- |
| **Add to Queue (next)** | `a` | Enqueue selected track as the *next* one (several `a` presses keep their order); with the queue focused: bulk-add panel for a pasted YouTube playlist link |
| **Add to End of Queue** | `e` | Enqueue selected track at the *end* of the queue |
| **Lock Queue** | `!` | Toggle "locked" (default: on): a played track moves to the end of the queue so it loops; unlocked, a played track leaves the queue (shown as `locked` in the queue title) |
| **Remove from Queue** | `d` | Dequeue selected track |
| **Move Track Up** | `4` | Move up in Queue/Playlist |
| **Move Track down** | `5` | Move down in Queue/Playlist |
| **Move to Top / Bottom** | `SHIFT + 4` / `SHIFT + 5` | Move the hovering queue item to the very top / bottom |
| **Clear Queue** | `SHIFT + x` | Clear Queue (asks first) |
| **Undo Clear Queue** | `CTRL + SHIFT + z` | Bring back the queue that was cleared last (in front of anything queued since) |
| **Queue to Playlist** | `CTRL + SHIFT + u` | Open the playlist editor with the queue's local tracks, name field focused (type a name, `CTRL+S` saves and stays in the menu) |

### Playlists, Meta Data, History, Downloads
| Action | Keybinding | Description |
| :--- | :--- | :--- |
| **Open Playlist Editor** | `SHIFT + p` | Open Playlist Creator/Editor (see the editor keys below) |
| **Open Meta Data Editor** | `SHIFT + m` | Open Meta Data Editor (see the editor keys below) |
| **Fetch meta data (AcoustID)** | `SHIFT + b` | Fingerprint lookup of the hovered local title (fixed key, also works from the main list) |
| **Listening History** | `SHIFT + h` | Open the history overlay; `←/→` or `1` / `2` / `3` switch tabs (`TAB` never does), `r` re-sorts; on Top Tracks `TAB` moves into **ADD SMART HISTORY TO QUEUE**: 16 lists (top 10/25/50/100, top 25 of the week / month / quarter / year, top 25 by time of day, last 25 newly added, least played), arrows pick, `Enter` queues, `ESC` leaves the pane |
| **Download Stream** | `y` | Download currently streaming track to the download folder (Settings → Download Folder, default `.cache\mousiki`) |

Keys inside the **playlist editor**: `e` on the Saved Playlists tab exports the selected playlist as M3U8 (default) or M3U (overlay with folder and format; the default folder is Settings → PATHS → PLAYLIST EXPORT PATH), `←/→` switch tab (in a text box only until you type something there, then they move the caret; `ESC` leaves the box again), `ALT+←/→` switch tab from anywhere, `TAB` cycle focus, `Enter` add track / load playlist, `4`/`5` move track, `D`/`DEL`/`BACKSPACE` remove track (delete playlist on the Saved tab), `CTRL+S` save playlist, `SHIFT+←/→` mark text, `CTRL+C/X/V` copy/cut/paste, `ESC` leave the text box / close (asks to save unsaved changes).

Keys inside the **meta data editor**: `←/→` switch EDIT / FETCH LIST tab (in the search box only until you type something there; in the field editor they always move the caret), `ALT+←/→` switch tab from anywhere, `ESC` leave the search box / field editor, then close, `TAB` cycle focus, `Enter` edit field (on FETCH LIST: run the whole batch), `a` add file to the fetch list, `r` edited files on top, `x` / `SHIFT+T` / `SHIFT+A` / `SHIFT+Y` filter for missing any / title / artist / year, `DEL`/`d` remove from the fetch list, `CTRL+S` apply edits to the files, `CTRL+SHIFT+X` discard edits.

Every editable/rebindable hotkey can also be found in the config.txt and — for an overview — in the in-app cheat sheet overview (`?`). Not every key was set to be rebindable. Rebinding can also be done in the config.txt file and under Settings → Reference in the app itself (a key already used by another action is refused).

Changing Fonts: In the config.txt you will also see lines like `A={A,a}`. This is used so you can re-font the UI without changing the font of the terminal, e.g. via `A={𝓐,𝓪}` (edit while the app is closed). 

## Playlist and Meta Data UI Behavior

The playlist menu, the meta data menu and the radio's station lists menu share one rule for their tabs and text boxes: `LEFT/RIGHT` switch the tab, also from a text box (playlist name, search) as long as nothing was typed into it. Once you type or edit something there, `LEFT/RIGHT` move the caret inside the box (the legend switches from `[←→] Switch Tab` to `[←→] Cursor`) until `ESC`, `TAB` or `ENTER` leaves the box again. Only an `ESC` outside a text box closes the menu, and with unsaved changes (e.g. a typed playlist name) it first asks `Save changes to "…" before exiting?`. `ALT+LEFT/RIGHT` switch tabs from everywhere, and `CTRL+S` saves in all three menus. The playlist UI starts in the Name field, where a name for the playlist can be chosen; the pane focus is changed via TAB. 
In the meta data menu the field editor (entered with `ENTER`) always uses `LEFT/RIGHT` for the caret; `ESC` takes you back to the library. I guess the handling of the meta data menu needs some practice, since it is rather complex task to perform, however after fitting in it works well. To sort the list (e.g. all edited titles on top, or show only titles with no meta data, or not title or no artist) you have to set the focus on the library pane and then use `x` / `SHIFT+T` / `SHIFT+A` / `SHIFT+Y` for all missing, title missing, artist missing, year missing (press again to clear). Using metatogger can be a bit faster, however it is not the Mousi-**key** way of doing things! Gotta love the terminal. 

## Default Keybinding (radio player)
### Keys (main ui screen )

All of them can also be found in the in-app cheat sheet (`?`, categorized, scrollable). The keys that are not hard coded can be rebound in Settings → REFERENCE.

| Action | Keybinding |
| :--- | :--- |
| Move in the station list | `ARROW_UP` / `ARROW_DOWN` (`j` / `k` only inside the menus) |
| Tune the hovered station | `Enter` |
| Next / previous channel | `n` / `b` (`b` follows the mode: `S` the channel played before, `L` the one before it in the list) |
| Shuffle: random channel | `#` |
| Mode `S` shuffle ↔ `L` list | `m` (the box next to SEARCH; it only affects `b`) |
| Recall slot 1-16 of the active preset | `1 2 3 4 5 6 7 8 9 0 e r t d f g` |
| Previous / next preset | `SHIFT + ←` / `SHIFT + →` |
| Search | `/` (`Enter` tunes the hovered hit, `ESC` clears) |
| Mute / unmute | `p` (with no channel loaded: tunes the hovered station like `Enter`) |
| Volume | `+` / `-` |
| Reconnect | `R` (capital, because `r` is a preset key) |
| Stop | `x` |
| Sort stations (list order ↔ name A-Z) | `SHIFT + t` |
| Switch scope block oscilloscope ↔ sphere | `.` (rebindable; same key as the lyrics-area cycle in the player; was `o` up to v3.0.0) |
| Loudness normalization / its overlay | `v` / `SHIFT + v` |
| Equalizer overlay | `SHIFT + e` (the preset key `e` is the lowercase letter, so there is no conflict) |
| Oscilloscope overlay | `SHIFT + o` |
| Sleep timer | `SHIFT + z` |
| Pause / resume (timeshift; the stream keeps buffering) | `SPACE` |
| Jump back / forward 30 s | `[` / `]` (shown for 4 s at the bottom of the scope block) |
| Jump back 5 min / back to live | `{` / `}` |
| Record (RECORD overlay: from now on, the last 1/5/10/15/30 min or everything buffered; `y` again stops) | `y` |
| PRESETS menu / RADIO BROWSER / STATION LISTS | `SHIFT + k` / `SHIFT + s` / `SHIFT + p` |
| Listening history | `h` |
| Big STATIONS overlay | `SHIFT + l` (shown as `L`) |
| Settings | `s` |
| Cheat sheet | `?` |
| Switch to the music player | `SHIFT + +` (the `*` character) |
| Quit | `q` or `CTRL + c` |

### Text fields

The main SEARCH box, the menus' search boxes and the name overlays edit text like the player's fields: `←/→` move the caret, `SHIFT+←/→` mark text, `HOME/END` jump to the ends, `BACKSPACE` / `DEL` delete the mark or one character, `CTRL+C` copies the mark (nothing marked: the whole field), `CTRL+X` cuts it, `CTRL+V` pastes (replacing the mark). While a text field has the keyboard `CTRL+C` copies instead of quitting and `SHIFT+←/→` marks text instead of switching the preset.

### Search

All searches (stations, station lists, presets, the STATION LISTS menu) are typo tolerant like the player's: an exact substring ranks first (earlier hit first), then close fuzzy matches; `-` `_` `.` `/` count as spaces. With a query the best match comes first; `SHIFT+t` (A-Z) overrides that order. The Radio Browser menu searches on the server and is not fuzzy. Lists scroll like the player's: the window only moves when the cursor leaves it; the hovered row scrolls as a marquee when its name is too long.

Type `p:` into the empty search box (`/` first; `/p:` works too) to search the saved station lists: the STATIONS pane turns into STATION LISTS and filters live. `Enter` fills the STATIONS pane with that list's stations **and tunes its first station**; while a list is shown the pane title reads `STATIONS (LIST: name - ESC: all)`. `ESC` clears a search that is still in the box, the next `ESC` returns to all stations. `s:` in the empty box goes back to searching stations.

### PRESETS menu (`SHIFT+k`)

A **preset** is a named set of 16 station slots (keys `1234567890ertdfg`). You can keep as many as you like; the active one is shown in the title of the PRESETS pane (`PRESETS (Morning)`) and `SHIFT+←/→` switches to the previous / next one from the main screen. The full-screen menu is laid out like the player's playlist menu, top to bottom: **SEARCH STATION / SEARCH PRESET** (`s:` filters stations, `p:` presets), **SELECT PRESET** (4 columns x 2 rows of names) and **STATIONS** beside **PRESETS** (the 16 slots of the active preset). A small `◀` behind a pane's title shows which pane has the keyboard.

| key | where | does |
|---|---|---|
| `TAB` | anywhere | next pane (SEARCH → SELECT PRESET → STATIONS) |
| `ARROWS` (`j/k/h/l`) | SELECT PRESET / STATIONS | move |
| `Enter` | STATIONS | tune the hovered station |
| `Enter` | SELECT PRESET | open the hovered preset **and tune its first filled slot** |
| `Enter` | search | tune the hovered station / open the hovered preset, then jump to that pane |
| `SHIFT+n` / `SHIFT+c` | SELECT PRESET | new preset / rename the hovered one (small name overlay, `Enter` applies, `ESC` cancels) |
| `1`..`0`, `e r t d f g` | STATIONS | **set the hovered station as that slot of the active preset** (same key again clears it; a station sits in one slot only, so it moves) |
| `DEL` / `BACKSPACE` | STATIONS | remove the hovered station from the active preset |
| `/` | STATIONS / SELECT PRESET | back to the search box |
| `ESC` | search | clear the search; if it is already empty, close the menu |
| `ESC` | STATIONS / SELECT PRESET | close the menu |

### RADIO BROWSER menu (`SHIFT+s`)

Searches the public station directory <https://www.radio-browser.info/> ([API docs](https://docs.radio-browser.info/)). Six input panes on top, **RESULTS** and **STATION INFO** (all details of the hovered result) side by side below:

| pane | what you type | sent as |
|---|---|---|
| NAME | part of the station name | `name` |
| TAGS | comma separated, **every** tag has to match: `rock, 80s` | `tagList` |
| COUNTRY | a name (`Germany`) or a 2-letter code (`DE`) | `country` / `countrycode` |
| STATE / REGION | part of the region name | `state` |
| LANGUAGE | as Radio Browser spells it: `german` | `language` |
| BITRATE | kbps: `128` = at least 128, `64-192` = range, `-192` = at most | `bitrateMin` / `bitrateMax` |

Every search also sends `hidebroken=true`, `order=clickcount&reverse=true` (most clicked first) and `limit=200`; empty panes are left out.

| key | does |
|---|---|
| `TAB` | NAME → TAGS → COUNTRY → STATE → LANGUAGE → BITRATE → RESULTS → NAME |
| `Enter` | in a pane: search (the focus jumps to RESULTS when the results arrive); in RESULTS: tune the hovered station |
| `ARROW_UP` / `ARROW_DOWN` | in a pane: one pane row up / down (down from the last row: RESULTS); in RESULTS: move the cursor |
| `a` | in RESULTS: add the hovered station to your station list (appended, so presets keep their slots; written to `stations.txt`) |
| `/` | in RESULTS: back to NAME |
| `ESC` | in a pane: clear it; empty pane or RESULTS: close the menu |
| `?` / `CTRL+c` | cheat sheet (RESULTS only) / quit (in a pane `CTRL+c` copies) |

A dot in front of a result means its stream is already in your station list; results whose last check failed are dimmed (and hidden by default). A result is lit as tuned whatever way the station was tuned. Streams are compared ignoring `http`/`https`, host case, a trailing `/` and tracking parameters (`?aggregator=web`, `utm_*`, …). Tuning counts one click at Radio Browser, as the API asks. The line under the results shows what is tuned in right now (with the track title when the stream sends one). Needs `curl` on the `PATH`. Servers tried in order: `de1`, `all`, `nl1`, `at1` `.api.radio-browser.info`; the one that answered last goes first; `MOUSIKI_RADIO_API=<base url>` replaces the list (own mirror, tests). If `a` finds no `stations.txt` yet it creates one from the built-in list plus the new station.

### STATION LISTS menu (`SHIFT+p`)

Named, ordered collections of stations, built and managed like the player's playlist editor. Two tabs, switched with `←/→` like the player's playlist editor (in the name field and the search boxes only until you type something there, then the arrows move the caret until `ESC` / `TAB` / `Enter`), or with `ALT+←/→` (`Option` on macOS) from every pane; the top pane uses the settings-style tab strip.

**Tab 1 - CREATE / EDIT.** The top pane's single row holds the list's `Name:` field (25 characters at most, a 25-column field that turns red while you type). Below it SEARCH ALL STATIONS (name, genre, country), then STATIONS (all stations, or the search results) beside LIST CONTENTS (what you are building). **Tab 2 - SAVED STATION LISTS.** The tab strip, SEARCH STATION LISTS and the results; `Enter` on a list moves its content into the editor on tab 1.

| key | where | does |
|---|---|---|
| `←/→` | lists, and text boxes not typed in yet | switch tab |
| `ALT+←/→` | anywhere | switch tab |
| `TAB` | anywhere | tab 1: name → search → STATIONS → LIST CONTENTS; tab 2: search ↔ list |
| `Enter` | name | on to the search |
| `Enter` | search, STATIONS | add the hovered station to the end of the list (a station is in a list once) |
| `Enter` | LIST CONTENTS | tune the hovered station (to audition the list) |
| `SHIFT+t` | STATIONS | sort: list order ↔ name A-Z; also sorts search results |
| `4` / `5`, `DEL` / `BACKSPACE` / `d` | LIST CONTENTS | move the hovered station up / down, remove it |
| `CTRL+S` | every pane of tab 1 | save under the typed name, stay in the menu (same name overwrites; empty name: "enter a name first") |
| `Enter` | tab 2 search / list | to the list / load the hovered list into tab 1 |
| `DEL` | tab 2 list | delete the hovered list after a Yes/No prompt |
| `/` | STATIONS, LIST CONTENTS, tab 2 list | back to the search box |
| `ESC` | a text box you typed in | leave the box (the text stays; `←/→` switch tabs again) |
| `ESC` | search with text | clear it |
| `ESC` | otherwise | close; with unsaved changes on tab 1 it asks `Save changes to "x" before exiting? [Y]es [N]o [ESC] cancel` |

`CTRL+S` saves from every pane, also while typing. Like the player, opening the menu starts a fresh list; an existing one comes back only through tab 2 → `Enter`. A dot behind a station in STATIONS means it is already in the list being built. Lists are saved to `stationlists.txt`:
```
[Morning drive]
https://stream.radioparadise.com/aac-128 | Radio Paradise
```
Stations are matched by URL, then by name; one that has been removed from `stations.txt` is dropped when the list is read. While a list is shown in the main pane its rows are numbered by position in the list.

### Big STATIONS overlay (`SHIFT+l`)

The STATIONS pane with the whole screen for its rows: same search box and state as the main screen (`/`, `p:`, `s:`, `SHIFT+t`, `ESC` clears the search, leaves a list, then closes), plus a PRESET NAME column. `Enter` tunes the hovered station; `n` `b` `#` `p` `x` `+` `-` work as in the main screen. Keys that would open another menu are ignored while it is open.

- `a` adds a station by its stream URL (`TAB` switches URL ↔ NAME; an empty NAME becomes the host name). It is appended to the end of the list (so the preset slots stay valid) and to `stations.txt`; a stream that is already in the list is refused.
- `SHIFT+c` gives the hovered station a **preset name**: a shorter second name that is shown in the PRESETS pane instead of the station name (empty = the station's own name again). Stored in `preset_names.txt` (`url<TAB>name`) next to `stations.txt`; the search also looks at it. The tuned entry of the PRESETS pane scrolls as a marquee when its name does not fit.

### Listening history (`h`)

One fused pane with the settings-style tab strip; the bottom border carries the info (`14 lines (max 10000)`, or the sort direction). Three tabs (`1 2 3`, `←/→`, `TAB`, `ARROW_UP/DOWN`, `HOME/END`, `ESC` / `q` / `h` close):

- **HISTORY** — WHEN / CHANNEL / ARTIST / TITLE / HEARD. A new line starts whenever the channel, the artist or the title changes (lines heard for less than 3 s are dropped). Hover a line and press `y`: a small overlay searches YouTube for "artist title" (the player's own online search), `Enter` downloads the hovered result (yt-dlp, opus) into the download folder; `[..]` downloading, `[ok]` done, `[!!]` failed. `TAB` edits the query, `ESC` closes the overlay.
- **TOP CHANNELS** — channels by time listened (`r` flips most / least first).
- **HABITS** — sessions (a 30 min gap starts a new one), time per day, channels, listening by hour of the day and by weekday.

Light-weight like the player's: the newest 10 000 lines are kept in memory and in `history_radio.txt`; beyond that the oldest line is folded into `archive_radio.txt` (per-channel totals, per-day / hour / weekday seconds), so TOP CHANNELS and HABITS stay lifetime figures. (Old file names without `_radio` are migrated once.)

## What had to change for the Windows port

Around thirteen files needed direct `#ifdef _WIN32` branches; a similar number needed changes that apply on every platform but were only ever exposed by something Windows does differently (mostly the UTF-8 path handling below). Everything else compiled and ran unmodified. Due to the fast modifications, it became to hard to track what has changed compared the original version. Since v2.5.0 this adjusted code also builds on macOS/Linux again, additions included (`setup.sh`), so that they can eventually be added to the original branch of this fork. The Linux/macOS side has seen less testing than Windows so far; feedback from other users is welcome.

### Console & terminal I/O

POSIX raw mode (`termios`), key polling (`read()` off `STDIN_FILENO`), window size (`ioctl(TIOCGWINSZ)`), and even `wcwidth()` for East-Asian character width have no Windows equivalent. All of it is replaced by Win32 Console API calls behind a small platform shim (`win_compat.h`/`.cpp`, new in this fork) — `SetConsoleMode`, `_kbhit`/`_getch` (with its own extended-key scan codes translated to match the POSIX escape-sequence path, so the rest of the app never has to know which platform it's on), `GetConsoleScreenBufferInfo`, and a hardcoded East-Asian-width table for `wcwidth`, since MSVC's CRT doesn't ship one at all.

Rendering initially used `SetConsoleOutputCP(CP_UTF8)`, which turned out to be unreliable across console hosts — box-drawing and other multi-byte glyphs could still come out as mojibake depending on the terminal. `win_compat.cpp` now converts to UTF-16 and writes through `WriteConsoleW` directly, which removes the ambiguity.

### Filenames, paths, and non-ASCII text

This was the deepest rabbit hole, and the one most likely to still bite on an untested edge case. MSVC's `std::filesystem::path::string()` converts through the process's **ANSI code page** — and a character with no mapping in that code page (which, for the vast majority of Windows installs, includes almost anything outside Western European Latin script) doesn't get substituted, it throws `std::system_error`. A library with any Japanese, Korean, Cyrillic, or similar filenames would crash outright the moment such a file scrolled into view, with no way to work around it from inside the app.

The fix is a small header, `path_utf8.h` (new), providing `path_utf8()` / `path_from_utf8()` as the only sanctioned way to convert between `fs::path` and the UTF-8 `std::string`s the rest of the codebase already speaks — UTF-16⇄UTF-8 conversion on Windows, a plain passthrough everywhere else. Every `.string()` call and every `fs::path(some_std_string)` construction across the tree (about twenty call sites, in `app.cpp`, `local_source.cpp`, `metadata_probe.cpp`, `settings.cpp`, `cache_manager.cpp`, `youtube_source.cpp`, `lyrics_fetcher.cpp`, `native_duration.cpp`, `console_log.cpp`) was audited and routed through it. A handful of related fixes came out of the same pass:

- `waveform.cpp`'s use of `miniaudio`'s narrow file-open API silently failed (and fell through to a much slower `ffmpeg` decode) for any non-ASCII path — switched to `ma_decoder_init_file_w` on Windows.
- Case-insensitive matching (search, extension checks) used to fold text byte-by-byte with `std::tolower`, which corrupts multi-byte UTF-8 sequences under a single-byte locale. Replaced with ASCII-only folding that leaves every non-ASCII byte untouched.
- The cache filename sanitizer kept only `[A-Za-z0-9]`, so any track with no Latin characters in its title collapsed to a generic `untitled` filename, and every such track collided on the same name. Non-ASCII codepoints are now kept verbatim (they're legal in NTFS filenames; the characters Windows actually forbids are all ASCII, and were already excluded).

### Subprocess execution

The original shells out to `ffprobe`, `ffmpeg`, `yt-dlp`, and Python for a handful of tasks. POSIX quoting/spawning and Windows' `CreateProcess`/command-line quoting rules are entirely different beasts, so `process_util.cpp` gained a full Windows implementation: an sh-compatible tokenizer that parses the POSIX-style quoted command string the rest of the app already builds, then re-serializes each argument using the (unintuitive, backslash-doubling) rules `CreateProcessW` actually expects. `CreateProcess`'s handle inheritance also turned out to be racy across concurrent spawns in a way POSIX's `posix_spawn` isn't — mousiki spawns several subprocesses from different threads at once by design (a background metadata sweep, a track's own `ffprobe`/`ffmpeg`, yt-dlp resolution, the lyrics helper), so every spawn now gets an explicit `PROC_THREAD_ATTRIBUTE_HANDLE_LIST` instead of inheriting every handle open in the process.

### Native audio-file parsing

`native_duration.cpp` reads MP4/OGG/MP3 headers directly (no subprocess) for fast duration lookups. It leaned on `pread()`, `off_t`, and `open()` — none of which exist as such on Windows. Replaced with `_wopen()` (taking the native wide path, with `_O_BINARY` so the CRT doesn't mangle binary audio data by translating CRLF sequences inside it), `_fstat64`, and a `pread` emulation built on `OVERLAPPED` I/O. `off_t` is 32-bit under MSVC, so every offset in this file is a plain `long long` now instead.

### The playback thread

Track switches used to spin up a fresh OS thread per track to call into WASAPI. WASAPI's COM objects are apartment-affine to whichever thread created them, which a fresh thread per track violates outright. Playback now runs through one persistent device-worker thread for the whole session, which made `Player` genuinely concurrent with the main thread for the first time — it gained an internal mutex (`player.h`/`.cpp`) to guard against the two actually racing.

### Locale

`main()` calls `setlocale(LC_ALL, "")` to get correct character handling for the active locale — inherited from the original, not Windows-specific. What's Windows-specific is the consequence: that call also sets `LC_NUMERIC`, and on a comma-decimal Windows locale (German, French, ...), every `std::stod()` call in the app — settings parsing, `ffprobe`/JSON duration, lyric timestamps — silently truncated at the first `.` with no exception thrown. `LC_NUMERIC` is now pinned back to `"C"` immediately after, independent of whatever the rest of the locale is doing.

### Thread-safety net

Every background `std::thread` (metadata sweep, decode, lyrics fetch, waveform pass, search, playlist add, the device worker) is now wrapped so an exception escaping it becomes a log line instead of an immediate, silent `std::terminate()` — the default behavior for an uncaught exception in a detached thread, and on Windows that means the whole process vanishes with no message at all. Several of the bugs above were originally diagnosed by their symptom being exactly this: total, silent process death with nothing to go on.

### Python helper scripts

`scripts/fetch_lyrics.py` and `scripts/lrc.py` are unchanged in what they fetch, but gained a UTF-8 stdout/stderr reconfiguration at startup. Python picks a text encoding for a redirected pipe from the OS locale; Linux desktop sessions inherit a UTF-8 `LANG` down to every child process automatically, Windows has no equivalent, so Python fell back to the ANSI code page — meaning a lookup for a Japanese, Cyrillic, or otherwise non-Latin track title could crash the script outright the instant it tried to print that title back (even just to report "no lyrics found"), which looked from the app's side identical to the script not existing at all. `scripts/fast_yt_search.py` — present upstream but never actually called from the C++ side — is now wired in as the default online search path (see below), with the same UTF-8 safeguard applied on principle.

## Added Features Beyond the Port

Below is a list of major and minor addition on top of the original v1.0. The design remained untouched.

### Major Additions / Modifications

- **Radio mode** in the same program (see [Radio mode](#default-keybinding-radio-player)): online radio with a tuning-dial look, ON/OFF AIR sign, 16-slot presets, Radio Browser search, station lists, recording to MP3, sleep timer, listening history, own oscilloscope/sphere and its own settings. `SHIFT` and `+` (the `*` character) switches between player and radio; the radio is closed completely when you leave it, the player waits in the background (paused) so it is back instantly. The cheat sheet shows the switch key for your keyboard layout (detected automatically, `MOUSIKI_KEYBOARD` overrides it - NOT TESTED YET IF IT FULLY WORKS).

<p align="center"><img width="848" height="411" alt="image" src="https://github.com/user-attachments/assets/efac8af6-6108-4558-911c-bab5f2e36931" /></p>

- **Port on linux/macOS** I adjusted the code so the current v2.5.0 also runs on the initial platforms again. I haven't tested this yet and there might be adjustments in the future. Note that macOS has no ALT key. When installing on macOS all cheat sheet and command legends will be adjusted accordingly.
- **Setup/portable (x64)** included in the latest release (since v2.1.0 for Win and since v2.5.0 also for Linux/macOS) as an alternative to building the app yourself. Setup size for windows is currently ~80MB and results in a ~250MB build (might optimize in the future), the portable .zip has ~100MB.
- **YX mode oscilloscope** as alternative to the lyrics ball. Parameters such as decay can be changed in an overlay menu via `SHIFT+o`. Besides the braille style there is an **image style** (Osci style: image): a real pixel picture with glow, drawn by the terminal itself via the Kitty graphics protocol (Kitty, WezTerm, Ghostty, Konsole …) or Sixel (Windows Terminal 1.22+, foot, xterm …); without either the braille scope stays. `MOUSIKI_GFX=kitty|sixel|off` forces a protocol, `MOUSIKI_CELLPX=10x20` the cell size (otherwise asked from the terminal). Frame rate 30 / 45 / 60 / 90.

<p align="center"><img width="849" height="400" alt="image" src="https://github.com/user-attachments/assets/a25d8ef3-fa17-4a6b-a7c5-28d214778e03" /></p>

- **Settings → REFERENCE**: the *Reset all keys* line sits at the top under the cheat-sheet note, `CTRL+SHIFT+U` undoes the last 5 key changes. Keys are shown as `SHIFT+t` (not `T`); the cheat sheet aligns its description column to the longest key and wraps descriptions at 120 columns.
- **COLORS tab** has a `TAB_NAMES` row (current / other tab name, `ColorTabCurrent` / `ColorTabOther`) like the radio; the playlist editor's name field (25 characters) sits in the tab strip.
- **Listening history** has *Listening by hour* and *Listening by weekday* in the HABITS tab; the playlist and history menus use the settings-style tab strip; the COLORS tab shows a colour swatch next to each value.
- **Sleep timer** with several options to choose from incl. fade-out mode (open via `SHIFT+z`). Can be turned off again. Timer resets after restart of the app and "off" is set as default.
 
<p align="center"><img width="848" height="395" alt="grafik" src="https://github.com/user-attachments/assets/39607465-4293-47b3-a380-dd49fe936a6d" /></p>

<p align="center"><img width="845" height="396" alt="image" src="https://github.com/user-attachments/assets/1b3d676a-69a4-45c6-ae12-b86f8be04239" /></p>

- **Adjust lyrics timing** menu where an offset of max. +/-120s can be added tot he lyrics. This is specially helpful when songs where downloaded from youtube, where a video version includes scenes before the actual song starts etc.

<p align="center"><img width="848" height="397" alt="grafik" src="https://github.com/user-attachments/assets/05c23222-efb6-44be-b74b-3e47803df2d1" /></p>

- **10 band EQ** overlay with 13 presets which can be opend in the main playback UI via `SHIFT+e`. Custom presets can be created, saved and deleted. Built in presets can't be deleted. 

<p align="center"><img width="848" height="399" alt="grafik" src="https://github.com/user-attachments/assets/534f4f2c-2d3e-4a35-a99c-e8cd78861dcd" /></p>

- **Track and queue list overlay** for the playback UI: `SHIFT+L` opens an enlarged list pane, `SHIFT+K` an enlarged queue pane (kept off `q` on purpose, so you cannot quit by accident), `SHIFT+↑/↓` scroll page-wise, `ESC` or the same key closes. Playback keys keep working inside the overlays. Queues are locked by default (a played track moves to the end instead of disappearing; unlock with `!` and it leaves the queue) and the queue can be saved as playlist (moving to the playlist menu). Several other commands such as add to end and move to top/bottom are also added
 
<p align="center"><img width="850" height="402" alt="grafik" src="https://github.com/user-attachments/assets/52a502e6-2ccc-4dd9-94e2-84663221bda6" /></p>

- **Playlist manager** - Via `SHIFT + p` or `P` respectively a playlist menu can be entered and playlists from local files can be created; search in main UI via `/p:`, hit `Enter` and its titles are added to the current queue.

<p align="center"><img width="847" height="400" alt="image" src="https://github.com/user-attachments/assets/45b012e0-41b5-4508-81a4-1871bae99209" /></p>

<p align="center"><img width="849" height="390" alt="image" src="https://github.com/user-attachments/assets/769f37cb-9a4b-45e2-83b7-ac14bfd4b5b1" /></p>

- **Meta/tag editor incl. fetch via AcoustID** (`SHIFT+M`, rebindable as `HKeyMetaEditor`) — a second full-screen overlay shaped similar to the playlist menu (tab strip, boxed panels, search field, hint/status footer) for changing a file's **name**, **artist**, **title**, **album** and **year**:
  - `TAB` cycles search field → library list → the five field rows, where typing edits the hovered field directly; `←/→` switches between the **EDIT** tab and the **FETCH LIST** tab (in the search field only until something is typed there; in a field row they move the caret and `SHIFT+←/→` marks text).
  - Every field that was touched — typed *or* filled in by a lookup — stays drawn in the **header colour**, and the matching **file rows in the LIBRARY panel are drawn in that same header colour** (with an `[ N edited ]` count in the panel's footer), so "which files does this session touch, and what exactly will be written?" is answerable at a glance.
  - sorting of the library pane for titles without any meta data (`x`) with missing title (`SHIFT+t`), missing artist (`SHIFT+a`) and missing year (`SHIFT+y`) is possible for better overview; `r` toggles the edited files to the top. `DEL`/`d` removes a title from the fetch list.
  - **AcoustID fetch (audio fingerprint)**: `SHIFT+B` looks up the hovered title (`Browse` list or inside the menu), `a` queues a title for the batch like the queue's add key, `ENTER` on the FETCH LIST tab runs the whole batch. Both ask the exact disclaimer *"Fetching meta data via AcoustID; not always accurate and previous meta data will be overwritten. Continue?"* first (Yes/No/ESC) and results only ever land **in the edit session**, never directly in the files. The lookup identifies the *audio*, not the text in the file name: `scripts/fetch_meta.py` runs `fpcalc` — a drop-in rebuild of Chromaprint's own tool, compiled by CMake from the vendored [Chromaprint](https://github.com/acoustid/chromaprint) 1.6.1 sources in `third_party/chromaprint/` (`tools/fpcalc.cpp` drives the library, ffmpeg does the decoding, so no prebuilt binary ships) — to fingerprint the first ~120 s of the file, sends fingerprint + duration to the AcoustID web service (paced to its 3 requests/second limit) and takes **artist + title** from the best-matching recording — confidence below 0.5 is reported as "no match" rather than written as a wrong tag, and among equally confident recordings the one whose duration is closest to the file wins. Progress shows as `3/7 …` while it runs.
  - Note that "year" and "album" is not fetched since it becomes rather complicated for a lot of albums, considering re-releases such as remasterd album versions. MusicBrainz (which could be added) would be necessary for that and an option to choose between different possible years and album names that were fetched. Since this makes the process of meta data editing rather complex, I discarded the idea to include it. It also might not work well with titles fetched from you tube via yt-dlp. 
  - **API key**: every AcoustID request is signed with an application key that is hard-coded as `API_KEY` at the top of `scripts/fetch_meta.py` — deliberately *not* a setting, because an AcoustID key belongs to one registered application rather than to a user, and the account-key/application-key mix-up is exactly what the service answers with *"invalid API key"*. A highly modified or rebranded build should maybe register its own application (its free!) at <https://acoustid.org/new-application> and change that one line; an invalid key fails with a status line that says so (`NO_KEY`) instead of guessing from the file name.
  - **Always-autosaved session**: the pending edits are written to `~/.cache/mousiki/meta_session/session.json` after every keystroke, so ESC, quitting or crashing keeps them as a backup — the audio files themselves are *never* touched by merely editing.
  - `CTRL+S` applies the session (asks *"Want to save?"*) — tags go through an `ffmpeg -c copy` remux into a temp file that is renamed over the original (audio stays bit-for-bit identical), a name edit becomes a plain rename; `CTRL+SHIFT+X` throws the pending edits away (asks *"Want to discard changes?"*). Failed entries stay in the session so they can be retried. These two are deliberately **not** rebindable: they are modifier combinations, which a hotkey string cannot express — the same reason `SHIFT+B` is matched directly too.
 
<p align="center"><img width="848" height="392" alt="image" src="https://github.com/user-attachments/assets/7f79c059-cfe0-4d80-a832-c1a84d901b4c" /></p>

<p align="center"><img width="846" height="387" alt="image" src="https://github.com/user-attachments/assets/0d3c6e3e-1c81-4786-8e42-9be1e43ecb91" /></p>

- **New Screen when no title loaded in Playmode "stop" mode** Added an Braille-Ascii music cassette and centered the statement that no track is currently loaded. Not thaaat of major change, but since it adds a design feature, which I didn't do before, I listeded here.

<p align="center"><img width="850" height="399" alt="grafik" src="https://github.com/user-attachments/assets/f04a4225-f4d1-48c4-a8c5-8ff9adf80a0a" /></p>

- **Playlist folder copy**: when the playlist path is changed in the settings, the existing playlists are copied to the new folder (nothing is moved or deleted; the music and download folders are never copied).
- **Editable path lists in the settings panel** — Settings → PATH now has a **LOCAL PATH**, **DOWNLOAD FOLDER** section and a **PLAYLIST PATH** section, the local paths one row per configured path ending in a `(+ new path)` row that appends a new empty line to type into (Enter on it opens the field immediately). Emptying a line removes that path. `LocalMusicPath=`/`PlaylistsPath=` in config.txt still work identically. Paths in those lists:
  - are *live*: committing a local path rescans the library on the spot instead of waiting for the next launch;
  - the playlist folder is a single path (one row, no "+ new path"); changing it copies the existing playlists over (see above).
  - yt-dlp download folder can now be set in the Setting; only one folder is possible and the folder will automatically be added to local paths, so no extra path adding necessary

<p align="center"><img width="849" height="390" alt="image" src="https://github.com/user-attachments/assets/10b49a1d-d66f-4b40-a829-e81416f0c416" /></p>

- **Listening history** via `SHIFT+h` including the last 100 tracks that had been played, the duration of titles where resorting can be done via the `r` key (default sort is "most played tracks on top" second sort is "least played title on top"), and tracking listening habits containing average session length, time music has been played per day, tracks per session, number of skips, replays and completion rates (how many times did a song finish).
  - top tracks (top 10 / 25 / 50 / 100) can be added to the queue in the second menu tab (`TAB` switches pane, `Enter` adds). Tabs are switched with `1` / `2` / `3`.

<p align="center"><img width="847" height="403" alt="image" src="https://github.com/user-attachments/assets/b345041f-176b-45a0-8955-68a30b766df1" /></p>

<p align="center"><img width="847" height="404" alt="image" src="https://github.com/user-attachments/assets/6fe10de4-ccad-4ef2-8f90-20994f7d4d9e" /></p>

<p align="center"><img width="849" height="399" alt="image" src="https://github.com/user-attachments/assets/dd1e4763-d72b-4d95-aaa2-51591e22e4d0" /></p>

- **Stereo Playback** - Can be toggled in the settings menu. Visualizations rely on a the usual duplicate mono channel.
- **Loudness Normalization** - Parameters can be set in the config.txt and toggled on and off via `v` and in the overlay menu tab via `SHIFT+v`, where parameters can be adjusted.

<p align="center"><img width="874" height="397" alt="grafik" src="https://github.com/user-attachments/assets/c4a217b5-8a81-4707-ad13-dc47b79df95a" /></p>

### Minor Additions / Modifications

- **Consistent keys (after v3.0.0)** - Volume is `+` / `-` in both modes (was `1` / `2` in the player), the lyrics area cycles with `.` (was `+`). Existing configs that still carry the old default keys are moved over automatically; keys you rebound yourself are kept. The playlist editor, the meta data editor and the radio's station lists menu switch tabs with `←/→` (`ALT+←/→` still works everywhere) and save with `CTRL+S`; in a text box the arrows move the caret once something was typed there, and `ESC` leaves the box before it closes the menu. The legends show which of the two the arrows do right now. In the Settings of both modes every field being edited (colours included, which had no caret in the radio) uses `←/→` for the caret and keeps `TAB`, until `ENTER` or `ESC`. `CTRL+S` needs the terminal's XON/XOFF flow control off, which Mousiki now does in raw mode. Also fixed: `ALT+L` / `CTRL+L` shared their key code with a hidden second mode switch (`CTRL+SHIFT+M`) and opened the radio instead of the lyrics timing overlay; that second switch is gone, `SHIFT` and `+` (the `*` character) is the only mode switch. In the radio, `.` now switches the scope block between oscilloscope and sphere, like the lyrics-area cycle in the player (`o` no longer does this). The settings of both modes show ``Unsaved changes! Save with `s` or discard with `ESC`.`` while something is not saved yet. Two commands start the app: `mousiki` and `lala` (installer, `.deb`, `.pkg`, portable zips, `setup.sh`, `setup.ps1`).
- **Karaoke, smart history, M3U8 export** - `k` opens a karaoke overlay (lyrics over the whole screen with a braille karaoke picture in the disk colours; Refresh UI moved to `r`, old configs are migrated). The history's queue pane is now **ADD SMART HISTORY TO QUEUE** with 16 lists in four columns (top N, top of the week / month / quarter / year, time of day, newly added and least played); `TAB` no longer switches the history tabs, `←/→` do. `e` on the playlist editor's Saved Playlists tab exports a playlist as M3U8 / M3U; the export folder is a new single path under Settings → PATHS (`PlaylistExportPath=`).

- **Categorized** Key commands in Reference tab within the Settings. The header color is no longer hardcoded: the COLORS tab has an **`HEADER`** field (config.txt: `ColorHeader=`, default `10` = palette index 10 of 256) which drives both those category titles and the path-list titles described below.
 
  <p align="center"><img width="850" height="400" alt="grafik" src="https://github.com/user-attachments/assets/e060895e-e6fa-4132-a9c9-67e2d0ea2167" /></p>

- **Categorized cheat sheet** available via `?`.

  <p align="center"><img width="850" height="358" alt="grafik" src="https://github.com/user-attachments/assets/f7b2b425-1156-4017-b354-bae54a7a3fb9" /></p>
  
- **Icon for .exe** is now included.
- **Full resizing of main playback UI** and menus when size of terminal is changed, e.g. fullscreen
- **Added color for legends** except those that are right on the border which remain using the border_color. 
- **Search Folder by Name via `/f:`** hit `Enter` and the content is shown in the local audio pane, similar using `f`; use `c` or `ESC` to clear. 
- **Folder Order and Sorting** `f` shows only the titles in a folder of the hovering track in the list. It now shows which folder. `SHIFT+n` was added in the past to toggle between showing the file name and the meta data track name in the local audio files list. I adjusted the sorting algorithm now sorts what is shown in the respective column, adapting to the set `SHIFT+n` mode. 
- **Copy/Paste/Cut in Search and Path Fields** All search fields now allow copy/paste/cut and the necessary marking. Same for fields to add local path.
- **Clear QUEUE** In the main UI `SHIFT+x` can now be used to clear the queue incl. a warning message that pops up. 
- **Fetched Lyrics Folder** is now placed in an extra folder such that lyrics of song_a in folder_a are located in folder_a/lyrics/ and are not placed directly next to track files. Note that changing the name of the file may trigger a new lyrics fetch process (did not find a reasonable solution for that since filenames can eventually also be changed via the Windows explorer or other programs such as Midnight commander or Yazi... Hard to keep track of...
- **Search Engine Optimization** - Added fuzzy search ("X-Files" didn't show up when searched "X Files", i.e. without dash) and optimized speed for also searching through meta data (not only file titles), which in Windows took >2-3 min. after the app was started to be available (cache cap was also an issue and a bunch of subprocess handling via ffprobe, which remains as a fallback method in case the newly included ID3v2.3/2.4 frame-walker that reads TIT2/TPE1/TALB directly out can't handle the tag layout of a file for some reason...). The speedup currently covers MP3 handling FLAC (Vorbis comments), OGG/Opus, and M4A/AAC. 
- **Hotkey remapping actually works in app (see above)** — as much as I understood a bug fix rather than a feature, but it's new behavior either way.
- **Shuffle-to-next** (`#`) — a manual one-off jump to a random track, independent of the persistent Shuffle play mode.
- **Special letters** (see above) — Fixed displaying and typing special letters like Umlaute (ä, ö ü) or accents á, à, Japanese letters etc. Emojis also work, but some 3-byte Emojis may mess up the UI when shown (can be turned off in settings)... 
- **Metadata-only / filename list rows** — `SHIFT+N` (or Settings → ON/OFF → **"Show meta data only"**) swaps every (search-)list row between the long-standing *filename + metadata* presentation and *metadata only*, i.e. the embedded title tag instead of the filename stem. Untagged files (and rows whose tags haven't been probed yet) keep their filename, so an untagged library never turns into a blank list. Applies to the main list, its search results, and both lists in the playlist editor. Also applies to the field between the disk animation and lyrics/sphere. Rebindable like every other hotkey (`HKeyToggleMetaOnly`; use `g` instead if you'd rather not rely on Shift).
- **Scrollable settings panel** —  REFERENCE now scroll with the cursor instead of growing past the panel, so the tab stays usable on a short terminal (32 rows and below).
- **Fast online search.** `scripts/fast_yt_search.py` hits YouTube's internal search endpoint directly instead of shelling out to `yt-dlp` for every keystroke-triggered search — `yt-dlp` is a general-purpose extractor for hundreds of sites and pays for that generality in startup time. `yt-dlp`'s own search is the fallback whenever the fast path comes back empty for any reason (script missing, network hiccup, or a genuine zero-result query), so nothing regresses if the fast path is ever unavailable. It approximates `yt-dlp`'s old `duration >= 90s` result filter (dropping shorts and live streams) but can't replicate the `categories *= 'Music'` half without a second request per result, which would defeat the point.
- **Tracker modules and chiptunes.** MOD / XM / IT / S3M and 20+ other tracker formats (via FFmpeg's libopenmpt), NES / SNES / Game Boy / Sega console music (`.nsf`, `.spc`, `.gbs`, `.vgm` … via FFmpeg's libgme) and C64 SID tunes (via the external `sidplayfp`, rendered once to a cached WAV; length `SidPlayLength` in `config.txt`, default 180 s) play like any other file, with the module title / SID name, author and year as metadata. Nothing is linked into Mousiki, so the licences of these libraries stay separate.
- **Radio timeshift.** The radio keeps the last 5–60 minutes (Settings → ON/OFF → *Timeshift buffer*, default 30) of the tuned station in a rolling buffer on disk (~11.5 MB per minute, in `~/.cache/mousiki/`, deleted on exit): `SPACE` pauses live radio, `[` / `]` jump 30 s, `{` 5 min back, `}` returns to live; the jump is shown in the scope block for 4 s and the dial shows `PAUSED` / `TIMESHIFT -m:ss`. `y` opens a RECORD overlay that freezes the moment it was pressed and can start the recording up to 30 minutes in the past ("save the last 5 minutes"). The buffer is written by the stream thread only, so the interface does not slow down.

- **Long-title handling.** Track titles that overflow their column now word-wrap (up to 3 lines) in the metadata panel, aligned under the value rather than repeating the label, and marquee-scroll horizontally in the local list when a track is hovered — both width-aware for wide (CJK) characters, not just byte-counted.
- **A Lyrics Engine toggle that actually gates fetching**, not just the panel's visibility (`.` to toggle, or Settings → On/Off) — previously the fetch ran and hit the network every single track regardless of whether the panel was shown. Toggling it off now shows the sphere visualization in that space instead of leaving it blank.
## Current Ideas on Features and Modifications 

**Basic Features (that will definitely be implemented soon):**
- no current to-dos... Feel free to start discussion or report issue! 

**Major New Features:**

- (NOT SURE ABOUT THIS, but idea sounds nice) Apart from regular playlists a modified playlist feature could be added: pixel art cassette tapes with limited number of tracks, A/B side, which can be shared. The cassettes could consist of a number of basic components like cassette style, label style, a decent number color sets (incl. a randomizer for composing the cassette style that optionally keeps track of what had been used already in the list of "cassette mixtapes"(i.e. with or without possible color redundancies or so))), may incl. a yt-dlp feature, where you can share a "cassette files / mixtapes (.mix files)" with others which include a list of commands (youtube urls) that can be shared and uploaded to your Mousiki player (commands that initialize starting fetching songs from you tube or elsewhere(local search included)); possible royalty free art that could be adjusted for that purpose (https://pixabay.com/illustrations/search/cassette%20tape/)

## Verifying downloads

Every release file (installer, `.deb`, `.pkg`, portable zips) is built by the public GitHub Actions workflow in this repository and comes with:

- `SHA256SUMS.txt` plus a GPG signature (`SHA256SUMS.txt.asc`), and a `.asc` signature next to every package
- a GitHub build-provenance attestation (proves which commit and workflow built the file)

**Release signing key** (GPG, ed25519, expires 2031-10-06)
Fingerprint: `0C8F 861B 48F4 7145 EE64  30F6 DE24 4FF4 9A0F 0902`

```bash
gpg --import mousiki-release-key.asc
gpg --fingerprint 0C8F861B48F47145EE6430F6DE244FF49A0F0902   # must show the fingerprint above
gpg --verify SHA256SUMS.txt.asc SHA256SUMS.txt
sha256sum -c --ignore-missing SHA256SUMS.txt                 # Windows: certutil -hashfile <file> SHA256
gh attestation verify <file> --repo StSchwerdtfeger/Mousiki-v3.0.0-Music-and-Radio-Player
```

Note: the files are not signed with a purchased code-signing certificate, so Windows SmartScreen and macOS Gatekeeper may still show a warning on first start.
