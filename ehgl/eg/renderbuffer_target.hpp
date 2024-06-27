#pragma once

#include "gl.hpp"

namespace eg
{
template <GLenum Target>
class RenderbufferTarget;

template <>
class RenderbufferTarget<GL_RENDERBUFFER>
{
public:
  constexpr GLenum target() const
  {
    return GL_RENDERBUFFER;
  }
  operator GLenum() const
  {
    return target();
  }
  GLuint get() const
  {
    GLint ret;
    glGetIntegerv(GL_RENDERBUFFER_BINDING, &ret);
    EG_CHECK_ERROR;
    return static_cast<GLuint>(ret);
  }
  GLint parameter(GLenum pname) const
  {
    GLint ret;
    glGetRenderbufferParameteriv(target(), pname, &ret);
    EG_CHECK_ERROR;
    return ret;
  }
  GLsizei width() const
  {
    GLint ret;
    glGetRenderbufferParameteriv(target(), GL_RENDERBUFFER_WIDTH, &ret);
    EG_CHECK_ERROR;
    return static_cast<GLsizei>(ret);
  }
  GLsizei height() const
  {
    GLint ret;
    glGetRenderbufferParameteriv(target(), GL_RENDERBUFFER_HEIGHT, &ret);
    EG_CHECK_ERROR;
    return static_cast<GLsizei>(ret);
  }
  GLenum internalFormat() const
  {
    GLint ret;
    glGetRenderbufferParameteriv(target(), GL_RENDERBUFFER_INTERNAL_FORMAT,
                                 &ret);
    EG_CHECK_ERROR;
    return static_cast<GLenum>(ret);
  }

  void bind(GLuint renderbuffer) const
  {
    glBindRenderbuffer(target(), renderbuffer);
    EG_CHECK_ERROR;
  }
  void unbind() const
  {
    glBindRenderbuffer(target(), 0);
    EG_CHECK_ERROR;
  }

  void data(GLenum internalFormat, GLsizei width, GLsizei height) const
  {
    glRenderbufferStorage(target(), internalFormat, width, height);
    EG_CHECK_ERROR;
  }
};

constexpr RenderbufferTarget<GL_RENDERBUFFER> renderbuffer {};
}
