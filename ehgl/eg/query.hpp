#pragma once

#include "gl.hpp"
#include <algorithm>

namespace eg
{
namespace core
{
template <typename T>
struct get_query_result_t;
template <>
struct get_query_result_t<GLint>
{
  static GLint apply(GLuint handler, GLenum pname)
  {
    GLint ret;
    glGetQueryObjectiv(handler, pname, &ret);
    EG_CHECK_ERROR;
    return ret;
  }
};
template <>
struct get_query_result_t<GLuint>
{
  static GLuint apply(GLuint handler, GLenum pname)
  {
    GLuint ret;
    glGetQueryObjectuiv(handler, pname, &ret);
    EG_CHECK_ERROR;
    return ret;
  }
};
}
}

namespace eg
{

class Query
{
  friend void swap(Query&, Query&);
  GLuint handler_;

  void release_if() const
  {
    if (handler_)
    {
      glDeleteQueries(1, &handler_);
      EG_CHECK_ERROR;
    }
  }

public:
  EG_DEFINE_OBJECT_OPERATOR(Query, GLuint, 0, handler_)

  constexpr static GLenum primitive_generated = GL_PRIMITIVES_GENERATED;
  constexpr static GLenum transform_feedback_written
      = GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN;

  void bind(GLenum target) const
  {
    glBeginQuery(target, get());
    EG_CHECK_ERROR;
  }
  void unbind(GLenum target) const
  {
    glEndQuery(target);
    EG_CHECK_ERROR;
  }
  bool available() const
  {
    GLint ret;
    glGetQueryObjectiv(get(), GL_QUERY_RESULT_AVAILABLE, &ret);
    EG_CHECK_ERROR;
    return ret == GL_TRUE;
  }
  template <typename T = GLint>
  T result() const
  {
    return core::get_query_result_t<T>::apply(get(), GL_QUERY_RESULT);
  }
#if EG_GL_VERSION >= 440 || defined(EG_ARB_QUERY_BUFFER_OBJECT)
  template <typename T = GLint>
  T result_nowait() const
  {
    return core::get_query_result_t<T>::apply(get(), GL_QUERY_RESULT_NO_WAIT);
  }
#endif
};
inline void swap(Query& l, Query& r)
{
  std::swap(l.handler_, r.handler_);
}
inline Query make_query()
{
  GLuint ret;
  glGenQueries(1, &ret);
  EG_CHECK_ERROR;
  return { ret };
}

}
