#pragma once
// The ON AIR sign (braille art, one string per row). Blank U+2800 cells are left
// uncoloured by the renderer; everything else gets the diagonal viz-gradient.
constexpr int kSignW = 34;
constexpr int kSignRows = 12;
inline const char* const kOnAirSign[kSignRows] = {
    "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣀⣀⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀",
    "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢠⢞⣇⣹⣘⣸⡳⡄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀",
    "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣛⡨⠭⠝⠫⠭⠕⡃⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀",
    "⢰⠋⠉⠉⠉⠉⠉⠉⠉⠉⠉⠉⠉⣷⣊⠭⢅⡨⠭⣐⣺⠉⠉⠉⠉⠉⠉⠉⠉⠉⠉⠉⠙⡆",
    "⢸⠀⣴⠟⠛⢷⣄⣿⣦⠀⣼⡆⡔⣗⣒⣖⡙⢙⣓⣒⣾⠂⠀⣾⣧⠀⢸⡇⣾⡟⠛⣷⠀⡇",
    "⢸⢸⡇⠀⠀⢸⣿⣿⢻⣧⣿⡇⢹⣴⣒⣿⣂⣐⣒⣒⢽⠁⣸⣏⣿⡆⢸⡇⣿⣷⣶⡟⠀⡇",
    "⢸⠈⢿⣤⣠⡾⠏⣿⠀⠹⣿⠇⠈⡟⢌⡘⡨⢈⢩⡡⣻⢀⡿⠉⠙⣿⢸⡇⢿⠇⠙⣷⠀⡇",
    "⠸⣄⣀⣀⣁⣀⣀⣀⣀⣀⣀⣀⣀⣈⣒⢽⡝⢫⡧⣒⣁⣀⣀⣀⣀⣀⣀⣀⣀⣀⣀⣀⣠⠇",
    "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⡏⢹⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀",
    "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⡇⢸⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀",
    "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⠾⣉⣉⣕⣊⣉⡉⠷⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀",
    "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⠑⠒⠓⠒⠒⠒⠒⠊⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀"
};

// The OFF AIR sign (shown instead of the ON AIR sign while nothing is live) and the satellite that swings in the free pane
// on the right then. Same conventions: braille rows, blank U+2800 cells are left uncoloured. Both are trimmed to their ink.
constexpr int kOffAirW = 34;
constexpr int kOffAirRows = 12;
inline const char* const kOffAirSign[kOffAirRows] = {
    "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣀⣀⣀⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀",
    "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢠⢞⣇⣹⣘⣸⡳⡄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀",
    "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣛⡩⠭⢽⠫⠭⢕⡃⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀",
    "⢰⠋⠉⠉⠉⠉⠉⠉⠉⠉⠉⠉⠉⣗⣊⠭⢅⡨⠭⣐⣺⠉⠉⠉⠉⠉⠉⠉⠉⣉⡉⠉⠙⡆",
    "⢸⢀⣶⠟⠻⣦⣾⡟⠛⣿⡟⠛⡔⣗⣒⣞⡙⢙⣓⣒⣿⠂⢀⣾⣧⠀⢸⣇⣾⡟⠻⣷⡀⡇",
    "⢸⢸⡇⠀⠀⣿⣿⡷⠖⣿⡷⠆⢹⣐⣒⣿⣢⣐⣒⣒⢽⠁⣸⣟⣿⣆⢸⣿⣿⣷⣾⡟⠀⡇",
    "⢸⠘⢿⣤⣴⡿⣿⠇⠀⣿⠀⠀⠈⡟⣜⡙⣸⢉⢩⡡⣻⢠⡿⠛⠙⣿⣼⡟⣿⡇⠙⣷⠀⡇",
    "⠸⣄⣀⣈⣁⣀⣀⣀⣀⣀⣀⣀⣀⣈⣲⢽⡽⢫⡿⣒⣁⣀⣀⣀⣀⣈⣀⣀⣈⣀⣀⣈⣠⠇",
    "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⣯⢹⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀",
    "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⡯⢸⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀",
    "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⠾⣭⣉⣵⣪⣉⡩⠷⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀",
    "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⠑⠒⠓⠒⠒⠒⠒⠊⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀"
};
constexpr int kSatW = 22;
constexpr int kSatRows = 13;
inline const char* const kSatellite[kSatRows] = {
    "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣴⡦⠀⠀",
    "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢠⡒⠤⡺⡏⠁⠀⠀",
    "⠀⠀⣠⢏⠓⢤⡀⠀⠀⠀⠀⠀⠀⠀⢸⠈⠳⢬⣑⠀⠀⠀",
    "⠀⡴⠥⡀⠙⠢⣈⠳⢄⠀⠀⣀⠤⠼⣍⠳⣶⠒⠁⠀⠀⠀",
    "⠈⠲⢄⡈⠒⢄⡀⣵⡳⢄⠞⠁⠀⠀⠀⢻⠁⠀⠀⠀⠀⠀",
    "⠀⠀⠀⠉⠲⣄⠞⠲⣬⠋⢠⠖⢲⠀⠀⣸⠀⠀⠀⠀⠀⠀",
    "⠀⠀⠀⠀⠀⠀⠀⡴⠁⠀⠘⠶⠊⠀⣴⡣⣴⡲⢄⡀⠀⠀",
    "⠀⠀⠀⠀⠀⠀⠀⡇⠀⠀⠀⠀⢠⠞⢩⢞⠀⠉⠲⢍⠢⣄",
    "⠀⠀⠀⠀⠀⡀⠸⣝⠦⣄⡠⠴⠃⠐⠣⣄⠙⠢⣄⠀⢙⠞",
    "⠀⠀⠀⡠⠊⠙⠪⣟⠑⠋⠀⠀⠀⠀⠀⠀⠙⠦⣀⣹⠋⠀",
    "⠀⠀⠘⠦⡀⠀⠀⢈⡷⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠁⠀⠀",
    "⠀⠀⠀⣾⣮⠳⡴⠋⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀",
    "⠀⠀⠀⠈⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀"
};
