#pragma once

#include "gl.hpp"

namespace eg
{
namespace core
{
template <unsigned int>
struct uniform_v_t;
template <unsigned int, unsigned int>
struct uniform_matrix_t;
}
}

namespace eg
{

class Uniform
{
  GLint handler_;

public:
  constexpr GLint get() const
  {
    return handler_;
  }
  constexpr operator GLint() const
  {
    return get();
  }
  constexpr Uniform(GLint handler = 0)
      : handler_(handler)
  {
  }

  template <unsigned int I, typename T>
  void v(T const* v, GLsizei count = 1)
  {
    core::uniform_v_t<I>::apply(get(), count, v);
  }
  template <unsigned int A, unsigned int B = A>
  void matrix(GLfloat const* v,
              GLsizei count = 1,
              GLboolean transpose = GL_FALSE) const
  {
    core::uniform_matrix_t<A, B>::apply(get(), count, transpose, v);
  }

  void operator()(GLfloat f1) const
  {
    glUniform1f(get(), f1);
    EG_CHECK_ERROR;
  }
  void operator()(GLfloat f1, GLfloat f2) const
  {
    glUniform2f(get(), f1, f2);
    EG_CHECK_ERROR;
  }
  void operator()(GLfloat f1, GLfloat f2, GLfloat f3) const
  {
    glUniform3f(get(), f1, f2, f3);
    EG_CHECK_ERROR;
  }
  void operator()(GLfloat f1, GLfloat f2, GLfloat f3, GLfloat f4) const
  {
    glUniform4f(get(), f1, f2, f3, f4);
    EG_CHECK_ERROR;
  }

  void operator()(GLint i1) const
  {
    glUniform1i(get(), i1);
    EG_CHECK_ERROR;
  }
  void operator()(GLint i1, GLint i2) const
  {
    glUniform2i(get(), i1, i2);
    EG_CHECK_ERROR;
  }
  void operator()(GLint i1, GLint i2, GLint i3) const
  {
    glUniform3i(get(), i1, i2, i3);
    EG_CHECK_ERROR;
  }
  void operator()(GLint i1, GLint i2, GLint i3, GLint i4) const
  {
    glUniform4i(get(), i1, i2, i3, i4);
    EG_CHECK_ERROR;
  }

  void operator()(GLuint i1) const
  {
    glUniform1ui(get(), i1);
    EG_CHECK_ERROR;
  }
  void operator()(GLuint i1, GLuint i2) const
  {
    glUniform2ui(get(), i1, i2);
    EG_CHECK_ERROR;
  }
  void operator()(GLuint i1, GLuint i2, GLuint i3) const
  {
    glUniform3ui(get(), i1, i2, i3);
    EG_CHECK_ERROR;
  }
  void operator()(GLuint i1, GLuint i2, GLuint i3, GLuint i4) const
  {
    glUniform4ui(get(), i1, i2, i3, i4);
    EG_CHECK_ERROR;
  }
};
}

namespace eg
{
namespace core
{
template <>
struct uniform_v_t<1>
{
  static void apply(GLint index, GLsizei count, GLfloat const* v)
  {
    glUniform1fv(index, count, v);
    EG_CHECK_ERROR;
  }
  static void apply(GLint index, GLsizei count, GLint const* v)
  {
    glUniform1iv(index, count, v);
    EG_CHECK_ERROR;
  }
  static void apply(GLint index, GLsizei count, GLuint const* v)
  {
    glUniform1uiv(index, count, v);
    EG_CHECK_ERROR;
  }
};
template <>
struct uniform_v_t<2>
{
  static void apply(GLint index, GLsizei count, GLfloat const* v)
  {
    glUniform2fv(index, count, v);
    EG_CHECK_ERROR;
  }
  static void apply(GLint index, GLsizei count, GLint const* v)
  {
    glUniform2iv(index, count, v);
    EG_CHECK_ERROR;
  }
  static void apply(GLint index, GLsizei count, GLuint const* v)
  {
    glUniform2uiv(index, count, v);
    EG_CHECK_ERROR;
  }
};
template <>
struct uniform_v_t<3>
{
  static void apply(GLint index, GLsizei count, GLfloat const* v)
  {
    glUniform3fv(index, count, v);
    EG_CHECK_ERROR;
  }
  static void apply(GLint index, GLsizei count, GLint const* v)
  {
    glUniform3iv(index, count, v);
    EG_CHECK_ERROR;
  }
  static void apply(GLint index, GLsizei count, GLuint const* v)
  {
    glUniform3uiv(index, count, v);
    EG_CHECK_ERROR;
  }
};
template <>
struct uniform_v_t<4>
{
  static void apply(GLint index, GLsizei count, GLfloat const* v)
  {
    glUniform4fv(index, count, v);
    EG_CHECK_ERROR;
  }
  static void apply(GLint index, GLsizei count, GLint const* v)
  {
    glUniform4iv(index, count, v);
    EG_CHECK_ERROR;
  }
  static void apply(GLint index, GLsizei count, GLuint const* v)
  {
    glUniform4uiv(index, count, v);
    EG_CHECK_ERROR;
  }
};

template <>
struct uniform_matrix_t<2, 2>
{
  static void
  apply(GLint index, GLsizei count, GLboolean transpose, GLfloat const* v)
  {
    glUniformMatrix2fv(index, count, transpose, v);
    EG_CHECK_ERROR;
  }
};
template <>
struct uniform_matrix_t<2, 3>
{
  static void
  apply(GLint index, GLsizei count, GLboolean transpose, GLfloat const* v)
  {
    glUniformMatrix2x3fv(index, count, transpose, v);
    EG_CHECK_ERROR;
  }
};
template <>
struct uniform_matrix_t<2, 4>
{
  static void
  apply(GLint index, GLsizei count, GLboolean transpose, GLfloat const* v)
  {
    glUniformMatrix2x4fv(index, count, transpose, v);
    EG_CHECK_ERROR;
  }
};
template <>
struct uniform_matrix_t<3, 2>
{
  static void
  apply(GLint index, GLsizei count, GLboolean transpose, GLfloat const* v)
  {
    glUniformMatrix3x2fv(index, count, transpose, v);
    EG_CHECK_ERROR;
  }
};
template <>
struct uniform_matrix_t<3, 3>
{
  static void
  apply(GLint index, GLsizei count, GLboolean transpose, GLfloat const* v)
  {
    glUniformMatrix3fv(index, count, transpose, v);
    EG_CHECK_ERROR;
  }
};
template <>
struct uniform_matrix_t<3, 4>
{
  static void
  apply(GLint index, GLsizei count, GLboolean transpose, GLfloat const* v)
  {
    glUniformMatrix3x4fv(index, count, transpose, v);
    EG_CHECK_ERROR;
  }
};
template <>
struct uniform_matrix_t<4, 2>
{
  static void
  apply(GLint index, GLsizei count, GLboolean transpose, GLfloat const* v)
  {
    glUniformMatrix4x2fv(index, count, transpose, v);
    EG_CHECK_ERROR;
  }
};
template <>
struct uniform_matrix_t<4, 3>
{
  static void
  apply(GLint index, GLsizei count, GLboolean transpose, GLfloat const* v)
  {
    glUniformMatrix4x3fv(index, count, transpose, v);
    EG_CHECK_ERROR;
  }
};
template <>
struct uniform_matrix_t<4, 4>
{
  static void
  apply(GLint index, GLsizei count, GLboolean transpose, GLfloat const* v)
  {
    glUniformMatrix4fv(index, count, transpose, v);
    EG_CHECK_ERROR;
  }
};
}
}
