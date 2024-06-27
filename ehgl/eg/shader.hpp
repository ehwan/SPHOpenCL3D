#pragma once

#include "gl.hpp"
#include <algorithm>
#include <initializer_list>
#include <memory>
#include <string>

namespace eg
{

class Shader
{
  friend void swap(Shader&, Shader&);

  GLuint handler_;

  void release_if() const
  {
    if (handler_)
    {
      glDeleteShader(handler_);
      EG_CHECK_ERROR;
    }
  }

public:
  EG_DEFINE_OBJECT_OPERATOR(Shader, GLuint, 0, handler_)

  void source(GLsizei count,
              GLchar const* const* strings,
              GLint const* lengths = nullptr) const
  {
    glShaderSource(get(), count, strings, lengths);
    EG_CHECK_ERROR;
  }
  void source(GLchar const* string, GLint const length) const
  {
    glShaderSource(get(), 1, &string, &length);
    EG_CHECK_ERROR;
  }
  void source(GLchar const* strings) const
  {
    glShaderSource(get(), 1, &strings, nullptr);
    EG_CHECK_ERROR;
  }
  void source(std::initializer_list<GLchar const*> strings,
              std::initializer_list<GLint> lengths) const
  {
    source(strings.size(), &(*strings.begin()), &(*lengths.begin()));
  }
  void source(std::initializer_list<GLchar const*> strings) const
  {
    source(strings.size(), &(*strings.begin()), nullptr);
  }

  bool compile() const
  {
    glCompileShader(get());
    EG_CHECK_ERROR;
    GLint ret;
    glGetShaderiv(get(), GL_COMPILE_STATUS, &ret);
    EG_CHECK_ERROR;
    return ret != GL_FALSE;
  }
  std::string errorMessage() const
  {
    GLint length;
    glGetShaderiv(get(), GL_INFO_LOG_LENGTH, &length);
    EG_CHECK_ERROR;

    if (length > 0)
    {
      std::unique_ptr<char[]> buf(new char[length]);
      GLsizei read = 0;
      glGetShaderInfoLog(get(), length, &read, buf.get());
      EG_CHECK_ERROR;
      if (read > 0)
      {
        return std::string(buf.get(), read);
      }
    }
    return std::string();
  }
};
inline void swap(Shader& l, Shader& r)
{
  std::swap(l.handler_, r.handler_);
}

inline Shader make_shader(GLenum shader_type)
{
  auto ret = glCreateShader(shader_type);
  EG_CHECK_ERROR;
  return { ret };
}
inline Shader make_vertex_shader()
{
  auto ret = glCreateShader(GL_VERTEX_SHADER);
  EG_CHECK_ERROR;
  return { ret };
}
inline Shader make_fragment_shader()
{
  auto ret = glCreateShader(GL_FRAGMENT_SHADER);
  EG_CHECK_ERROR;
  return { ret };
}
inline Shader make_geometry_shader()
{
  auto ret = glCreateShader(GL_GEOMETRY_SHADER);
  EG_CHECK_ERROR;
  return { ret };
}

#if EG_GL_VERSION >= 400 || defined(EG_ARB_TESSELLATION_SHADER)
// tessellation shader
inline Shader make_tess_control_shader()
{
  auto ret = glCreateShader(GL_TESS_CONTROL_SHADER);
  EG_CHECK_ERROR;
  return { ret };
}
inline Shader make_tess_eval_shader()
{
  auto ret = glCreateShader(GL_TESS_EVALUATION_SHADER);
  EG_CHECK_ERROR;
  return { ret };
}
#endif
#if EG_GL_VERSION >= 430 \
    || (defined(EG_ARB_COMPUTE_SHADER) && EG_GL_VERSION >= 420)
inline Shader make_compute_shader()
{
  auto ret = glCreateShader(GL_COMPUTE_SHADER);
  EG_CHECK_ERROR;
  return { ret };
}
#endif

}
