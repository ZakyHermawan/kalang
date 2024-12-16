#include "parser.hpp"
#include <iostream>
#include <map>

#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/TargetSelect.h"


static std::unique_ptr<llvm::LLVMContext> TheContext;
static std::unique_ptr<llvm::Module> TheModule;
static std::unique_ptr<llvm::IRBuilder<>> Builder;
static std::map<std::string, llvm::Value *> NamedValues;

llvm::Value* NumberExprAST::codegen()
{
  return llvm::ConstantInt::get(llvm::Type::getInt32Ty(*TheContext), (m_numVal), true);
}

llvm::Value *VariableExprAST::codegen() {
  // Look this variable up in the function.
  llvm::Value *V = NamedValues[m_name];
  if (nullptr == V)
  {
    printf("Unknown variable name\n");
  }
  return V;
}

llvm::Value *BinaryExprAST::codegen() {
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

llvm::Value *CallExprAST::codegen() {
  // Look up the name in the global module table.
  llvm::Function *CalleeF = TheModule->getFunction(m_callee);
  if (!CalleeF)
    printf("Unknown function referenced\n");

  // If argument mismatch error.
  if (CalleeF->arg_size() != m_args.size())
    printf("Incorrect # arguments passed");

  std::vector<llvm::Value *> ArgsV;
  for (unsigned i = 0, e = m_args.size(); i != e; ++i) {
    ArgsV.push_back(m_args[i]->codegen());
    if (!ArgsV.back())
      return nullptr;
  }

  return Builder->CreateCall(CalleeF, ArgsV, "calltmp");
}

llvm::Function *PrototypeAST::codegen() {
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

llvm::Function *FuncAST::codegen() {
  // First, check for an existing function from a previous 'extern' declaration.
  llvm::Function *TheFunction = TheModule->getFunction(m_proto->getName());

  if (!TheFunction)
    TheFunction = m_proto->codegen();

  if (!TheFunction)
    return nullptr;

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

static void InitializeModule() {
  // Open a new context and module.
  TheContext = std::make_unique<llvm::LLVMContext>();
  TheModule = std::make_unique<llvm::Module>("Just In Time Compiler", *TheContext);

  // Create a new builder for the module.
  Builder = std::make_unique<llvm::IRBuilder<>>(*TheContext);
}

static void repl()
{
  while(1)
  {
    Parser parser; // reset parser state
    printf("REPL> ");
    parser.read_line();
    if(std::cin.eof())
    {
      printf("\n");
      break;
    }
    std::string source_code = parser.get_source();
    // printf("source code: %s\n", source_code.c_str());
    parser.parse();
    TheModule->print(llvm::errs(), nullptr);
  }
  return;
}

void runFile(const char* fileName)
{
  Parser parser;
  parser.read_file(fileName);
  // printf("source code:\n%s", parser.get_source().c_str());
  parser.parse();

  // TheModule->print(llvm::errs(), nullptr);
  std::string Str;
  llvm::raw_string_ostream OS(Str);
  OS << *TheModule;
  OS.flush();
  // std::cout << OS.str() << std::endl;
  std::ofstream myfile;
  myfile.open ("dump.ll");
  myfile << Str << std::endl;
  myfile.close();
}

int main(int argc, char** argv)
{
  InitializeModule();
  if(argc == 1)
  {
    repl();
  }
  else if(argc == 2)
  {
    runFile(argv[1]);
  }
  else {
    printf("Error, usage: ./kaleidoscope [path]\n");
    return 1;
  }
  // Parser parser;
  return 0;
}
