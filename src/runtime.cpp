#include "runtime.hpp"

#include <memory>

std::unique_ptr<llvm::LLVMContext> TheContext;
std::unique_ptr<llvm::Module> TheModule;
std::unique_ptr<llvm::IRBuilder<>> Builder;
std::map<std::string, llvm::Value *> NamedValues;
std::shared_ptr<llvm::orc::KalangJIT> TheJIT;
std::unique_ptr<llvm::FunctionPassManager> TheFPM;
std::unique_ptr<llvm::LoopAnalysisManager> TheLAM;
std::unique_ptr<llvm::FunctionAnalysisManager> TheFAM;
std::unique_ptr<llvm::CGSCCAnalysisManager> TheCGAM;
std::unique_ptr<llvm::ModuleAnalysisManager> TheMAM;
std::unique_ptr<llvm::PassInstrumentationCallbacks> ThePIC;
std::unique_ptr<llvm::StandardInstrumentations> TheSI;
std::map<std::string, std::unique_ptr<PrototypeAST>> FunctionProtos;
llvm::ExitOnError ExitOnErr;

void InitializeModule() {
  // Open a new context and module.
  TheContext = std::make_unique<llvm::LLVMContext>();
  TheModule = std::make_unique<llvm::Module>("Just In Time Compiler", *TheContext);
  TheModule->setDataLayout(TheJIT->getDataLayout());

  // Create a new builder for the module.
  Builder = std::make_unique<llvm::IRBuilder<>>(*TheContext);

  TheFPM = std::make_unique<llvm::FunctionPassManager>();
  TheLAM = std::make_unique<llvm::LoopAnalysisManager>();
  TheFAM = std::make_unique<llvm::FunctionAnalysisManager>();
  TheCGAM = std::make_unique<llvm::CGSCCAnalysisManager>();
  TheMAM = std::make_unique<llvm::ModuleAnalysisManager>();
  ThePIC = std::make_unique<llvm::PassInstrumentationCallbacks>();
  TheSI = std::make_unique<llvm::StandardInstrumentations>(true, true);
  TheSI->registerCallbacks(*ThePIC, TheFAM.get());

  // Add transform passes.
  // Do simple "peephole" optimizations and bit-twiddling optzns.
  TheFPM->addPass(llvm::InstCombinePass());
  // Reassociate expressions.
  TheFPM->addPass(llvm::ReassociatePass());
  // Eliminate Common SubExpressions.
  TheFPM->addPass(llvm::GVNPass());
  // Simplify the control flow graph (deleting unreachable blocks, etc).
  TheFPM->addPass(llvm::SimplifyCFGPass());

  // Register analysis passes used in these transform passes.
  llvm::PassBuilder PB;
  PB.registerModuleAnalyses(*TheMAM);
  PB.registerFunctionAnalyses(*TheFAM);
  PB.crossRegisterProxies(*TheLAM, *TheFAM, *TheCGAM, *TheMAM);
}

llvm::Function* getFunction(std::string Name) {
  // First, see if the function has already been added to the current module.
  if (auto *F = TheModule->getFunction(Name))
    return F;

  // If not, check whether we can codegen the declaration from some existing
  // prototype.
  auto FI = FunctionProtos.find(Name);
  if (FI != FunctionProtos.end())
    return FI->second->codegen();

  // If no existing prototype exists, return null.
  return nullptr;
}

