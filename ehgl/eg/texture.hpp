#pragma once

#include "gl.hpp"
#include <algorithm>

namespace eg
{

class Texture
{
  friend void swap(Texture&, Texture&);
  GLuint handler_;

  void release_if() const
  {
    if (handler_)
    {
      glDeleteTextures(1, &handler_);
      EG_CHECK_ERROR;
    }
  }

public:
  EG_DEFINE_OBJECT_OPERATOR(Texture, GLuint, 0, handler_)

  /// take real integer
  static void active(unsigned int N)
  {
    glActiveTexture(GL_TEXTURE0 + N);
    EG_CHECK_ERROR;
  }
  static unsigned int active()
  {
    GLint ret;
    glGetIntegerv(GL_ACTIVE_TEXTURE, &ret);
    EG_CHECK_ERROR;
    return static_cast<unsigned int>(ret) - GL_TEXTURE0;
  }
  static unsigned int max_combined_texture_units()
  {
    GLint ret;
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &ret);
    EG_CHECK_ERROR;
    return static_cast<unsigned int>(ret);
  }
};
inline void swap(Texture& l, Texture& r)
{
  std::swap(l.handler_, r.handler_);
}

inline Texture make_texture()
{
  GLuint ret;
  glGenTextures(1, &ret);
  EG_CHECK_ERROR;
  return { ret };
}

}
