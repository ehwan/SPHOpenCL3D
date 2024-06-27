#pragma once

#include <chrono>
#include <type_traits>

namespace eg
{
namespace time
{
using clock = std::chrono::system_clock;
using duration = clock::duration;
using uduration = std::chrono::duration<std::make_unsigned_t<duration::rep>,
                                        duration::period>;

template <typename Dur = uduration,
          typename Ratio = std::ratio<1>,
          typename Rep,
          typename Per>
constexpr Dur to_duration(std::chrono::duration<Rep, Per> const& dur)
{
  return std::chrono::duration_cast<Dur>(dur);
}
template <typename Dur = uduration, typename Ratio = std::ratio<1>, typename T>
constexpr Dur to_duration(T secs)
{
  return std::chrono::duration_cast<Dur>(std::chrono::duration<T, Ratio>(secs));
}

}
}
