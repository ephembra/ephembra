# ephembra

ephembra is an animated solar system demo for the JPL DE440 Ephemerides.

![ephembra](/images/ephembra.png)

## introduction

the ephembra OpenGL solar system demo plots objects from the JPL DE440 
Ephemerides. the demo adopts a hybrid 2.5D rendering style, combining 
the nanovg 2D vector canvas with 3D projective transforms implemented
using OpenGL-style 4x4 matrices.

the animated demo includes solar system objects with their orbit trails,
with options for text legends, grid, and zodiac layers plus controls for
rendering parameters such as font size, line thickness, and zodiac offset.

the ephemeris is ported from the MATLAB code in [NASA JPL Development
Ephemerides DE440][DE440].

## build

ephembra requires the following dependencies:

- submodules: [matio], [nanovg], [imgui], [stb]  
- packages: [GLAD], [GLFW], [freetype], [zlib], [brotli], [bzip2], [libpng]

[matio]: https://github.com/tbeu/matio
[stb]: https://github.com/nothings/stb
[nanovg]: https://github.com/memononen/nanovg
[imgui]: https://github.com/ocornut/imgui
[GLAD]: https://github.com/Dav1dde/glad
[GLFW]: https://github.com/glfw/glfw
[freetype]: https://github.com/freetype/freetype
[zlib]: https://github.com/madler/zlib
[brotli]: https://github.com/google/brotli
[bzip2]: https://gitlab.com/federicomenaquintero/bzip2
[libpng]: https://github.com/glennrp/libpng
[DE440]: https://www.researchgate.net/publication/360748183_NASA_JPL_Development_Ephemerides_DE440

ephembra has been tested on the following operating systems:

- Ubuntu 24.04 LTS
- FreeBSD 14.3

ephembra should work on the following operating systems:

- Windows 11
- macOS 15

```
cmake -B build -G Ninja
cmake --build build
```
