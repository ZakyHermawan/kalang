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
