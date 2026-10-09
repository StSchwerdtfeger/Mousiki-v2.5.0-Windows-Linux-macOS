#pragma once
// The release version, shown on the ABOUT APP tab of the player and the radio.
// CMake passes it in from project(mousiki VERSION x.y.z) in CMakeLists.txt, which is the
// one place to change it for a release; the fallback below is only used by a build that
// does not go through CMakeLists.txt.
#ifndef MOUSIKI_VERSION
#define MOUSIKI_VERSION "dev"
#endif
