#include "ast.hpp"
#include <memory>

enum ErrorCode
{
  syntaxErr,
};

std::unique_ptr<ExprAST> logErr(const char* str, ErrorCode errCode);
std::unique_ptr<PrototypeAST> logErrP(const char* str, ErrorCode errCode);

template<typename ... Args>
std::string string_format(const std::string& format, Args ... args);
