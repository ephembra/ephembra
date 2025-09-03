# ephembra

ephembra is an animated solar system demo for the JPL DE440 Ephemerides.

![ephembra](/images/ephembra.png)

## introduction

the ephembra OpenGL solar system demo plots objects from the JPL DE440 
Ephemerides. the demo adopts a hybrid 2.5D rendering style, combining 
the nanovg 2D vector canvas with 3D projective transforms implemented
using OpenGL-style 4x4 matrices.

the animated demo includes solar system objects with their orbit trails,
with options for text legends, grid, and zodiac layers, plus controls for
rendering parameters such as font size, line thickness, and zodiac offset.
the demo uses an interactive animation to illustrate the contrast between:

- heliocentric model: Sun-centered view of the solar system.
- geocentric model: Earth-centered view of the zodiac.

the ephemeris is ported from the MATLAB code in [NASA JPL Development
Ephemerides DE440][DE440].

## coordinate systems

ephembra relies on coordinate transforms from several reference frames:

- rest frame using the International Celestial Reference System.
- ecliptic frame using IAU 2006 obliquity and precession.

### rest frame

the JPL DE440 ephemerides are expressed in the [International Celestial
Reference System][ICRS], a quasi-inertial reference frame centered on
the [Solar System Barycenter][SSB]. in this frame, the origin is at the
SSB with coordinates _(x,y,z) = (0,0,0)_.

the ICRS axes are fixed relative to distant quasars, independent of
Earth’s orientation. for continuity with earlier systems, the ICRS is
aligned within a few milliarcseconds to the mean equator and equinox of
J2000.0; _Julian Date 2451545.0 TT, corresponding to noon on
January 1st, 2000 Terrestrial Time_.

thus, while the ICRS is barycentric and not tied to Earth’s motion,
its orientation remains close to Earth’s equator at J2000.0,
making it a natural _"rest frame"_ for solar system ephemerides;
the X-axis points to 0° Aries, the Y-axis points to 90° Libra,
and the Z-axis points perpendicular to the equatorial plane (up).

### ecliptic frame

to render the grid on the Earth’s orbital plane within the solar system,
ephembra applies a tilt followed by a rotation from the ICRS rest frame
using the [IAU 2006 precession][IAU2006]. specifically, it applies the
model’s mean obliquity (tilt) and precession (rotation) to obtain the _mean
ecliptic of date_, i.e. the plane of Earth’s mean orbital motion around
the Sun, used to center the zodiac.

for simplicity, nutation terms are omitted: only the obliquity
and precession of the mean vernal equinox are applied. this produces
an ecliptic-aligned frame suitable for zodiacal grids.

### cartoon scaling

ephembra includes _"cartoon scaling"_, which scales solar system objects
for an improved "perceptual" screen layout; otherwise, the planets are
too sparsely distributed to easily visualize the entire solar system.
this introduces some rendering issues, described in [zodiac](doc/zodiac.md).

## navigation

- keyboard navigation
  - (`W`, `A`, `Z`, `S`, `D`, `C`) = (_+x_, _+y_, _+z_, _-x_, _-y_, _-z_)
  - (`ESC`, `P`) = (_exit_, _save-screenshot_)
- mouse navigation
  - scroll wheel = zoom Z-axis
  - mouse click = object selection
- user interface controls
  - transform: IAU2006 precession, (X,Y,Z) rotation, (X,Y,Z) translation
  - style: trail width, line width, planet scale, font size, symbol size
  - grid: layer toggle, grid divisions, grid scale
  - zodiac: layer toggle, symbol offset, zodiac offset, zodiac scale
  - legends: symbols, names, distances

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
[ICRS]: https://aa.usno.navy.mil/faq/ICRS_doc
[IAU2006]: https://www.agi.com/getmedia/c85a440a-cf71-4e08-ad78-fa73736cee6c/Precession-nutation-Theories-and-their-Implementation.pdf
[SSB]: https://nanograv.org/glossary/solar-system-barycenter-ssb
[J2000]: https://aa.usno.navy.mil/faq/J2000

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
