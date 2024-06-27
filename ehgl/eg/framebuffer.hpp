#pragma once

#include "gl.hpp"
#include <algorithm>

namespace eg
{

class Framebuffer
{
  friend void swap(Framebuffer&, Framebuffer&);
  GLuint handler_;

  void release_if() const
  {
    if (handler_)
    {
      glDeleteFramebuffers(1, &handler_);
      EG_CHECK_ERROR;
    }
  }

public:
  EG_DEFINE_OBJECT_OPERATOR(Framebuffer, GLuint, 0, handler_)
};
inline void swap(Framebuffer& l, Framebuffer& r)
{
  std::swap(l.handler_, r.handler_);
}

inline Framebuffer make_framebuffer()
{
  GLuint ret;
  glGenFramebuffers(1, &ret);
  EG_CHECK_ERROR;
  return { ret };
}

}
