#pragma once

#include <string>
#include <iostream>

class Logger {
public:
  enum class Level { ERROR, FATAL, WARNING };

  void log(Level level, std::string msg) {
    std::cout << msg << '\n';
  }
};
