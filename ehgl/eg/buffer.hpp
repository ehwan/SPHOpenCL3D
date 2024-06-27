#pragma once

#include "gl.hpp"
#include <algorithm>

namespace eg
{

class Buffer
{
  friend void swap(Buffer&, Buffer&);
  GLuint handler_;

  void release_if() const
  {
    if (handler_)
    {
      glDeleteBuffers(1, &handler_);
    }
  }

public:
  EG_DEFINE_OBJECT_OPERATOR(Buffer, GLuint, 0, handler_)
};

inline void swap(Buffer& l, Buffer& r)
{
  std::swap(l.handler_, r.handler_);
}

inline Buffer make_buffer()
{
  GLuint ret;
  glGenBuffers(1, &ret);
  EG_CHECK_ERROR;
  return { ret };
}

}
