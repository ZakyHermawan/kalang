#include "runtime.hpp"
#include "ast.hpp"

llvm::Value* NumberExprAST::codegen()
{
  return llvm::ConstantInt::get(llvm::Type::getInt32Ty(*TheContext), (m_numVal), true);
}

llvm::Value *VariableExprAST::codegen()
{
  // Look this variable up in the function.
  llvm::Value *V = NamedValues[m_name];
  if (nullptr == V)
  {
    printf("Unknown variable name\n");
  }
  return V;
}

llvm::Value *BinaryExprAST::codegen()
{
  llvm::Value *lhs = m_lhs->codegen();
  llvm::Value *rhs = m_rhs->codegen();
  if (!lhs || !rhs)
    return nullptr;

  switch (m_op) {
  case tok_plus:
    return Builder->CreateAdd(lhs, rhs, "addtemp");
  case tok_min:
    return Builder->CreateSub(lhs, rhs, "subtemp");
  case tok_mult:
    return Builder->CreateMul(lhs, rhs, "multemp");
  default:
    printf("Error: Invalid binary operator\n");
    return nullptr;
  }
}

llvm::Value *CallExprAST::codegen()
{
  // Look up the name in the global module table.
  llvm::Function *CalleeF = getFunction(m_callee);
  if (!CalleeF)
  {
    printf("Unknown function referenced\n");
  }

  // If argument mismatch error.
  if (CalleeF->arg_size() != m_args.size())
  {
    printf("Incorrect # arguments passed");
  }

  std::vector<llvm::Value *> ArgsV;
  for (unsigned i = 0, e = m_args.size(); i != e; ++i) {
    ArgsV.push_back(m_args[i]->codegen());
    if (!ArgsV.back())
      return nullptr;
  }

  return Builder->CreateCall(CalleeF, ArgsV, "calltmp");
}

llvm::Function *PrototypeAST::codegen()
{
  // Make the function type:  int(int,int) etc.
  std::vector<llvm::Type *> Integers(m_args.size(), llvm::Type::getInt32Ty(*TheContext));
  llvm::FunctionType *FT =
      llvm::FunctionType::get(llvm::Type::getInt32Ty(*TheContext), Integers, false);

  llvm::Function *F =
      llvm::Function::Create(FT, llvm::Function::ExternalLinkage, m_name, TheModule.get());

  // Set names for all arguments.
  unsigned int Idx = 0;
  for (auto &Arg : F->args())
  {
    Arg.setName(m_args[Idx++]);
  }
  return F;
}

llvm::Function *FuncAST::codegen()
{
  // First, check for an existing function from a previous 'extern' declaration.
  auto& P = *m_proto;
  FunctionProtos[m_proto->getName()] = std::move(m_proto);
  llvm::Function* TheFunction = getFunction(P.getName());

  if (!TheFunction)
  {
    return nullptr;
  }

  // Create a new basic block to start insertion into.
  llvm::BasicBlock *BB = llvm::BasicBlock::Create(*TheContext, "entry", TheFunction);
  Builder->SetInsertPoint(BB);

  // Record the function arguments in the NamedValues map.
  NamedValues.clear();
  for (auto &Arg : TheFunction->args())
    NamedValues[std::string(Arg.getName())] = &Arg;

  if (llvm::Value *RetVal = m_body->codegen()) {
    // Finish off the function.
    Builder->CreateRet(RetVal);

    // Validate the generated code, checking for consistency.
    verifyFunction(*TheFunction);
    return TheFunction;
  }

  // Error reading body, remove function.
  TheFunction->eraseFromParent();
  return nullptr;
}

