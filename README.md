# SPHOpenCL3D
3D Smoothed Particle Hydrodynamics with OpenCL

 - OpenCL implementation of 3D SPH
 - Marching Cubes implementation using [libMC33++](https://github.com/dvega68/MC33_cpp_library)
 - OpenGL rendering using SFML

## Result

YouTube:
[![Dambreak Video](https://img.youtube.com/vi/cos2Pyizmww/0.jpg)](https://www.youtube.com/shorts/cos2Pyizmww)

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