#pragma once

#include <exception>
#include <string>

/*
  Exception class for model engine.
*/
class ModelException : public std::exception
{
  std::string msg;
public:
  ModelException(const std::string& error_msg) : msg(error_msg) {}

  const char* what() const throw() { return msg.c_str(); }
};
