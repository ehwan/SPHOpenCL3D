cmake_minimum_required(VERSION 3.5)
project( sph
  LANGUAGES CXX
)
set( CMAKE_CXX_COMPILER /usr/bin/clang++ )
set( CMAKE_C_COMPILER /usr/bin/clang )
set( CMAKE_EXPORT_COMPILE_COMMANDS ON ) # for clangd compile_commands.json

add_executable( sph
  engine.cpp
  main.cpp

  # MC33_cpp_library/source/grid3d.cpp
  MC33_cpp_library/source/libMC33++.cpp
  # MC33_cpp_library/source/MC33.cpp
  # MC33_cpp_library/source/surface.cpp
)
find_package( OpenCL REQUIRED )
target_link_libraries( sph PUBLIC OpenCL::OpenCL )
target_include_directories( sph PUBLIC MC33_cpp_library/include )
target_compile_definitions( sph PUBLIC 
  SPH_OPENCL_KERNEL_FILE="${CMAKE_CURRENT_SOURCE_DIR}/kernels.cl"
  SPH_OPENCL_FLAG_FILE="${CMAKE_CURRENT_SOURCE_DIR}/flags.h"
)
set_target_properties(sph PROPERTIES CXX_STANDARD 17 )