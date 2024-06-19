#pragma once
#include <string>
#include <memory>

class ExprAST
{
public:
  virtual ~ExprAST() = default;
};

class NumberExprAST : public ExprAST
{
private:
  int m_numVal;
public:
  NumberExprAST(int numVal): m_numVal(numVal) {}
};

class VariableExprAST : public ExprAST
{
private:
  std::string m_name;
public:
  VariableExprAST(std::string& name): m_name(Name) {}
};

class BinaryExprAST: public ExprAST
{
private:
  char m_op;
  std::unique_ptr<ExprAST> m_lhs, m_rhs;
public:
  BinaryExprAST(std::unique_ptr<ExprAST> lhs, char op, std::unique_ptr<ExprAST> rhs)
    : m_lhs(std::move(lhs)), op(m_op), m_rhs(std::move(rhs)) {}
};

// function call ex: f(x, y)
class callExprAST: public ExprAST
{
private:
  std::string m_callee;
  std::vector<std::unique_ptr<ExprAST>> m_args;
public:
  callExprAST(std::string& callee, std::vector<std::unique_ptr<ExprAST>> args)
    : m_callee(callee), m_args(std::move(args)) {}
};

// function prorotype
class PrototypeAST
{
private:
  std::string m_name;
  std::vector<std::unique_ptr<ExprAST>> m_args;
public:
  PrototypeAST(std::string& name, std::vector<std::unique_ptr<ExprAST>> args)
    : m_name(name), m_args(std::move(m_args)) {}
};

class FuncAST
{
private:
  std::unique_ptr<PrototypeAST> m_proto;
  std::unique_ptr<ExprAST> m_body;
public:
  FuncAST(std::unique_ptr<PrototypeAST> proto, std::unique_ptr<ExprAST> body)
    : m_proto(std::move(proto)), m_body(std::move(body)) {}
};


