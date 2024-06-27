#pragma once

#include "gl.hpp"
#include <algorithm>

namespace eg
{

class VertexArray
{
  friend void swap(VertexArray&, VertexArray&);
  GLuint handler_;

  void release_if() const
  {
    if (handler_)
    {
      glDeleteVertexArrays(1, &handler_);
      EG_CHECK_ERROR;
    }
  }

public:
  EG_DEFINE_OBJECT_OPERATOR(VertexArray, GLuint, 0, handler_)

  void bind() const
  {
    glBindVertexArray(get());
    EG_CHECK_ERROR;
  }
  void unbind() const
  {
    glBindVertexArray(0);
    EG_CHECK_ERROR;
  }
};
inline void swap(VertexArray& l, VertexArray& r)
{
  std::swap(l.handler_, r.handler_);
}

inline VertexArray make_vertex_array()
{
  GLuint ret;
  glGenVertexArrays(1, &ret);
  EG_CHECK_ERROR;
  return { ret };
}

}
