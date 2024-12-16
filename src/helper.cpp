#include "helper.hpp"

#include <memory>
#include <stdexcept>

std::unique_ptr<ExprAST> logErr(const char* str, ErrorCode errCode) {
  if(errCode == syntaxErr)
  {
    fprintf(stderr, "Syntax Error: %s\n", str);
  }
  else
  {
    fprintf(stderr, "Error: %s\n", str);
  }
  return nullptr;
}

std::unique_ptr<PrototypeAST> logErrP(const char* str, ErrorCode errCode) {
  fprintf(stderr, "Error on function prototype!\n");
  logErr(str, errCode);
  return nullptr;
}

template<typename... Args>
std::string string_format(const std::string& format, Args... args)
{
  int size_s = std::snprintf(nullptr, 0, format.c_str(), args...) + 1; // extra space for '\0'
  if(size_s <= 0)
  {
    throw std::runtime_error("Error during formatting");
  }
  auto size = static_cast<size_t>(size_s);
  std::unique_ptr<char[]>buff(new char[size]);
  std::snprintf(buff.get(), size, format.c_str(), args...);
  return std::string(buff.get(), buff.get() + size - 1); // ignore '\0' at the end
}
