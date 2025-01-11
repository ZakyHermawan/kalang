#pragma once

#include "jit.hpp"
#include "ast.hpp"

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/PassManager.h>
#include <llvm/IR/PassInstrumentation.h>
#include <llvm/Passes/StandardInstrumentations.h>
#include <llvm/Support/Error.h>
#include <llvm/IR/Function.h>
#include <llvm/Transforms/Utils/Mem2Reg.h>

extern std::unique_ptr<llvm::LLVMContext> TheContext;
extern std::unique_ptr<llvm::Module> TheModule;
extern std::unique_ptr<llvm::IRBuilder<>> Builder;
extern std::map<std::string, llvm::AllocaInst *> NamedValues;
extern std::shared_ptr<llvm::orc::KalangJIT> TheJIT;
extern std::unique_ptr<llvm::FunctionPassManager> TheFPM;
extern std::unique_ptr<llvm::LoopAnalysisManager> TheLAM;
extern std::unique_ptr<llvm::FunctionAnalysisManager> TheFAM;
extern std::unique_ptr<llvm::CGSCCAnalysisManager> TheCGAM;
extern std::unique_ptr<llvm::ModuleAnalysisManager> TheMAM;
extern std::unique_ptr<llvm::PassInstrumentationCallbacks> ThePIC;
extern std::unique_ptr<llvm::StandardInstrumentations> TheSI;
extern std::map<std::string, std::unique_ptr<PrototypeAST>> FunctionProtos;
extern llvm::ExitOnError ExitOnErr;

void InitializeModule();
llvm::Function* getFunction(std::string Name);
llvm::AllocaInst* CreateEntryBlockAlloca(llvm::Function* TheFunction, llvm::StringRef varName);

