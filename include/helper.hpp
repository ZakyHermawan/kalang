#include "ast.hpp"
#include <memory>

std::unique_ptr<ExprAST> logErr(const char* str);
std::unique_ptr<PrototypeAST> logErrP(const char* str);

