# miniz 3.0.2 (vendored)

Single-file deflate/zlib implementation by Rich Geldreich et al. -- <https://github.com/richgel999/miniz>, MIT license (see `LICENSE`).

Used by `src/terminal_gfx.cpp` (and the radio mode) to compress the oscilloscope picture before it is sent to the terminal
over the Kitty graphics protocol (`o=z`). Only `mz_compress2()` / `mz_compressBound()` are used; the files are the unmodified
amalgamated `miniz.c` / `miniz.h` of the 3.0.2 release (the file header still says 3.0.0). No system zlib is needed.
