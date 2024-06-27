#pragma once

#include "shader.hpp"
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace eg
{

inline std::vector<char> load_file(char const* filename)
{
  std::ifstream stream(filename);
  stream.seekg(0, stream.end);
  size_t len = stream.tellg();
  std::vector<char> buf(len + 1);
  buf.back() = (char)0;
  stream.seekg(0, stream.beg);
  stream.read(buf.data(), len);
  return buf;
}

inline Shader load_shader(const char* filename, GLenum shader_type)
{
  Shader sh = eg::make_shader(shader_type);
  auto buf = load_file(filename);
  sh.source(buf.data());
  if (sh.compile() == false)
  {
    throw std::runtime_error(sh.errorMessage());
  }
  return sh;
}

}