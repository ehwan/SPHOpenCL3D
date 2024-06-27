// compile arguments :
// -lsfml-window -lsfml-graphics -lsfml-system -framework OpenGL

#include <stdexcept>
#define GL_SILENCE_DEPRECATION
#include "eg.hpp"
#include <fstream>
#include <iostream>
#include <vector>

eg::debug::window_context window;
eg::EyeAngle eye;
eg::Perspective projection;

// Checkered-Tile Shading
struct tile_program_t
{
  eg::Program program;
  eg::VertexArray vao;
  // vertex buffer, normal buffer
  eg::Buffer vb, nb;

  void init()
  {
    program = eg::make_program();
    vao = eg::make_vertex_array();

    auto vertex
        = eg::load_shader(SPH_RENDER_DIR "/tile_vert.glsl", GL_VERTEX_SHADER);
    auto fragment
        = eg::load_shader(SPH_RENDER_DIR "/tile_frag.glsl", GL_FRAGMENT_SHADER);

    program.attachShader(vertex);
    program.attachShader(fragment);
    program.bindFragDataLocation(0, "outcol");
    if (program.link_detach() == false)
    {
      std::cout << "program link failed : \n"
                << program.errorMessage() << std::endl;
      return;
    }

    vao = eg::make_vertex_array();
    vao.bind();

    GLfloat tileVB_[] = { -10.0, 0.0, -10.0, 10.0, 0.0, -10.0,
                          -10.0, 0.0, 10.0,  10.0, 0.0, 10.0 };
    vb = eg::make_buffer();
    eg::array_buffer.bind(vb);
    eg::array_buffer.data(GL_STATIC_DRAW, sizeof(float) * 3 * 4, tileVB_);
    eg::VertexAttrib(0).enableArray();
    eg::VertexAttrib(0).pointer(3, GL_FLOAT);

    GLfloat tileNVB_[]
        = { 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0 };
    nb = eg::make_buffer();
    eg::array_buffer.bind(nb);
    eg::array_buffer.data(GL_STATIC_DRAW, sizeof(float) * 3 * 4, tileNVB_);
    eg::VertexAttrib(1).enableArray();
    eg::VertexAttrib(1).pointer(3, GL_FLOAT);
  }

  void draw()
  {
    program.bind();
    vao.bind();

    program.uniformLocation("u_eye").matrix<4>(&eye()(0, 0));
    program.uniformLocation("u_projection").matrix<4>(&projection()(0, 0));
    program.uniformLocation("light0").v<3>(&eye.position()(0));
    Eigen::Vector3f lightvec = -eye.axis(2);
    program.uniformLocation("lightvec").v<3>(&lightvec(0));

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  }
};

// Marching Cubes fluid rendering
struct fluid_program_t
{
  eg::Program program;
  eg::VertexArray vao;
  // vertex buffer, normal buffer, index buffer
  eg::Buffer vb, nb, ib;

  std::ifstream file;
  float t = 0;
  int nverts = 0, ntri = 0;

  std::vector<float> verts;
  std::vector<float> norms;
  std::vector<unsigned int> inds;

  void init()
  {
    auto vertex
        = eg::load_shader(SPH_RENDER_DIR "/vert.glsl", GL_VERTEX_SHADER);
    auto fragment
        = eg::load_shader(SPH_RENDER_DIR "/frag.glsl", GL_FRAGMENT_SHADER);
    program = eg::make_program();
    program.attachShader(vertex);
    program.attachShader(fragment);
    program.bindFragDataLocation(0, "outcol");
    if (program.link_detach() == false)
    {
      std::cout << "program link failed : \n"
                << program.errorMessage() << std::endl;
    }
    program.bind();

    vao = eg::make_vertex_array();
    vao.bind();

    vb = eg::make_buffer();
    nb = eg::make_buffer();
    ib = eg::make_buffer();
    eg::array_buffer.bind(vb);
    eg::array_buffer.data(GL_DYNAMIC_DRAW, sizeof(float) * 3 * 1000000, 0);

    eg::array_buffer.bind(nb);
    eg::array_buffer.data(GL_DYNAMIC_DRAW, sizeof(float) * 3 * 1000000, 0);

    eg::element_array_buffer.bind(ib);
    eg::element_array_buffer.data(GL_DYNAMIC_DRAW,
                                  sizeof(unsigned int) * 3 * 1500000, 0);

    eg::array_buffer.bind(vb);
    eg::VertexAttrib(0).enableArray();
    eg::VertexAttrib(0).pointer(3, GL_FLOAT);

    eg::array_buffer.bind(nb);
    eg::VertexAttrib(1).enableArray();
    eg::VertexAttrib(1).pointer(3, GL_FLOAT);
  }

  void read_file()
  {
    file.read((char*)&t, sizeof(float));
    file.read((char*)&nverts, sizeof(int));
    file.read((char*)&ntri, sizeof(int));
    std::cout << t << " " << nverts << " " << ntri << "\n";
    verts.resize(3 * nverts);
    norms.resize(3 * nverts);
    inds.resize(3 * ntri);
    file.read((char*)verts.data(), sizeof(float) * 3 * nverts);
    file.read((char*)norms.data(), sizeof(float) * 3 * nverts);
    file.read((char*)inds.data(), sizeof(unsigned int) * 3 * ntri);

    eg::array_buffer.bind(vb);
    eg::array_buffer.subData(0, sizeof(float) * 3 * nverts, verts.data());
    eg::array_buffer.bind(nb);
    eg::array_buffer.subData(0, sizeof(float) * 3 * nverts, norms.data());
    eg::element_array_buffer.bind(ib);
    eg::element_array_buffer.subData(0, sizeof(unsigned int) * 3 * ntri,
                                     inds.data());
  }
  void draw()
  {
    program.bind();
    vao.bind();

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q) && file.eof() == false
        && t < 19.5)
    {
      read_file();
    }
    program.uniformLocation("u_eye").matrix<4>(&eye()(0, 0));
    program.uniformLocation("u_projection").matrix<4>(&projection()(0, 0));
    program.uniformLocation("light0").v<3>(&eye.position()(0));

    if (nverts > 0)
      // for( int i=0; i<ntri; ++i )
      //{
      // glDrawElements( GL_LINE_LOOP, 3, GL_UNSIGNED_INT, (const
      // void*)(3*i*sizeof(int)) );
      //}
      glDrawElements(GL_TRIANGLES, 3 * ntri, GL_UNSIGNED_INT, 0);
  }
};

tile_program_t tile_program;
fluid_program_t fluid_program;

// WASD camera moving
void move()
{
  {
    float spd = 1.0f / 60.0f;
    float anglespd = 1.0f / 60.0f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
    {
      if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
      {
        auto angle = eye.angle();
        angle.x() -= spd;
        eye.angle(angle);
      }
      else
      {
        eye.move(2, spd);
      }
    }
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
    {
      if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
      {
        auto angle = eye.angle();
        angle.x() += spd;
        eye.angle(angle);
      }
      else
      {
        eye.move(2, -spd);
      }
    }
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
    {
      if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
      {
        auto angle = eye.angle();
        angle.y() += spd;
        eye.angle(angle);
      }
      else
      {
        eye.move(0, -spd);
      }
    }
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
    {
      if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
      {
        auto angle = eye.angle();
        angle.y() -= spd;
        eye.angle(angle);
      }
      else
      {
        eye.move(0, +spd);
      }
    }
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::R))
    {
      eye.move(1, spd);
    }
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::F))
    {
      eye.move(1, -spd);
    }
  }
}

void event(eg::debug::window_context& w)
{
  if (w.event().type == sf::Event::Closed)
  {
    w.close();
  }
}
void enterframe(eg::debug::window_context& w)
{
  move();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  // glLineWidth(0.1f);
  tile_program.draw();
  fluid_program.draw();
  w.swap_buffer();
}

int main(int argc, char** argv)
{
  window.create(1000, 1000, "hello OpenGL");
  window.set_enterframe(enterframe);
  window.set_event(event);
  std::cout << "OpenGL Version " << eg::context.major_version() << "."
            << eg::context.minor_version() << std::endl;
  std::cout << "Vendor : " << eg::context.vendor() << std::endl;
  std::cout << "Renderer : " << eg::context.renderer() << std::endl;
  std::cout << "Shader : " << eg::context.shading_version() << std::endl;

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glDisable(GL_CULL_FACE);
  // glCullFace(GL_FRONT);
  // glFrontFace(GL_CCW);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  tile_program.init();
  fluid_program.init();

  eye.set({ 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 });
  eye.position({ 0.2, 1.0, 2.0 });
  eye.angle({ -0.5, -0.4, 0 });
  // projection.frustum( {-0.3,-0.3}, {1.3,1.3}, 1, 5 );
  projection.perspective(3.141592f / 2.0f, 1.0f, 0.1, 5.0f);

  if (argc < 2)
  {
    throw std::runtime_error("Argument error: enter vertices data file!");
  }
  fluid_program.file = std::ifstream(argv[1]);

  window.run();
  return 0;
}
