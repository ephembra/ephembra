# ephembra

ephembra is an animated solar system demo for the JPL DE440 Ephemerides.

ephembra includes a animated OpenGL demo using the JPL DE440 Ephemerides
combined with [nanovg](https://github.com/memononen/nanovg) to plot
planetary orbit trails using a 3D projection and 2D vector graphics.
ephembra uses [matio](https://github.com/tbeu/matio) to convert the
MATLAB coefficients from [NASA JPL Development Ephemerides DE440](https://www.researchgate.net/publication/360748183_NASA_JPL_Development_Ephemerides_DE440).

![ephembra](/images/ephembra.png)

## build instructions

```
cmake -B build -G Ninja
cmake --build build
```
