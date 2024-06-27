#pragma once

#include "buffer.hpp"
#include "buffer_target.hpp"
#include "program.hpp"
#include "uniform.hpp"
#include "vertex_array.hpp"
#include "vertex_attrib.hpp"
#include <cassert>
#include <iostream>
#include <vector>

namespace eg
{
namespace debug
{

class debug_context
{
  friend debug_context make_debug_context();
  struct color_t
  {
    GLubyte data[4];

    static GLubyte to_byte(float x)
    {
      return static_cast<GLubyte>(
          std::max(std::min(static_cast<int>(x * 255.0f), 255), 0));
    }
    color_t()
        : data { 255, 255, 255, 255 }
    {
    }
    color_t(GLubyte r, GLubyte g, GLubyte b, GLubyte a)
        : data { r, g, b, a }
    {
    }
    color_t(float r, float g, float b, float a)
        : data { to_byte(r), to_byte(g), to_byte(b), to_byte(a) }
    {
    }
    color_t(float r, float g, float b)
        : data { to_byte(r), to_byte(g), to_byte(b), 255 }
    {
    }
  };

protected:
  eg::VertexArray vertexarray_;
  const eg::VertexAttrib a_position_ { 0 };
  const eg::VertexAttrib a_color_ { 1 };

  eg::Program program_;
  eg::Uniform u_projection_;

  unsigned int count_;
  std::vector<float> positions_;
  std::vector<color_t> colors_;
  eg::Buffer position_buffer_;
  eg::Buffer color_buffer_;

  color_t last_color_;

  GLenum mode_;
  unsigned int dimension_ { 0 };

  template <typename T>
  static void sync_buffer(std::vector<T>& vec, eg::Buffer& buf)
  {
    auto target = eg::copy_write_buffer;
    target.bind(buf);
    const size_t size = sizeof(T) * vec.size();
    if (target.size() < size)
    {
      target.data(GL_STREAM_DRAW, size * 1.5, nullptr);
    }
    target.subData(0, size, vec.data());
    target.unbind();
  }

  void sync_color_count()
  {
    if (colors_.size() < count_)
    {
      colors_.insert(colors_.end(), count_ - colors_.size(), last_color_);
    }
  }

  void init_program();
  void init_buffer();

public:
  void init()
  {
    init_program();
    init_buffer();

    program_.bind();
    vertexarray_.bind();
    a_position_.enableArray();
    a_color_.enableArray();
    vertexarray_.unbind();
  }
  void begin(GLenum mode)
  {
    mode_ = mode;
    dimension_ = 0;
    count_ = 0;
    positions_.clear();
    colors_.clear();
  }
  void vertex2f(float x, float y)
  {
    assert(dimension_ == 0 || dimension_ == 2);
    dimension_ = 2;

    sync_color_count();

    ++count_;
    positions_.push_back(x);
    positions_.push_back(y);
  }
  void vertex3f(float x, float y, float z)
  {
    assert(dimension_ == 0 || dimension_ == 3);
    dimension_ = 3;

    sync_color_count();

    ++count_;
    positions_.push_back(x);
    positions_.push_back(y);
    positions_.push_back(z);
  }
  void set_color(float r, float g, float b, float a)
  {
    last_color_ = color_t(r, g, b, a);
  }
  void set_color(float r, float g, float b)
  {
    last_color_ = color_t(r, g, b);
  }
  void set_projection(GLfloat const* data = nullptr)
  {
    program_.bind();
    if (data)
    {
      u_projection_.matrix<4>(data);
    }
    else
    {
      const GLfloat buf[] = {
        1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1,
      };
      u_projection_.matrix<4>(buf);
    }
  }
  void end()
  {
    if (count_ == 0)
    {
      return;
    }
    sync_buffer(positions_, position_buffer_);
    sync_color_count();
    sync_buffer(colors_, color_buffer_);

    program_.bind();
    vertexarray_.bind();
    eg::array_buffer.bind(position_buffer_);
    a_position_.pointer(dimension_, GL_FLOAT);
    eg::array_buffer.bind(color_buffer_);
    a_color_.pointer(4, GL_UNSIGNED_BYTE, GL_TRUE);
    eg::array_buffer.unbind();
    glDrawArrays(mode_, 0, count_);
    EG_CHECK_ERROR;
    vertexarray_.unbind();
  }
};
void debug_context::init_program()
{
  const char* vertex_source = "#version 330 core\n"
                              "uniform mat4 u_projection;\n"
                              "in vec4 a_position;\n"
                              "in vec4 a_color;\n"
                              "out vec4 o_color;\n"
                              "void main(){\n"
                              "o_color = a_color;\n"
                              "gl_Position = u_projection * a_position;\n"
                              "}";
  auto vs = eg::make_vertex_shader();
  vs.source(vertex_source);
  if (vs.compile() == false)
  {
    std::cout << "debug vertex shader failed : " << std::endl
              << vs.errorMessage() << std::endl;
  }
  const char* fragment_source = "#version 330 core\n"
                                "out vec4 outcol;\n"
                                "in vec4 o_color;\n"
                                "void main(){\n"
                                "outcol = o_color;\n"
                                "}";
  auto fs = eg::make_fragment_shader();
  fs.source(fragment_source);
  if (fs.compile() == false)
  {
    std::cout << "debug fragment shader failed : " << std::endl
              << fs.errorMessage() << std::endl;
  }
  program_ = eg::make_program();
  program_.attachShader(vs);
  program_.attachShader(fs);
  program_.bindAttribLocation(a_position_, "a_position");
  program_.bindAttribLocation(a_color_, "a_color");
  program_.bindFragDataLocation(0, "outcol");
  if (program_.link_detach() == false)
  {
    std::cout << "debug program link failed : " << std::endl
              << program_.errorMessage() << std::endl;
  }
  u_projection_ = program_.uniformLocation("u_projection");

  program_.bind();
  set_projection();
}
void debug_context::init_buffer()
{
  vertexarray_ = eg::make_vertex_array();
  position_buffer_ = eg::make_buffer();
  color_buffer_ = eg::make_buffer();
}

debug_context make_debug_context()
{
  debug_context ret;
  ret.init();
  return ret;
}

}
}
