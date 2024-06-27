cmake_minimum_required(VERSION 3.5)
set( CMAKE_CXX_COMPILER /usr/bin/clang++ )
set( CMAKE_C_COMPILER /usr/bin/clang )
set( CMAKE_EXPORT_COMPILE_COMMANDS ON ) # for clangd compile_commands.json


project( sph
  LANGUAGES CXX
)
add_executable( sph
  engine.cpp
  main.cpp

  MC33_cpp_library/source/libMC33++.cpp
)
find_package( OpenCL REQUIRED )
target_link_libraries( sph PUBLIC OpenCL::OpenCL )
target_include_directories( sph PUBLIC MC33_cpp_library/include )
target_compile_definitions( sph PUBLIC 
  SPH_OPENCL_KERNEL_FILE="${CMAKE_CURRENT_SOURCE_DIR}/kernels.cl"
  SPH_OPENCL_FLAG_FILE="${CMAKE_CURRENT_SOURCE_DIR}/flags.h"
)
set_target_properties(sph PROPERTIES CXX_STANDARD 17 )

project( sph_render
  LANGUAGES CXX
)
add_executable( sph_render
  rendering/main.cpp
)
find_package( Eigen3 REQUIRED )
find_package( OpenGL REQUIRED )
find_package( SFML COMPONENTS graphics window system REQUIRED )
target_include_directories( sph_render PUBLIC ehgl )
target_link_libraries( sph_render PUBLIC Eigen3::Eigen OpenGL::GL sfml-graphics sfml-window sfml-system )
set_target_properties(sph_render PROPERTIES CXX_STANDARD 17)
target_compile_definitions( sph_render PUBLIC 
  SPH_RENDER_DIR="${CMAKE_CURRENT_SOURCE_DIR}/rendering"
)