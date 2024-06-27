# SPHOpenCL3D
3D Smoothed Particle Hydrodynamics with OpenCL

 - OpenCL implementation of 3D SPH
 - Marching Cubes implementation using [libMC33++]
 - OpenGL rendering using SFML

## Result

YouTube:
[![Dambreak Video](https://img.youtube.com/vi/cos2Pyizmww/0.jpg)](https://www.youtube.com/shorts/cos2Pyizmww)

## Dependencies
 - OpenCL
 - OpenGL
 - [SFML](https://www.sfml-dev.org/index.php) - Windowing and OpenGL context
 - [libMC33++](https://github.com/dvega68/MC33_cpp_library) - Marching Cubes
 - [ehgl](https://github.com/ehwan/ehgl) - OpenGL c++ wrapper

## Build & Run

To start the simulation,
```bash
$ mkdir build
$ cd build
$ cmake ..
$ make
$ ./sph
```
This will run the simulation and emit a vertices data file `vertices.dat`.

To render the simulation data,
```bash
$ mkdir build
$ cd build
$ cmake ..
$ make
$ ./sph_render vertices.dat
```
Pressing 'Q' will play the simulation.