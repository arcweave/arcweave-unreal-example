#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <exception>

namespace Arcweave {
  class RuntimeErrorException : public std::exception {
    public:
    std::string message;
    size_t line = -1;
    size_t charPositionInLine = -1;
    RuntimeErrorException(std::string msg) {
      message = msg;
    };
    RuntimeErrorException(std::string msg, size_t _line, size_t _charPositionInLine) {
      message = msg;
      line = _line;
      charPositionInLine = _charPositionInLine;
    };
    std::string what() {
      if (line > -1) {
        std::ostringstream oss;
        oss << "line " << line << ":" << charPositionInLine << " " << message << std::endl;
        return oss.str().c_str();
      }
      return message;
    }
  };

  class ParseErrorException : public std::exception {
    public:
    std::string message;
    size_t line = -1;
    size_t charPositionInLine = -1;
    ParseErrorException(std::string msg) {
      message = msg;
    };
    ParseErrorException(std::string msg, size_t _line, size_t _charPositionInLine) {
      message = msg;
      line = _line;
      charPositionInLine = _charPositionInLine;
    };
    std::string what() {
      if (line > -1) {
        std::ostringstream oss;
        oss << "line " << line << ":" << charPositionInLine << " " << message << std::endl;
        return oss.str().c_str();
      }
      return message;
    }
  };
}