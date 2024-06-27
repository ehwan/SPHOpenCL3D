#pragma once

#include "loop.hpp"
#include <SFML/Window.hpp>
#include <functional>
#include <vector>

namespace eg
{
namespace debug
{

class window_context
{
protected:
  sf::Window window_;
  eg::loop_t loop_ { 1.0f / 60.0f };
  bool run_ { false };
  sf::Event eventdata_;

  std::function<void(window_context&)> enterframe_;
  std::function<void(window_context&)> event_;

public:
  window_context()
  {
  }
  window_context(unsigned int w,
                 unsigned int h,
                 const char* title,
                 unsigned int options = sf::Style::Default)
  {
    create(w, h, title, options);
  }
  void create(unsigned int w,
              unsigned int h,
              const char* title,
              unsigned int options = sf::Style::Default)
  {
    sf::ContextSettings context;
    context.depthBits = 24;
    context.stencilBits = 8;
    context.antialiasingLevel = 0;
    context.majorVersion = 4;
    context.minorVersion = 1;
    context.attributeFlags = context.Core;
    window_.create(sf::VideoMode(w, h), title, options, context);
    window_.setActive();
  }
  void close()
  {
    run_ = false;
  }
  void swap_buffer()
  {
    window_.display();
  }
  sf::Window& window()
  {
    return window_;
  }
  eg::loop_t& loop()
  {
    return loop_;
  }
  void set_enterframe(std::function<void(window_context&)> func)
  {
    enterframe_ = std::move(func);
  }
  void set_event(std::function<void(window_context&)> func)
  {
    event_ = std::move(func);
  }
  sf::Event& event()
  {
    return eventdata_;
  }
  sf::Event const& event() const
  {
    return eventdata_;
  }

  void run()
  {
    run_ = true;
    while (run_)
    {
      while (window_.pollEvent(eventdata_))
      {
        if (event_)
        {
          event_(*this);
        }
      }
      if (loop_())
      {
        if (enterframe_)
        {
          enterframe_(*this);
        }
      }
    }
  }
};

}
}
