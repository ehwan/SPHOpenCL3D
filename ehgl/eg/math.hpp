#pragma once

#include <Eigen/Dense>
#include <cmath>
#include <memory>

namespace eg
{

class Eye
{
  Eigen::Vector3f axis_[3];
  Eigen::Vector3f position_;
  std::unique_ptr<Eigen::Matrix4f> matrix_;
  bool cached_;

public:
  Eye()
  {
    axis_[0] = Eigen::Vector3f::UnitX();
    axis_[1] = Eigen::Vector3f::UnitY();
    axis_[2] = Eigen::Vector3f::UnitZ();
    position_.setZero();
    matrix_.reset(new Eigen::Matrix4f);
    reset_cache();
  }

  template <typename Mat>
  void cache(Mat& mat) const
  {
    // out = axis dot ( in - position )
    Eigen::Matrix3f trns;
    trns << axis_[0].transpose(), axis_[1].transpose(), axis_[2].transpose();

    mat << trns, -trns * position_, 0, 0, 0, 1;
  }
  void cache()
  {
    cache(*matrix_);
    cached_ = true;
  }
  void reset_cache()
  {
    cached_ = false;
  }

  Eigen::Vector3f const& axis(unsigned int i) const
  {
    return axis_[i];
  }
  Eigen::Vector3f const& position() const
  {
    return position_;
  }
  void position(Eigen::Vector3f const& p)
  {
    position_ = p;
    reset_cache();
  }
  void move(unsigned int i, float factor)
  {
    position_ += axis_[i] * factor;
    reset_cache();
  }

  void set(Eigen::Vector3f const& x,
           Eigen::Vector3f const& y,
           Eigen::Vector3f const& z)
  {
    axis_[0] = x;
    axis_[1] = y;
    axis_[2] = z;
    reset_cache();
  }
  void look(Eigen::Vector3f const& to, Eigen::Vector3f const& up)
  {
    // z axis
    axis_[2] = -to;

    // x = y cross z
    axis_[0] = up.cross(axis_[2]).normalized();

    // y = z cross x
    axis_[1] = axis_[2].cross(axis_[0]);
    reset_cache();
  }
  void look(Eigen::Vector3f const& to)
  {
    look(to, Eigen::Vector3f::UnitY());
  }

  Eigen::Matrix4f& operator()()
  {
    if (cached_ == false)
    {
      cache();
    }
    return *matrix_;
  }
  Eigen::Matrix4f operator()() const
  {
    Eigen::Matrix4f ret;
    cache(ret);
    return ret;
  }
};
class EyeAngle : public Eye
{
  Eigen::Array3f angle_;

public:
  EyeAngle()
  {
    angle_.setZero();
  }

  Eigen::Array3f const& angle() const
  {
    return angle_;
  }
  void angle(Eigen::Array3f const& arr)
  {
    angle_.x() = std::min(1.57079632679f, std::max(arr.x(), -1.57079632679f));
    angle_.y() = std::fmod(arr.y(), 6.28318530718f);
    angle_.z() = std::min(3.14159265359f, std::max(arr.z(), -3.14159265359f));

    const Eigen::Array3f cos = angle_.cos();
    const Eigen::Array3f sin = angle_.sin();

    const Eigen::Vector3f axis1 { cos.y() * cos.z(), cos.y() * sin.z(),
                                  -sin.y() };
    const Eigen::Vector3f axis3 { cos.x() * sin.y(), -sin.x(),
                                  cos.x() * cos.y() };
    set(axis1, axis3.cross(axis1), axis3);
  }
};
class Perspective
{
  Eigen::Array3f min_, max_;
  std::unique_ptr<Eigen::Matrix4f> matrix_;
  bool cached_;

public:
  Perspective()
  {
    matrix_.reset(new Eigen::Matrix4f);
    reset_cache();
  }
  float near() const
  {
    return min_.z();
  }
  float far() const
  {
    return max_.z();
  }
  template <typename Mat>
  void cache(Mat& mat) const
  {
    const Eigen::Array3f sizeI = 1.0f / (max_ - min_);
    const Eigen::Array3f add_div = (min_ + max_) * sizeI;
    mat.setZero();
    mat(0, 0) = 2 * near() * sizeI.x();
    mat(1, 1) = 2 * near() * sizeI.y();
    mat(0, 2) = add_div.x();
    mat(1, 2) = add_div.y();
    mat(2, 2) = -add_div.z();
    mat(3, 2) = -1;
    mat(2, 3) = -2 * near() * far() * sizeI.z();
  }
  void cache()
  {
    cache(*matrix_);
    cached_ = true;
  }
  void reset_cache()
  {
    cached_ = false;
  }
  void frustum(Eigen::Array2f const& min,
               Eigen::Array2f const& max,
               float near,
               float far)
  {
    min_ << min, near;
    max_ << max, far;
    reset_cache();
  }
  /// in radians , aspect w/h
  void perspective(const float view_angle,
                   const float aspect,
                   const float near,
                   const float far)
  {
    const float hh = std::tan(view_angle * 0.5f) * near;
    const float hw = hh * aspect;

    min_ << -hw, -hh, near;
    max_ << hw, hh, far;
    reset_cache();
  }

  Eigen::Matrix4f& operator()()
  {
    if (cached_ == false)
    {
      cache();
    }
    return *matrix_;
  }
  Eigen::Matrix4f operator()() const
  {
    Eigen::Matrix4f ret;
    cache(ret);
    return ret;
  }
};
template <unsigned int N>
class Ortholog
{
  using Arr = Eigen::Array<float, N, 1>;
  using Vec = Eigen::Matrix<float, N, 1>;
  using Mat = Eigen::Matrix<float, N + 1, N + 1>;
  using Mat4 = Eigen::Matrix4f;

  Arr min_, max_;

public:
  void set(Arr const& min, Arr const& max)
  {
    min_ = min;
    max_ = max;
  }

  template <typename Mat_>
  void cache(Mat_& mat)
  {
    const Arr sizeI = 1.0f / (max_ - min_);

    mat.setZero();
    mat.diagonal().template head<N>() = 2 * sizeI;
    mat.template topRightCorner<N, 1>() = -(min_ + max_) * sizeI;
    mat.template bottomRightCorner<1, 1>() << 1;
  }
  template <typename Mat_ = Mat>
  Mat_ matrix() const
  {
    Mat_ ret;
    cache(ret);
    return ret;
  }
};

}
