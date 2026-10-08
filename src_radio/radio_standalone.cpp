// Stand-alone radio executable (mousiki_radio, optional CMake target MOUSIKI_BUILD_RADIO_STANDALONE): development and
// tests. In the normal program the radio is started by main() (src/main.cpp) and SHIFT and + (the '*' key) leaves it for the player.
#include <clocale>
#include <iostream>
#include <string>
#include "mode_switch.h"
#if defined(_WIN32)
#include "win_compat.h"
#endif

int main(int argc, char** argv) {
#if defined(_WIN32)
    muisc::win_bootstrap_env();
    if (!muisc::win_console_init()) {
        std::cerr << "mousiki_radio: this console does not support ANSI escape sequences (use Windows Terminal).\n";
        return 1;
    }
#endif
    if (!std::setlocale(LC_ALL, "")) {
        std::setlocale(LC_ALL, "C.UTF-8");
    } else {
        const char* cur = std::setlocale(LC_CTYPE, nullptr);
        if (cur && std::string(cur) == "C") std::setlocale(LC_ALL, "C.UTF-8");
    }
    std::setlocale(LC_NUMERIC, "C");
    const int rc = radio_main(argc, argv);
#if defined(_WIN32)
    muisc::win_console_restore();
#endif
    return rc == kExitSwitchMode ? 0 : rc;
}
