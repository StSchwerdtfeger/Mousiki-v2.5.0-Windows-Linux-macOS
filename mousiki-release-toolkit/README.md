# Mousiki release toolkit

Toolkit to create setup and portable version packages of the Mousiki TUI music player for Win/Linux/macOS. Set file paths (only dummy paths below) and repository names in case necessary for your demands. To create a setup package for macOS you have to create a repository on your Github account to make use of Github Actions. The packaging for Windows and Linux runs fully locally. The packaging for Linux (Ubuntu / Debian) uses WSL. The toolkit needs adjustments running it on other systems (written for Windows). Theoretically the packages can also be signed (not mandatory). I did not test and do so in the latest release for v2.5.0, but the toolkit was adjusted to be able to. 

One Python script (`release.py`, standard library only) turns the **Mousiki source `.zip`** into installers and portable zips.
Copy the `packaging/` and `.github/` folders into the root of your Mousiki GitHub repository
(`StSchwerdtfeger/Mousiki-Windows-Native-Port`); you run everything from Windows.

| Platform | Installer | Portable | Signing |
|---|---|---|---|
| Windows | `Mousiki-<ver>-windows-x64-setup.exe` (Inno Setup) | `…-windows-x64-portable.zip` | **SignPath Foundation** (Authenticode) |
| Ubuntu / Debian | `mousiki_<ver>_amd64.deb` | `…-linux-amd64-portable.zip` | **GPG** detached signatures |
| macOS | `Mousiki-<ver>-macos-universal.pkg` (arm64 + x86_64) | `…-macos-universal-portable.zip` | none (ad-hoc only) |

plus `SHA256SUMS.txt` (and `.asc`, `mousiki-release-key.asc` when GPG-signed) in `dist/`.

## What the installers create

| | Folder / entries |
|---|---|
| Windows | Start menu → Programs → **Mousiki**: *Mousiki*, *Mousiki User Manual (PDF)*, *Mousiki install folder*, *Uninstall Mousiki* (+ optional desktop icon / PATH entry) |
| macOS | **/Applications/Mousiki/**: `Mousiki.command` (double-click starts the player in Terminal), `Mousiki User Manual.pdf`, `Uninstall Mousiki.command`; plus `mousiki` in `/usr/local/bin` |
| Ubuntu | App menu entries **Mousiki** and **Mousiki User Manual** (a "Mousiki" menu folder on KDE/XFCE/MATE/LXQt – GNOME has no menu folders, so there both entries show up in the app grid); manual at `/usr/share/doc/mousiki/Mousiki_User_Guide.pdf`; command `mousiki` |

The manual is found automatically (first `*.pdf` under the source's `Mousiki_User_Manual*` folder).

## Where each platform is built

* **Windows** – on your PC (CMake + Visual Studio Build Tools + Inno Setup) **or** on GitHub Actions (required for signing).
* **Linux** – inside **WSL** (Ubuntu) on your PC, or on GitHub Actions. 
* **macOS** – only on GitHub Actions (or a Mac).

## One-time setup (Windows)

```powershell
python packaging\release.py doctor
python packaging\release.py build --target windows --install-deps    # winget: CMake, VS Build Tools (C++), Inno Setup
wsl --install -d Ubuntu                                              # once, then reboot + create the Linux user
winget install GitHub.cli ; gh auth login                            # cloud builds (macOS, signed Windows)
winget install GnuPG.Gpg4win                                         # GPG signing of the Linux files
```
Put `packaging/` and `.github/` into the repo root, commit, push.

## Everyday use

```powershell
# 1) new source zip -> repo (keeps .git, .github, packaging), commit, push
python packaging\release.py update-repo --zip C:\path\Mousiki-v2_5_1-main.zip

# 2) everything: Windows (local, unsigned) + Linux (WSL) + macOS (cloud)
python packaging\release.py all --zip C:\path\Mousiki-...zip --repo StSchwerdtfeger/Mousiki-Windows-Native-Port --gpg-key <KEYID>

# 3) the release run: signed Windows (cloud) + Linux (WSL) + macOS (cloud), after step 1
python packaging\release.py all --sign-windows --repo StSchwerdtfeger/Mousiki-Windows-Native-Port --gpg-key <KEYID> --zip C:\path\Mousiki-...zip
```
Single targets: `build --target windows`, `linux-wsl`, `cloud --targets macos`, `cloud --sign-windows --targets windows`.
`--update-repo` on `all` does step 1 first. (`set MOUSIKI_RELEASE_REPO=...` and `MOUSIKI_GPG_KEY=...` save typing.)

| Goal | Option |
|---|---|
| New version → new installers | run again with the new zip (only changed files are re-synced, CMake recompiles only those) |
| Refresh bundled ffmpeg / yt-dlp / Python / requests | `--refresh-tools` |
| Full rebuild of binaries | `--clean` (`--clean-all` also drops the download cache) |
| Smaller installers, rely on tools on PATH | `--no-bundle-tools` |
| Force a version | `--version 2.5.1` |

## Windows signing with SignPath Foundation

SignPath Foundation signs **open-source projects** and **verifies where the binaries come from**: they must be built by a public
GitHub Actions workflow from the public repository. That is why a signed build never uses a local zip – it builds the pushed repo
state (`update-repo` pushes it for you) and the CI flow is: compile → SignPath signs `mousiki.exe` + `fpcalc.exe` → package
(portable zip + installer with the signed exes) → SignPath signs the setup.exe → checksums.

**One-time setup**
1. Apply at <https://signpath.org/> (Apache-2.0 + public repo qualify; they also look at project reputation/traction). Mention that the installer bundles unmodified FFmpeg / yt-dlp / Python – ask whether signing the installer is fine; if not, delete the "Sign installer" step and sign only the two exes.
2. Add the section from `packaging/signpath/code-signing-policy.md` to your repo README/home page (mandatory).
3. In SignPath create the project (repository URL = your repo, trusted build system = GitHub.com), install the **SignPath GitHub App** on the repo, and add two artifact configurations from `packaging/signpath/` – slug `binaries` (`binaries.xml`) and `installer` (`installer.xml`) – and a policy `release-signing` with origin verification. Check the XML in SignPath's editor, it validates it.
4. GitHub repo → Settings → Secrets and variables → Actions: secret `SIGNPATH_API_TOKEN`; variables `SIGNPATH_ORGANIZATION_ID`, `SIGNPATH_PROJECT_SLUG` (optional: `SIGNPATH_SIGNING_POLICY_SLUG`, `SIGNPATH_CONFIG_BINARIES`, `SIGNPATH_CONFIG_INSTALLER`).

Each signing request may need your approval in the SignPath portal; the run waits up to 30 min per request.
Third-party tools inside the installer (ffmpeg.exe, yt-dlp.exe, python.exe) stay as upstream shipped them – only your files are signed.
Without these steps (or without `--sign-windows`) the Windows files are built unsigned.

## Linux: GPG signatures (no apt repository)

For a directly downloaded `.deb` the standard approach is a **signed checksum file** plus a published key fingerprint.
```powershell
gpg --quick-generate-key "Mousiki Releases <you@example.com>" ed25519 sign 5y    # once
gpg --fingerprint you@example.com                                                # publish this fingerprint
python packaging\release.py checksums --gpg-key you@example.com                  # or pass --gpg-key to build/all
```
Creates `SHA256SUMS.txt.asc`, `mousiki_<ver>_amd64.deb.asc` and `mousiki-release-key.asc`. Back up the secret key. Users verify with:
```bash
gpg --import mousiki-release-key.asc
gpg --verify SHA256SUMS.txt.asc SHA256SUMS.txt && sha256sum -c --ignore-missing SHA256SUMS.txt
gpg --verify mousiki_<ver>_amd64.deb.asc mousiki_<ver>_amd64.deb
```
(`apt` itself only checks repository metadata, so there is no way to make `dpkg -i` verify a lone `.deb`; this is the usual practice.)
Run the signing where your key lives – `all` signs at the end on your PC, after the WSL/cloud results are in `dist/`.

## macOS
Not signed or notarized (no Apple Developer account). On other Macs: right-click → Open, or
`xattr -dr com.apple.quarantine <file or folder>`. The `.pkg` and the portable zip work the same way. ffmpeg is not bundled (`brew install ffmpeg`).

## Package contents
`app/` (player), `scripts/` (+ `fpcalc` built from the vendored Chromaprint), `docs/` (manual), `config.txt`, `LICENSE`, `THIRD-PARTY.txt`, a **launcher**
(`mousiki.cmd` / `mousiki`) that puts the bundled tools on `PATH`, and for Windows ffmpeg, yt-dlp, embedded Python 3.12 + `requests`.
Linux `.deb`: depends on system ffmpeg/python3, ships yt-dlp; Linux portable also bundles a static ffmpeg; macOS bundles yt-dlp only.
**Portable zips** contain `portable.txt`: settings, cache and history then live in `data/` next to the launcher. Delete the file to use `~/.config/mousiki`.

## Notes
* `--sync-version` patches the *build copy* when `CMakeLists.txt` / the About screen still say an older version than the README title (your current zip: 2.3.0 vs 2.5.0).
* Uninstalling never touches `%USERPROFILE%\.config\mousiki` / `~/.config/mousiki`.
* The Inno `AppId` GUID in `release.py` is fixed so new installers upgrade old installs in place – do not change it.
* Bundled FFmpeg builds are GPL; licence files and download sources are listed in `THIRD-PARTY.txt`.
