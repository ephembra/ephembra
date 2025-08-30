# ephembra

a little bit of fun regarding the formal mathematics of astrology.

- https://github.com/ephembra/ephembra

I have been working on an astronomical mathematics curiosity related to
the formal mathematics of astrology and I have had a visualization idea
on my mind that I wanted to try out.

## background

I have ported JPL's DE440 Ephemerides MATLAB code to C and have made a
tiny animated solar system demo with several graphical layers including
a geocentric projection of the zodiac.

- step 1. goal of showing the projective geometry behind retrogrades in
  astrology due to the geocentric projection of the zodiac.
- step 2. tune orbits by adding what I call "cartoon mode" which scales
  the solar system orbits for an improved "perceptual" layout.

## problem statement

the issue is the planets are too sparsely placed to visualize the
information using realistic scales. the second problem is the cartoon
scaling of the planets is not angle preserving, so the zodiac positions
in "cartoon mode" are not accurate. the zodiac projection pins
in "cartoon mode" should be in the same position relative to the
perceptual zodiac, as they are in respect to the real zodiac. I want to
make the cartoon mode angle preserving with respect to the zodiac.

so I tried to calculate an angular error between the real zodiac and
the "cartoon mode" zodiac and then apply a rotation to each planetary
orbit so that the zodiac pins are accurate *relatively speaking*. at
first glance, it seemed like a simple `atan2` with x,y offsets and
`asin` for an inverse, to find a delta error angle for each planet
based on the cartoon zodiac angles minus the real zodiac angle, but on
further investigation it is more involved.

relative angles in the scaled orbits depend on the projected size of the
zodiac ring, the scaled radius of each planet, and the real scales of
the solar system. it seems that I can't simply compute an angular
difference between the real and cartoon zodiacs and then apply it. the
positions don't match. there are three radii:

- current heliocentric distance of the planets (eccentric orbit),
- scaled average distance used for the cartoon scaling,
- SSB-centered zodiac circle with lines projected from the Earth.

## mathematical solution

this is what I started with but it is not sufficient:

map angles from a point (x0, y0) inside of a circle of radius 'r'
relative to center origin (0, 0) to points on the circumference.

  • θ (theta): central angle from (0,0) to point on circle [0, 2π).
  • φ (phi): ray angle from (x0,y0) to point on circle [0, 2π).

domain: |(x0,y0)| < r, r > 0, results normalized to [0, 2π)

forward: θ → φ:   phi = atan2(r*sinθ - y0, r*cosθ - x0)
inverse: φ → θ:   theta = phi + arcsin((y0*cosφ - x0*sinφ)/r)
