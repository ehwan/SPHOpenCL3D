#pragma once

#include "eg/gl.hpp"
#include "eg/context.hpp"
#include "eg/buffer_target.hpp"
#include "eg/buffer.hpp"
#include "eg/framebuffer_target.hpp"
#include "eg/framebuffer.hpp"
#include "eg/program.hpp"
#include "eg/shader.hpp"
#include "eg/renderbuffer_target.hpp"
#include "eg/renderbuffer.hpp"
#include "eg/texture_target.hpp"
#include "eg/texture.hpp"
#include "eg/vertex_array.hpp"
#include "eg/vertex_attrib.hpp"
#include "eg/uniform.hpp"
#include "eg/transform_feedback.hpp"
#include "eg/query.hpp"
#include "eg/sync.hpp"

#include "eg/loop.hpp"
#include "eg/math.hpp"

#include "eg/debug.hpp"
#include "eg/debug_window.hpp"

#include "eg/util.hpp"

#undef EG_DEFINE_OBJECT_OPERATOR
#undef EG_GL_VERSION

namespace eg
{

inline VertexAttrib Program::vertexAttrib( const char* name ) const
{
  const GLint ret = glGetAttribLocation( get() , name );
  return { static_cast< GLuint >( ret ) };
}
inline Uniform Program::uniformLocation( const char* name ) const
{
  return { glGetUniformLocation( get() , name ) };
}

}
