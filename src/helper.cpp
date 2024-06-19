#include "helper.hpp"

std::unique_ptr<ExprAST> logErr(const char* str) {
  fprintf(stderr, "Error: %s", str);
  return nullptr;
}

std::unique_ptr<PrototypeAST> logErrP(const char* str) {
  logErr(str);
  return nullptr;
}

