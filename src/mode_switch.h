#pragma once
// Music player <-> radio mode. Both modes live in ONE program, switched with SHIFT and + (the '*' character, a fixed
// key in both modes): a mode ends by returning kExitSwitchMode from its run function and main() starts the other one.
// The radio is destroyed completely on every switch (audio device, threads, caches, terminal state). The player is
// only SUSPENDED while the radio runs (playback paused, its App object kept alive by main()), so coming back to it is
// instant; it really ends when the program quits.
constexpr int kExitSwitchMode = 77;

// The radio mode (src_radio/radio_main.cpp). Returns 0 on a normal quit, kExitSwitchMode for the mode switch ('*').
int radio_main(int argc, char** argv);
