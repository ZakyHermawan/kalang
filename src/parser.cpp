#include "parser.hpp"
#include "helper.hpp"
#include "ast.hpp"
#include "token.hpp"
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <iostream>
#include <cmath>
#include <sstream>
#include <fstream>

#include "runtime.hpp"

Parser::Parser()
  : m_eofReached(0), m_nextToken(tok_start), m_nextChar(' '),
  m_curr_idx(0), m_curr_token(tok_start)
{
  m_binopPrecedence[tok_less] = 10;
  m_binopPrecedence[tok_plus] = 20;
  m_binopPrecedence[tok_min]  = 20;
  m_binopPrecedence[tok_mult] = 40;
}

std::unique_ptr<PrototypeAST> Parser::parsePrototype()
{
  if(m_curr_token != tok_identifier)
  {
    printf("Error: Expected function name in prototype\n");
    return nullptr;
  }
  std::string functionName = m_identifierStr;
  advanceToken(); // consume function name
  if(m_curr_token != tok_open_paren)
  {
    printf("Error: Expected ( after function name in prototype\n");
    return nullptr;
  }
  advanceToken(); // consume (
  std::vector<std::string> argNames;
  while(m_curr_token == tok_identifier)
  {
    argNames.push_back(m_identifierStr);
    advanceToken(); // consume argument name
    if(m_curr_token == tok_comma)
    {
      advanceToken();
    }
  }
  if(m_curr_token != tok_close_paren)
  {
    printf("Error: Expected ) after function name in prototype\n");
    return nullptr;
  }
  advanceToken(); // consume )

  return std::make_unique<PrototypeAST>(functionName, std::move(argNames));
}

int Parser::getTokPrec()
{
  if(!isascii(m_curr_token))
  {
    printf("Could not accept non ascii operator\n");
    return -1;
  }
  int prec = m_binopPrecedence[m_curr_token];
  if(prec <= 0)
  {
    return -1;
  }
  return prec;
}

std::unique_ptr<ExprAST> Parser::parseParenExpr()
{
  advanceToken(); // consume (
  auto expr = parseExpression();
  if(nullptr == expr)
  {
    return nullptr;
  }

  if(m_curr_token != tok_close_paren)
  {
    printf("Error: Expected )\n");
    return nullptr;
  }
  advanceToken(); // consume )
  return std::move(expr);
}

// identifierExpr := identifier | identifier '(' expression* ')'
std::unique_ptr<ExprAST> Parser::parseIdentifierExpr()
{
  std::string identifier = m_identifierStr;
  advanceToken(); // consume identifier

  if(m_curr_token != tok_open_paren) // just identifier
  {
    return std::make_unique<VariableExprAST>(identifier);
  }

  advanceToken(); // consume (
  std::vector<std::unique_ptr<ExprAST>> args;
  if(m_curr_token != tok_close_paren)
  {
    while(1)
    {
      auto arg = parseExpression();
      if(arg)
      {
        args.push_back(std::move(arg));
      }
      else
      {
        return nullptr;
      }

      if(m_curr_token == tok_close_paren)
      {
        break;
      }
      if(m_curr_token != tok_comma)
      {
        printf("Error: Expected after comma after argument\n");
        return nullptr;
      }
      advanceToken(); // eat ,
    }
  }
  advanceToken(); // consume )
  return std::make_unique<CallExprAST>(identifier, std::move(args));
}

std::unique_ptr<ExprAST> Parser::parseNumberExpr()
{
  auto numberExpr = std::make_unique<NumberExprAST>(m_numVal);
  advanceToken(); // consume number
  return std::move(numberExpr);
}

std::unique_ptr<ExprAST> Parser::parsePrimary()
{
  switch(m_curr_token)
  {
    case tok_identifier:
      return parseIdentifierExpr();
    case tok_number:
      return parseNumberExpr();
    case tok_open_paren:
      return parseParenExpr();
    default:
      return nullptr;
  }
}

std::unique_ptr<ExprAST> Parser::parseBinOpRHS(int exprPrec, std::unique_ptr<ExprAST> lhs)
{
  while(true)
  {
    int currPrec = getTokPrec();
    if(currPrec < exprPrec )
    {
      return lhs;
    }

    Token binop = m_curr_token;
    advanceToken(); // eat operator

    auto rhs = parsePrimary();
    if(nullptr == rhs)
    {
      return nullptr;
    }

    int nextPrec = getTokPrec();
    if(currPrec < nextPrec)
    {
      rhs = parseBinOpRHS(currPrec + 1, std::move(rhs));
      if(nullptr == rhs)
      {
        return nullptr;
      }
    }
    lhs = std::make_unique<BinaryExprAST>(std::move(lhs), binop, std::move(rhs));
  }
}

std::unique_ptr<ExprAST> Parser::parseExpression()
{
  auto lhs = parsePrimary();
  if(nullptr == lhs)
  {
    return nullptr;
  }
  return parseBinOpRHS(0, std::move(lhs));
}

void Parser::read_line()
{
  getline(std::cin, m_source);
}

std::string Parser::get_source()
{
  return m_source;
}

void Parser::skip_whitespace()
{
  while(isspace(m_source[m_curr_idx]))
  {
    m_curr_idx++;
  }
}

int Parser::scan_int()
{
  char curr_char = m_source[m_curr_idx];
  int result = 0;
  while(isdigit(curr_char))
  {
    result = result * 10 + curr_char - '0';
    m_curr_idx++;
    if(m_curr_idx >= m_source.length() || isspace(m_source[m_curr_idx]))
    {
      break;
    }
    curr_char = m_source[m_curr_idx];
  }
  return result;
}

// entry point
void Parser::parse()
{
  m_curr_token = advanceToken();
  while(m_curr_token != tok_eof)
  {
    if(m_curr_token == tok_def)
    {
      if(auto fAst = parseDefinition())
      {
        if(auto* fIR = fAst->codegen())
        {
          // printf("Parsed function definition");
          // fIR->print(llvm::errs());
          printf("\n");
          ExitOnErr(TheJIT->addModule(
            llvm::orc::ThreadSafeModule(std::move(TheModule), std::move(TheContext))
          ));
          InitializeModule();
        }
        // printf("parsed definisi\n");
      }
      else
      {
        // errror
        advanceToken();
      }
    }
    else if(m_curr_token == tok_decl)
    {
      if(auto protoAst = parseDeclaration())
      {
        if(auto* fIR = protoAst->codegen())
        {
          // printf("parsed deklarasi\n");
          // fIR->print(llvm::errs());
          printf("\n");
          FunctionProtos[protoAst->getName()] = std::move(protoAst);
        }
      }
      else
      {
        advanceToken();
      }
    }
    else if(m_curr_token == tok_semicolon)
    {
      advanceToken();
    }
    else
    {
      if(auto fAst = parseTopLevel())
      {
        if(auto* fIR = fAst->codegen())
        {
          // fIR->print(llvm::errs());
          // printf("\n");
          auto RT = TheJIT->getMainJITDylib().createResourceTracker();
          auto TSM = llvm::orc::ThreadSafeModule(std::move(TheModule), std::move(TheContext));
          ExitOnErr(TheJIT->addModule(std::move(TSM), RT));
          InitializeModule();
          auto ExprSymbol = ExitOnErr(TheJIT->lookup("__anon_expr"));
          assert(ExprSymbol && "Function not found");

          // Get the symbol's address and cast it to the right type (takes no
          // arguments, returns an int) so we can call it as a native function.
          int (*FP)() = (int (*)())(intptr_t)ExprSymbol.getAddress();
          fprintf(stderr, "Evaluated to %d\n", FP());
          ExitOnErr(RT->remove());

        }
        // printf("Top Level parsed\n");
      }
      else
      {
        advanceToken();
      }
    }
  }
}

void Parser::read_file(const char* fileName)
{
  std::ifstream sourceFile;
  sourceFile.open(fileName);
  if(sourceFile.is_open())
  {
    std::stringstream ss;
    ss << sourceFile.rdbuf();
    m_source = ss.str();
  }
}

Token Parser::getCurrToken()
{
  return m_curr_token;
}

Token Parser::advanceToken()
{
  // avoid overflow
  if(m_curr_idx >= m_source.length())
  {
    m_curr_token = tok_eof;
    return tok_eof;
  }
  char curr_char;
  std::string identifier;

  skip_whitespace();
  curr_char = m_source[m_curr_idx];

  // every m_current_idx is incremented after scanning the token
  if(isdigit(curr_char))
  {
    m_numVal = scan_int();
    // printf("int scanned: %d\n", res);
    m_curr_token = tok_number;
  }
  else if(isalpha(curr_char))
  {
    do{
      curr_char = m_source[m_curr_idx];
      identifier += curr_char;
      ++m_curr_idx;
    } while( m_curr_idx < m_source.length() && isalnum(m_source[m_curr_idx]));
    if(identifier == "fungsi")
    {
      // printf("Keyword fungsi scanned\n");
      m_curr_token = tok_def;
    }
    else if(identifier == "deklarasi")
    {
      // printf("Keyword deklarasi scanned\n");
      m_curr_token = tok_decl;
    }
    else
    {
      // printf("identifier scanned: %s\n", identifier.c_str());
      m_identifierStr = identifier;
      m_curr_token = tok_identifier;
    }
  }
  else if(curr_char == '+')
  {
    // printf("PLUS operator scanned\n");
    ++m_curr_idx;
    m_curr_token = tok_plus;
  }
  else if(curr_char == '-')
  {
    // printf("MIN operator scanned\n");
    ++m_curr_idx;
    m_curr_token = tok_min;
  }
  else if(curr_char == '*')
  {
    // printf("MULT operator scanned\n");
    ++m_curr_idx;
    m_curr_token = tok_mult;
  }
  else if(curr_char == '<')
  {
    // printf("< scanned\n");
    ++m_curr_idx;
    m_curr_token = tok_less;
  }
  else if(curr_char == '(')
  {
    // printf("( scanned\n");
    ++m_curr_idx;
    m_curr_token = tok_open_paren;
  }
  else if(curr_char == ')')
  {
    // printf(") scanned\n");
    ++m_curr_idx;
    m_curr_token = tok_close_paren;
  }
  else if(curr_char == ',')
  {
    // printf(", scanned\n");
    ++m_curr_idx;
    m_curr_token = tok_comma;
  }
  else if(curr_char == ';')
  {
    // printf("semicolon scanned\n");
    ++m_curr_idx;
    m_curr_token = tok_semicolon;
  }
  else if(curr_char == '\n')
  {
  }
  else{
    if(m_curr_idx >= m_source.length())
    {
      m_curr_token = tok_eof;
    }
    ++m_curr_idx;
    m_curr_token = tok_notdef;
  }
  return m_curr_token;
}

std::unique_ptr<FuncAST> Parser::parseDefinition()
{
  advanceToken(); // consume definisi
  auto prototype = parsePrototype();
  if(nullptr == prototype)
  {
    return nullptr;
  }
  auto expr = parseExpression();
  if(expr)
  {
    return std::make_unique<FuncAST>(std::move(prototype), std::move(expr));
  }
  return nullptr;
}
std::unique_ptr<PrototypeAST> Parser::parseDeclaration()
{
  advanceToken(); // consume deklarasi
  return parsePrototype();
}
std::unique_ptr<FuncAST> Parser::parseTopLevel()
{
  auto expr = parseExpression();
  if(expr)
  {
    // make anonymous prototype
    auto prototype = std::make_unique<PrototypeAST>("__anon_expr", std::vector<std::string>());
    return std::make_unique<FuncAST>(std::move(prototype), std::move(expr));
  }
  return nullptr;
}
