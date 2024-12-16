#pragma once
#include "ast.hpp"
#include "token.hpp"

#include <string>
#include <fstream>
#include <unordered_map>


class Parser
{
private:
  std::string m_identifierStr;
  int m_numVal;
  char m_lastChar;
  std::ifstream m_file;
  bool m_eofReached;
  char m_nextChar;
  int m_nextToken;
  std::unordered_map<int, int> m_binopPrecedence;
  std::string m_source;
  int m_curr_idx;
  Token m_curr_token;

public:
  Parser();
  Parser(std::string& filename);
  ~Parser() = default;

  int getTokPrec();

  std::unique_ptr<ExprAST> parseParenExpr();
  std::unique_ptr<ExprAST> parseIdentifierExpr();
  std::unique_ptr<ExprAST> parseNumberExpr();
  std::unique_ptr<ExprAST> parsePrimary();
  std::unique_ptr<ExprAST> parseBinOpRHS(int exprPrec, std::unique_ptr<ExprAST> lhs);
  std::unique_ptr<ExprAST> parseExpression();
  std::unique_ptr<FuncAST> parseDefinition();
  std::unique_ptr<PrototypeAST> parseDeclaration();
  std::unique_ptr<PrototypeAST> parsePrototype();
  std::unique_ptr<FuncAST> parseTopLevel();
  
  void read_line(); // repl
  std::string get_source();
  void skip_whitespace();
  int scan_int();
  void parse();

  void read_file(const char* fileName);

  Token getCurrToken();
  Token advanceToken();
};
