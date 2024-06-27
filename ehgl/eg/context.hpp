#pragma once

#include "gl.hpp"
#include <vector>

namespace eg
{
namespace core
{

class Context
{
public:
  GLint integer(GLenum pname) const
  {
    GLint ret;
    glGetIntegerv(pname, &ret);
    EG_CHECK_ERROR;
    return ret;
  }
  const char* string(GLenum pname) const
  {
    const char* ret = reinterpret_cast<const char*>(glGetString(pname));
    EG_CHECK_ERROR;
    return ret;
  }

  int major_version() const
  {
    return integer(GL_MAJOR_VERSION);
  }
  int minor_version() const
  {
    return integer(GL_MINOR_VERSION);
  }
  const char* vendor() const
  {
    return string(GL_VENDOR);
  }
  const char* renderer() const
  {
    return string(GL_RENDERER);
  }
  unsigned int extension_size() const
  {
    return static_cast<unsigned int>(integer(GL_NUM_EXTENSIONS));
  }
  const char* extension(GLuint index) const
  {
    const char* ret
        = reinterpret_cast<const char*>(glGetStringi(GL_EXTENSIONS, index));
    EG_CHECK_ERROR;
    return ret;
  }
  std::vector<const char*> extensions() const
  {
    const unsigned int count = extension_size();
    std::vector<const char*> ret(count);
    for (unsigned int i = 0; i < count; ++i)
    {
      ret[i] = extension(i);
    }
    return ret;
  }

  const char* shading_version() const
  {
    return string(GL_SHADING_LANGUAGE_VERSION);
  }
};

}
}

namespace eg
{
constexpr core::Context context {};
}
