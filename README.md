# Planets

Created using C++, this program features spherical objects with planet textures, mimmicking real life orbits. The sun planet also using phong shading that illuminates the other planets in the scene. The user can also freely move around the scene using a camera viewpoint created from scratch.

### How to run
- Download and install the latest [CMake](https://cmake.org/download/) version
- Build "CMakeLists.txt" in Visual Studio
- Build and run "453-skeleton.exe"

### Controls

To control the spherical camera:
- Scroll wheel zooms in and out on the cube
- Holding the right mouse button and dragging allows you to rotate the camera around the cube
To control the animation:
- Space bar: toggle pause
- Left and right arrow keys: changes the speed of the animation
- R key: Resets the animation (reset rotation and speed) 

### -info

OS: Windows 10
IDE: Microsoft Visual Studio 2019

#-notes
Transformations Heirarchy can be found inside PlanetObject struct with the following order:
 - Orbit inclination * Orbit rotation * Distance from the sun * 2nd orbit incl. * 2nd orbit rotation * distance from 2nd orbit * Axis tilt * Axis rotation
 - light source is at the very centre (0,0,0), inside the sun
 - orbital inclination of the moon does not exactly match the equator of the moon on purpose, i tilted the earth too much to make the tilt obvious, however when
I match the moon's orbit with the earth's tilt it looks really wierd so i lessened its orbital inclination
 - Phong shading includes ambient and diffuser
