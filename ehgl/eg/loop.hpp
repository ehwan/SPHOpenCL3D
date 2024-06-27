#pragma once

#include "time.hpp"

namespace eg
{

class loop_t
{
  time::uduration time_gap_;
  time::uduration gap_saved_;
  time::clock::time_point saved_time_;

public:
  template <typename T>
  loop_t(T&& time_gap)
      : time_gap_(time::to_duration(time_gap))
  {
    reset();
  }
  loop_t()
  {
    reset(time::uduration::zero());
  }

  void reset()
  {
    saved_time_ = time::clock::now();
    gap_saved_ = gap_saved_.zero();
  }
  template <typename Dur>
  void reset(Dur dur)
  {
    time_gap_ = time::to_duration(dur);
    reset();
  }
  time::uduration const& gap() const
  {
    return gap_saved_;
  }
  float dt() const
  {
    return std::chrono::duration_cast<
               std::chrono::duration<float, std::ratio<1>>>(gap_saved_)
        .count();
  }

  bool operator()()
  {
    const time::uduration gap = time::clock::now() - saved_time_;
    if (gap >= time_gap_)
    {
      saved_time_ = time::clock::now();
      gap_saved_ = gap;
      return true;
    }
    return false;
  }
};

}
