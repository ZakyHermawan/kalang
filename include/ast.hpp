#pragma once
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/StandardInstrumentations.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Transforms/Scalar/Reassociate.h"
#include "llvm/Transforms/Scalar/SimplifyCFG.h"

#include <string>
#include <memory>
#include <vector>
#include <utility>

#include "token.hpp"

class ExprAST
{
public:
  virtual ~ExprAST() = default;
  virtual llvm::Value* codegen() = 0;
};

class NumberExprAST : public ExprAST
{
private:
  int m_numVal;
public:
  NumberExprAST(int numVal): m_numVal(numVal) {}
  llvm::Value* codegen() override;
};

class VariableExprAST : public ExprAST
{
private:
  std::string m_name;
public:
  VariableExprAST(std::string& name): m_name(name) {}
  const std::string& getName() const { return m_name; }

  llvm::Value* codegen() override;
};

class BinaryExprAST: public ExprAST
{
private:
  Token m_op;
  std::unique_ptr<ExprAST> m_lhs, m_rhs;
public:
  BinaryExprAST(std::unique_ptr<ExprAST> lhs, Token op, std::unique_ptr<ExprAST> rhs)
    : m_lhs(std::move(lhs)), m_op(op), m_rhs(std::move(rhs)) {}
  llvm::Value* codegen() override;
};

// function call ex: f(x, y)
class CallExprAST: public ExprAST
{
private:
  std::string m_callee;
  std::vector<std::unique_ptr<ExprAST>> m_args;
public:
  CallExprAST(std::string& callee, std::vector<std::unique_ptr<ExprAST>> args)
    : m_callee(callee), m_args(std::move(args)) {}
  llvm::Value* codegen() override;
};

class IfExprAST: public ExprAST
{
private:
  std::unique_ptr<ExprAST> m_cond, m_then, m_else;
public:
  IfExprAST(std::unique_ptr<ExprAST> cond, std::unique_ptr<ExprAST> then, std::unique_ptr<ExprAST> else_)
    : m_cond(std::move(cond)), m_then(std::move(then)), m_else(std::move(else_)) {}

  llvm::Value* codegen() override;
};

class ForExprAST: public ExprAST
{
private:
  std::string m_varName;
  std::unique_ptr<ExprAST> m_start, m_end, m_step, m_body;

public:
  ForExprAST(
    const std::string& varName,
    std::unique_ptr<ExprAST> start,
    std::unique_ptr<ExprAST> end,
    std::unique_ptr<ExprAST> step,
    std::unique_ptr<ExprAST> body
  ):
    m_varName(varName),
    m_start(std::move(start)),
    m_end(std::move(end)),
    m_step(std::move(step)),
    m_body(std::move(body)) {}

  llvm::Value* codegen() override;
};

// function prorotype
class PrototypeAST
{
private:
  std::string m_name;
  std::vector<std::string> m_args;
public:
  PrototypeAST(const std::string& name, std::vector<std::string> args)
    : m_name(name), m_args(std::move(args)) {}
  const std::string& getName() const { return m_name; }
  llvm::Function* codegen();
};

class FuncAST
{
private:
  std::unique_ptr<PrototypeAST> m_proto;
  std::unique_ptr<ExprAST> m_body;
public:
  FuncAST() {}
  FuncAST(std::unique_ptr<PrototypeAST> proto, std::unique_ptr<ExprAST> body)
    : m_proto(std::move(proto)), m_body(std::move(body)) {}
  llvm::Function* codegen();
};

class VarExprAST: public ExprAST
{
private:

  using varTypes = std::vector<std::pair<std::string, std::unique_ptr<ExprAST>>>;
  varTypes m_varNames;
  std::unique_ptr<ExprAST> m_body;
  std::unique_ptr<ExprAST> m_retVal;

public:
  VarExprAST(varTypes varNames, std::unique_ptr<ExprAST> body, std::unique_ptr<ExprAST> retVal): 
    m_varNames(std::move(varNames)), m_body(std::move(body)), m_retVal(std::move(retVal)) {}

  llvm::Value* codegen() override;
};
