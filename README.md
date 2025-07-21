# C-Chess-Engine
A simple chess engine made in C++ using the UCI protocol.

## Build Instructions
- CMakeLists.txt is included, with an optional flag which when set, makes the engine play the top move in its opening book.
- Run `cmake` on the root directory and then `make`.
- Dependencies:
    - C++23 compatible compiler

```bash
mkdir build
cd build
cmake ..
make
```

- Or to specify the configuration:
```bash
cmake -S . -B build
cmake --build build --config Release
```

### CMake Configuration Details

C++ Standard: The project enforces C++23.

Build Types: Supports both Debug and Release builds. Defaults to Debug if not specified.

#### Compiler Flags:
- Debug: Includes debug symbols (-g) and -march=native for CPU-specific optimizations.
- Release: Uses -O3, -march=native, and -DNDEBUG for maximum optimization.

#### Output Directory:
- The executable and copied book.bin are output to ${CMAKE_BINARY_DIR}/bin.

#### Build Flags:
- TOPBOOK compile definition is enabled by default; it causes the engine to always select the top move from the opening book.
- You can disable it by commenting out the related line in CMakeLists.txt if you want more varied book play.

## Running
To use the opening book, the book.bin file must be in the same directory as the engine executable (CMake does this).
The engine uses the UCI interface; connect to the engine via a compatible GUI like [Cute Chess](https://cutechess.com/).

## License
MIT License - see [LICENSE](./LICENSE) for details.

## Acknowledgments
- [Chess Programming Wiki](https://www.chessprogramming.org/)
- Opening book is from the Banksia Gui [website](https://banksiagui.com/otherdownloads/)