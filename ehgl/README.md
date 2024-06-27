# ehgl
header-only OpenGL c++ wrapper

RAII based GL Objects and memory allocation

- This project is using `Eigen` for Perspective & Camera matrix construction ( `eg/math.hpp` )
and `SFML` for debug window creation ( `eg/debug_window.hpp` ).

- comment out those files in `eg.hpp` to remove dependancy.

- for your own OpenGL Loader, include main header file to `eg/gl.hpp`

## Sample Code

```cpp
// example/mandelbrot/mandelbrot.cpp

#include <eg.hpp>

// GL Program object
eg::Program program;
// GL VertexArrayObject object
eg::VertexArray varray;

// SFML windows ( for test )
eg::debug::window_context window;


int main()
{
/*
 ...
*/

  std::cout << "OpenGL Version " << eg::context.major_version() << "." << eg::context.minor_version() << std::endl;
  std::cout << "Vendor : " << eg::context.vendor() << std::endl;
  std::cout << "Renderer : " << eg::context.renderer() << std::endl;
  std::cout << "Shader : " << eg::context.shading_version() << std::endl;

  // program creation & linkage
  {
    // make vertex shader
    auto vertex = eg::make_shader( GL_VERTEX_SHADER );
    vertex.source( vertex_shader );
    vertex.compile();

    // make fragment shader
    auto fragment = eg::make_shader( GL_FRAGMENT_SHADER );
    fragment.source( fragment_shader );
    fragment.compile();

    // make program
    program = eg::make_program();
    program.attachShader( vertex );
    program.attachShader( fragment );
    program.bindFragDataLocation( 0, "outcol" );
    if( program.link_detach() == false )
    {
      std::cout << "program link failed : \n" << program.errorMessage() << std::endl;
      return 0;
    }
  }
  varray = eg::make_vertex_array();
  window.run();

  return 0;
}

```
