#pragma once

// include header for Apple
#if defined(__APPLE__) || defined(__MACOSX)
  #define GL_SILENCE_DEPRECATION
  #include <OpenGL/gl3.h>
#else
// include header for Linux
#endif
#include <exception>
#include <iostream>

#define EG_DEFINE_OBJECT_OPERATOR(class_name, handler_type, default_val, \
                                  handler_name)                          \
  handler_type get() const                                               \
  {                                                                      \
    return handler_name;                                                 \
  }                                                                      \
  operator handler_type() const                                          \
  {                                                                      \
    return get();                                                        \
  }                                                                      \
  class_name(handler_type handler = default_val)                         \
      : handler_name(handler)                                            \
  {                                                                      \
  }                                                                      \
  class_name(class_name const&) = delete;                                \
  class_name& operator=(class_name const&) = delete;                     \
  class_name(class_name&& rhs)                                           \
      : handler_name(rhs.handler_name)                                   \
  {                                                                      \
    rhs.handler_name = default_val;                                      \
  }                                                                      \
  class_name& operator=(class_name&& rhs)                                \
  {                                                                      \
    release_if();                                                        \
    handler_name = rhs.handler_name;                                     \
    rhs.handler_name = default_val;                                      \
    return *this;                                                        \
  }                                                                      \
  ~class_name()                                                          \
  {                                                                      \
    release_if();                                                        \
  }

#define EG_THROW_EXCEPTION

#define EG_CHECK_ERROR                       \
  while (auto __eg_err_var__ = glGetError()) \
  {                                          \
    eg::throw_exception(__eg_err_var__);     \
  }

namespace eg
{

class exception : public std::exception
{
  int err_;

public:
  exception(int err)
      : err_(err)
  {
  }

  int error_code() const
  {
    return err_;
  }
  const char* what() const noexcept override
  {
    switch (err_)
    {
    case GL_NO_ERROR:
      return "GL_NO_ERROR";
    case GL_INVALID_ENUM:
      return "GL_INVALID_ENUM";
    case GL_INVALID_VALUE:
      return "GL_INVALID_VALUE";
    case GL_INVALID_OPERATION:
      return "GL_INVALID_OPERATION";
    case GL_OUT_OF_MEMORY:
      return "GL_OUT_OF_MEMORY";
    default:
      return "UNKNOWN_GL_ERROR";
    }
  }
};

static inline void throw_exception(int err)
{
  throw eg::exception(err);
}

}

#define EG_GL_VERSION 330
// #define EG_ARB_SHADER_STORAGE_BUFFER_OBJECT
// #deifne EG_ARB_COMPUTE_SHADER
// #define EG_ARB_BASE_INSTANCE
// #define EG_ARB_SHADER_ATOMIC_COUNTERS
// #define EG_ARB_DRAW_INDIRECT
// #define EG_ARB_TRANSFORM_FEEDBACK2
// #define EG_ARB_QUERY_BUFFER_OBJECT
// #define EG_ARB_TESSELLATION_SHADER
