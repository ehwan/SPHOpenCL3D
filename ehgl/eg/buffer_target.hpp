#pragma once

#include "gl.hpp"
#include <memory>

#ifndef EG_ACCESS_BIT_DEFINED
  #define EG_ACCESS_BIT_DEFINED
namespace eg
{
namespace core
{
struct read_only_t
{
};
struct write_only_t
{
};
struct read_write_t
{
};
}
}

namespace eg
{
constexpr core::read_only_t read_only {};
constexpr core::write_only_t write_only {};
constexpr core::read_write_t read_write {};
}
#endif

namespace eg
{
namespace core
{
template <GLenum>
struct buffer_target_binding_t;

struct unmap_deleter_t
{
  GLenum target_;

  unmap_deleter_t(GLenum target)
      : target_(target)
  {
  }

  void operator()(void const*) const
  {
    glUnmapBuffer(target_);
    EG_CHECK_ERROR;
  }
};

template <typename Crtp>
class BufferTargetBase
{
  constexpr GLenum target() const
  {
    return static_cast<Crtp const&>(*this).target();
  }

protected:
  GLint parameter(GLenum param) const
  {
    GLint ret;
    glGetBufferParameteriv(target(), param, &ret);
    EG_CHECK_ERROR;
    return ret;
  }
  void bind(GLuint buffer, GLuint bindindex) const
  {
    glBindBufferBase(target(), bindindex, buffer);
    EG_CHECK_ERROR;
  }
  void
  bind(GLuint buffer, GLuint bindindex, GLintptr offset, GLsizeiptr size) const
  {
    glBindBufferRange(target(), bindindex, buffer, offset, size);
    EG_CHECK_ERROR;
  }

public:
  template <typename = void>
  GLuint get() const
  {
    GLint ret;
    glGetIntegerv(buffer_target_binding_t<target()>::value, &ret);
    EG_CHECK_ERROR;
    return static_cast<GLuint>(ret);
  }
  operator GLenum() const
  {
    return target();
  }
  GLenum usage() const
  {
    return static_cast<GLenum>(parameter(GL_BUFFER_USAGE));
  }
  GLsizei size() const
  {
    return static_cast<GLsizei>(parameter(GL_BUFFER_SIZE));
  }
  bool isMapped() const
  {
    return parameter(GL_BUFFER_MAPPED) == GL_TRUE;
  }

  void bind(GLuint buffer) const
  {
    glBindBuffer(target(), buffer);
    EG_CHECK_ERROR;
  }
  void unbind() const
  {
    glBindBuffer(target(), 0);
    EG_CHECK_ERROR;
  }

  void data(GLenum usage, GLsizei size, void const* data) const
  {
    glBufferData(target(), size, data, usage);
    EG_CHECK_ERROR;
  }
  template <typename T, size_t N>
  void data(GLenum usage, T const (&data)[N]) const
  {
    this->data(usage, sizeof(T) * N, data);
  }
  void subData(GLintptr offset, GLsizeiptr size, void const* data) const
  {
    glBufferSubData(target(), offset, size, data);
    EG_CHECK_ERROR;
  }
  template <typename T, size_t N>
  void subData(GLintptr offset, T const (&data)[N]) const
  {
    subData(offset, sizeof(T) * N, data);
  }
  template <typename T, size_t N>
  void subData(T const (&data)[N]) const
  {
    subData(0, data);
  }
  void copySubData(GLenum readTarget,
                   GLintptr readOffset,
                   GLintptr writeOffset,
                   GLsizeiptr size) const
  {
    glCopyBufferSubData(readTarget, target(), readOffset, writeOffset, size);
    EG_CHECK_ERROR;
  }
  void read(GLintptr offset, GLsizeiptr size, void* data) const
  {
    glGetBufferSubData(target(), offset, size, data);
    EG_CHECK_ERROR;
  }
  void read(GLsizeiptr size, void* data) const
  {
    read(0, size, data);
  }

  template <typename T = void>
  std::unique_ptr<T const, unmap_deleter_t> map(read_only_t) const
  {
    auto ret = std::unique_ptr<T const, unmap_deleter_t>(
        reinterpret_cast<T const*>(glMapBuffer(target(), GL_READ_ONLY)),
        unmap_deleter_t(target()));
    EG_CHECK_ERROR;
    return ret;
  }
  template <typename T = void>
  std::unique_ptr<T, unmap_deleter_t> map(write_only_t) const
  {
    auto ret = std::unique_ptr<T, unmap_deleter_t>(
        reinterpret_cast<T*>(glMapBuffer(target(), GL_WRITE_ONLY)),
        unmap_deleter_t(target()));
    EG_CHECK_ERROR;
    return ret;
  }
  template <typename T = void>
  std::unique_ptr<T, unmap_deleter_t> map(read_write_t) const
  {
    auto ret = std::unique_ptr<T, unmap_deleter_t>(
        reinterpret_cast<T*>(glMapBuffer(target(), GL_READ_WRITE)),
        unmap_deleter_t(target()));
    EG_CHECK_ERROR;
    return ret;
  }
  template <typename T = void>
  std::unique_ptr<T, unmap_deleter_t> map() const
  {
    return map<T>(core::read_write_t());
  }
};
}
}

namespace eg
{

template <GLenum Target>
class BufferTarget : public core::BufferTargetBase<BufferTarget<Target>>
{
public:
  constexpr GLenum target() const
  {
    return Target;
  }
};
template <>
class BufferTarget<GL_ELEMENT_ARRAY_BUFFER>
    : public core::BufferTargetBase<BufferTarget<GL_ELEMENT_ARRAY_BUFFER>>
{
  using parent = core::BufferTargetBase<BufferTarget<GL_ELEMENT_ARRAY_BUFFER>>;

public:
  constexpr GLenum target() const
  {
    return GL_ELEMENT_ARRAY_BUFFER;
  }
  // count limit
  GLuint max_elements_indices() const
  {
    return static_cast<GLuint>(parent::parameter(GL_MAX_ELEMENTS_INDICES));
  }
  // end - start + 1 limit
  GLuint max_elements_vertices() const
  {
    return static_cast<GLuint>(parent::parameter(GL_MAX_ELEMENTS_VERTICES));
  }
};

template <>
class BufferTarget<GL_UNIFORM_BUFFER>
    : public core::BufferTargetBase<BufferTarget<GL_UNIFORM_BUFFER>>
{
  using parent = core::BufferTargetBase<BufferTarget<GL_UNIFORM_BUFFER>>;

public:
  using parent::bind;
  constexpr GLenum target() const
  {
    return GL_UNIFORM_BUFFER;
  }
};
template <>
class BufferTarget<GL_TRANSFORM_FEEDBACK_BUFFER>
    : public core::BufferTargetBase<BufferTarget<GL_TRANSFORM_FEEDBACK_BUFFER>>
{
  using parent
      = core::BufferTargetBase<BufferTarget<GL_TRANSFORM_FEEDBACK_BUFFER>>;

public:
  using parent::bind;
  constexpr GLenum target() const
  {
    return GL_TRANSFORM_FEEDBACK_BUFFER;
  }
};
#if EG_GL_VERSION >= 400 || defined(EG_ARB_DRAW_INDIRECT)
constexpr BufferTarget<GL_DRAW_INDIRECT_BUFFER> draw_indirect_buffer;
#endif
#if EG_GL_VERSION >= 420 || defined(EG_ARB_SHADER_ATOMIC_COUNTERS)
template <>
class BufferTarget<GL_ATOMIC_COUNTER_BUFFER>
    : public core::BufferTargetBase<BufferTarget<GL_ATOMIC_COUNTER_BUFFER>>
{
  using parent = core::BufferTargetBase<BufferTarget<GL_ATOMIC_COUNTER_BUFFER>>;

public:
  using parent::bind;
  constexpr GLenum target() const
  {
    return GL_ATOMIC_COUNTER_BUFFER;
  }
};
constexpr BufferTarget<GL_ATOMIC_COUNTER_BUFFER> atomic_counter_buffer {};
#endif

#if EG_GL_VERSION >= 430 || defined(EG_ARB_SHADER_STORAGE_BUFFER_OBJECT)
template <>
class BufferTarget<GL_SHADER_STORAGE_BUFFER>
    : public core::BufferTargetBase<BufferTarget<GL_SHADER_STORAGE_BUFFER>>
{
  using parent = core::BufferTargetBase<BufferTarget<GL_SHADER_STORAGE_BUFFER>>;

public:
  using parent::bind;
  constexpr GLenum target() const
  {
    return GL_SHADER_STORAGE_BUFFER;
  }
};
constexpr BufferTarget<GL_SHADER_STORAGE_BUFFER> shader_storage_buffer {};
#endif

#if EG_GL_VERSION >= 440 || defined(EG_ARB_QUERY_BUFFER_OBJECT)
constexpr BufferTarget<GL_QUERY_BUFFER> query_buffer {};
#endif

constexpr BufferTarget<GL_ARRAY_BUFFER> array_buffer {};
constexpr BufferTarget<GL_ELEMENT_ARRAY_BUFFER> element_array_buffer {};
constexpr BufferTarget<GL_UNIFORM_BUFFER> uniform_buffer {};
constexpr BufferTarget<GL_TRANSFORM_FEEDBACK_BUFFER>
    transform_feedback_buffer {};
constexpr BufferTarget<GL_PIXEL_PACK_BUFFER> pixel_pack_buffer {};
constexpr BufferTarget<GL_PIXEL_UNPACK_BUFFER> pixel_unpack_buffer {};
constexpr BufferTarget<GL_COPY_READ_BUFFER> copy_read_buffer {};
constexpr BufferTarget<GL_COPY_WRITE_BUFFER> copy_write_buffer {};

}

namespace eg
{
namespace core
{
#define EG_BUFFER_BINDING_ENUM(a, b)   \
  template <>                          \
  struct buffer_target_binding_t<a>    \
  {                                    \
    constexpr static GLenum value = b; \
  }
EG_BUFFER_BINDING_ENUM(GL_ARRAY_BUFFER, GL_ARRAY_BUFFER_BINDING);
EG_BUFFER_BINDING_ENUM(GL_ELEMENT_ARRAY_BUFFER,
                       GL_ELEMENT_ARRAY_BUFFER_BINDING);
EG_BUFFER_BINDING_ENUM(GL_UNIFORM_BUFFER, GL_UNIFORM_BUFFER_BINDING);
EG_BUFFER_BINDING_ENUM(GL_TRANSFORM_FEEDBACK_BUFFER,
                       GL_TRANSFORM_FEEDBACK_BUFFER);
EG_BUFFER_BINDING_ENUM(GL_PIXEL_PACK_BUFFER, GL_PIXEL_PACK_BUFFER_BINDING);
EG_BUFFER_BINDING_ENUM(GL_PIXEL_UNPACK_BUFFER, GL_PIXEL_UNPACK_BUFFER_BINDING);
// EG_BUFFER_BINDING_ENUM( GL_COPY_READ_BUFFER , GL_COPY_READ_BUFFER_BINDING );
// EG_BUFFER_BINDING_ENUM( GL_COPY_WRITE_BUFFER , GL_COPY_WRITE_BUFFER_BINDING
// );

#if EG_GL_VERSION >= 400 || defined(EG_ARB_DRAW_INDIRECT)
EG_BUFFER_BINDING_ENUM(GL_DRAW_INDIRECT_BUFFER,
                       GL_DRAW_INDIRECT_BUFFER_BINDING);
#endif
#if EG_GL_VERSION >= 420 || defined(EG_ARB_SHADER_ATOMIC_COUNTERS)
EG_BUFFER_BINDING_ENUM(GL_ATOMIC_COUNTER_BUFFER,
                       GL_ATOMIC_COUNTER_BUFFER_BINDING);
#endif
#if EG_GL_VERSION >= 430 || defined(EG_ARB_SHADER_STORAGE_BUFFER_OBJECT)
EG_BUFFER_BINDING_ENUM(GL_SHADER_STORAGE_BUFFER,
                       GL_SHADER_STORAGE_BUFFER_BINDING);
#endif

#if EG_GL_VERSION >= 440 || defined(EG_ARB_QUERY_BUFFER_OBJECT)
EG_BUFFER_BINDING_ENUM(GL_QUERY_BUFFER, GL_QUERY_BUFFER_BINDING);
#endif

#undef EG_BUFFER_BINDING_ENUM
}
}
