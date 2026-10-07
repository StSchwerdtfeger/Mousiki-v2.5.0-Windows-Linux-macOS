#pragma once
// Music player <-> radio mode. Both modes live in ONE program: a mode ends by returning kExitSwitchMode from its run
// function (Ctrl+Shift+M), main() then lets everything of that mode go out of scope -- audio device, threads, caches,
// terminal state -- and starts the other one. Only one mode is alive at any time.
constexpr int kExitSwitchMode = 77;

// The radio mode (src_radio/radio_main.cpp). Returns 0 on a normal quit, kExitSwitchMode for Ctrl+Shift+M.
int radio_main(int argc, char** argv);
