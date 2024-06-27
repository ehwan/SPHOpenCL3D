#pragma once

#include "gl.hpp"
#include <algorithm>

#if EG_GL_VERSION >= 400 || defined(EG_ARB_TRANSFORM_FEEDBACK2)
namespace eg
{
class TransformFeedback
{
  friend void swap(TransformFeedback&, TransformFeedback&);
  GLuint handler_;

  void release_if() const
  {
    if (handler_)
    {
      glDeleteTransformFeedbacks(1, &handler_);
      EG_CHECK_ERROR;
    }
  }

public:
  EG_DEFINE_OBJECT_OPERATOR(TransformFeedback, GLuint, 0, handler_)

  void bind() const
  {
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, get());
    EG_CHECK_ERROR;
  }
  void unbind() const
  {
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
    EG_CHECK_ERROR;
  }

  void pause() const
  {
    glPauseTransformFeedback();
    EG_CHECK_ERROR;
  }
  void resume() const
  {
    glResumeTransformFeedback();
    EG_CHECK_ERROR;
  }
  void draw(GLenum mode) const
  {
    glDrawTransformFeedback(mode, get());
    EG_CHECK_ERROR;
  }
  void draw_instanced(GLenum mode, size_t instance_count) const
  {
    glDrawTransformFeedbackInstanced(mode, get(), instance_count);
    EG_CHECK_ERROR;
  }
  void begin(GLenum mode) const
  {
    glBeginTransformFeedback(mode);
    EG_CHECK_ERROR;
  }
  void end() const
  {
    glEndTransformFeedback();
    EG_CHECK_ERROR;
  }
};

inline void swap(TransformFeedback& l, TransformFeedback& r)
{
  std::swap(l.handler_, r.handler_);
}
inline TransformFeedback make_transform_feedback()
{
  GLuint ret;
  glGenTransformFeedbacks(1, &ret);
  EG_CHECK_ERROR;
  return { ret };
}
}
#endif
