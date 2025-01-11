#include "runtime.hpp"
#include "ast.hpp"
#include <iterator>

llvm::Value* NumberExprAST::codegen()
{
  return llvm::ConstantInt::get(llvm::Type::getInt32Ty(*TheContext), (m_numVal), true);
}

llvm::Value* VariableExprAST::codegen()
{
  // Look this variable up in the function.
  llvm::AllocaInst* A = NamedValues[m_name];
  if (nullptr == A)
  {
    printf("Unknown variable name\n");
    return nullptr;
  }
  return Builder->CreateLoad(A->getAllocatedType(), A, m_name.c_str());
}

llvm::Value* BinaryExprAST::codegen()
{
  if (m_op == tok_assignment)
  {
    // do not emit lhs as expression
    // left hand side equation
    auto* lhse = dynamic_cast<VariableExprAST*>(m_lhs.get());
    if(!lhse)
    {
      printf("destination of '=' must be a variable\n");
      return nullptr;
    }
    auto* val = m_rhs->codegen();
    if(!val)
    {
      return nullptr;
    }
    auto* variable = NamedValues[lhse->getName()];
    if(!variable)
    {
      printf("Unknown variable name\n");
      return nullptr;
    }
    Builder->CreateStore(val, variable);
    return val;
  }
  llvm::Value* lhs = m_lhs->codegen();
  llvm::Value* rhs = m_rhs->codegen();
  if (!lhs || !rhs)
    return nullptr;
 
  switch (m_op) {
  case tok_plus:
    return Builder->CreateAdd(lhs, rhs, "addtemp");
  case tok_min:
    return Builder->CreateSub(lhs, rhs, "subtemp");
  case tok_mult:
    return Builder->CreateMul(lhs, rhs, "multemp");
  case tok_less:
    return Builder->CreateICmpSLT(lhs, rhs, "cmptmp");
  default:
    printf("Error: Invalid binary operator\n");
    return nullptr;
  }
}

llvm::Value* CallExprAST::codegen()
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

llvm::Value* IfExprAST::codegen()
{
  llvm::Value* condv = m_cond->codegen();
  if(!condv)
  {
    return nullptr;
  }

  auto condvType = condv->getType();
  if(condvType->isIntegerTy(1))
  {
    condv = Builder->CreateICmpEQ(
      condv,
      llvm::ConstantInt::get(llvm::Type::getInt1Ty(*TheContext), 1), "ifcond"
    );
  }
  else
  {
    condv = Builder->CreateICmpEQ(
      condv,
      llvm::ConstantInt::get(llvm::Type::getInt32Ty(*TheContext), 1), "ifcond"
    );
  }

  llvm::Function* TheFunction = Builder->GetInsertBlock()->getParent();
  auto* thenBB = llvm::BasicBlock::Create(*TheContext, "then", TheFunction);
  auto* elseBB = llvm::BasicBlock::Create(*TheContext, "else", TheFunction);
  auto* mergeBB = llvm::BasicBlock::Create(*TheContext, "mergeif", TheFunction);

  Builder->CreateCondBr(condv, thenBB, elseBB);
  Builder->SetInsertPoint(thenBB);

  auto* thenV = m_then->codegen();
  if(!thenV)
  {
    return nullptr;
  }
  Builder->CreateBr(mergeBB);

  Builder->SetInsertPoint(elseBB);
  auto* elseV = m_else->codegen();
  if(!elseV)
  {
    return nullptr;
  }
  Builder->CreateBr(mergeBB);

  Builder->SetInsertPoint(mergeBB);
  auto* phiNode = Builder->CreatePHI(llvm::Type::getInt32Ty(*TheContext), 2, "iftmp");
  phiNode->addIncoming(thenV, thenBB);
  phiNode->addIncoming(elseV, elseBB);
  return phiNode;
}

llvm::Value* ForExprAST::codegen()
{
  llvm::Function* TheFunction = Builder->GetInsertBlock()->getParent();
  llvm::AllocaInst* Alloca = CreateEntryBlockAlloca(TheFunction, m_varName);

  auto startV = m_start->codegen();
  if(!startV)
  {
    return nullptr;
  }
  Builder->CreateStore(startV, Alloca);

  auto* preLoopBB = Builder->GetInsertBlock();
  auto* loopBB = llvm::BasicBlock::Create(*TheContext, "loop", TheFunction);
  auto* afterLoopBB = llvm::BasicBlock::Create(*TheContext, "afterloop", TheFunction);
  
  Builder->CreateBr(loopBB);
  Builder->SetInsertPoint(loopBB);

  auto* oldVal = NamedValues[m_varName];
  NamedValues[m_varName] = Alloca;
 
  auto body = m_body->codegen();
  if(!body)
  {
    return nullptr;
  }
 
  llvm::Value* stepVal = nullptr;
  if(m_step)
  {
    stepVal = m_step->codegen();
    if(!stepVal)
    {
      return nullptr;
    }
  }
  else
  {
    // the default value of stepVal is 1
    stepVal = llvm::ConstantInt::get(llvm::Type::getInt32Ty(*TheContext), 1);
  }

  auto* currVar = Builder->CreateLoad(Alloca->getAllocatedType(), Alloca, m_varName.c_str());
  auto* nextVar = Builder->CreateAdd(currVar, stepVal, "nextvar");
  Builder->CreateStore(nextVar, Alloca);

  auto* endCond = m_end->codegen();
  if(!endCond)
  {
    return nullptr;
  }

  endCond = Builder->CreateICmpEQ(
    endCond,
    llvm::ConstantInt::get(llvm::Type::getInt1Ty(*TheContext), 1), "loopcond"
  );

  Builder->CreateCondBr(endCond, loopBB, afterLoopBB);
  Builder->SetInsertPoint(afterLoopBB);

  // restore the unshadowed variable (restore the old value of that variable)
  if(oldVal)
  {
    NamedValues[m_varName] = oldVal;
  }
  else
  {
    // if there is no variable being shadowed
    NamedValues.erase(m_varName);
  }

  return llvm::ConstantInt::get(llvm::Type::getInt32Ty(*TheContext), 0);
}


llvm::Function* PrototypeAST::codegen()
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

llvm::Function* FuncAST::codegen()
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
  {
    auto* Alloca = CreateEntryBlockAlloca(TheFunction, Arg.getName());
    Builder->CreateStore(&Arg, Alloca);
    NamedValues[std::string(Arg.getName())] = Alloca;
  }

  if (llvm::Value *RetVal = m_body->codegen()) {
    // Finish off the function.
    Builder->CreateRet(RetVal);

    // Validate the generated code, checking for consistency.
    verifyFunction(*TheFunction);
    TheFPM->run(*TheFunction, *TheFAM);
    return TheFunction;
  }

  // Error reading body, remove function.
  TheFunction->eraseFromParent();
  return nullptr;
}

llvm::Value* VarExprAST::codegen()
{
  std::vector<llvm::AllocaInst*> oldBindings;
  auto* TheFunction = Builder->GetInsertBlock()->getParent();

  for(unsigned i = 0; i < m_varNames.size(); ++i)
  {
    const std::string& varName = m_varNames[i].first;
    ExprAST* init = m_varNames[i].second.get();

    // initialize value
    llvm::Value* initVal;
    if(init)
    {
      initVal = init->codegen();
      if(!initVal)
      {
        return nullptr;
      }
    }
    else
    {
      // set 0 as the default value
      initVal = llvm::ConstantInt::get(llvm::Type::getInt32Ty(*TheContext), 0);
    }
    llvm::AllocaInst* Alloca = CreateEntryBlockAlloca(TheFunction, varName);
    auto x = Builder->CreateStore(initVal, Alloca);
    oldBindings.push_back(NamedValues[varName]);
    NamedValues[varName] = Alloca;
  }


  auto bodyVal = m_body->codegen();
  if(!bodyVal)
  {
    return nullptr;
  }


  auto retVal = m_retVal->codegen();
  if(!retVal)
  {
    return nullptr;
  }
  for(unsigned i = 0; i < m_varNames.size(); ++i)
  {
    NamedValues[m_varNames[i].first] = oldBindings[i];
  }

  return retVal;
}

