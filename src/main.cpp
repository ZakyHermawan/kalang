#include "runtime.hpp"
#include "parser.hpp"
#include "jit.hpp"
#include "runtime.hpp"

#include <iostream>
#include <map>


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
    parser.parse();
  }
  return;
}

void runFile(const char* fileName)
{
  Parser parser;
  parser.read_file(fileName);
  parser.parse();

  std::string Str;
  llvm::raw_string_ostream OS(Str);
  OS << *TheModule;
  OS.flush();

  std::ofstream myfile;
  myfile.open ("dump.ll");
  myfile << Str << std::endl;
  myfile.close();
}

int main(int argc, char** argv)
{
  
  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();
  llvm::InitializeNativeTargetAsmParser();
  TheJIT = ExitOnErr(llvm::orc::KalangJIT::Create());
 
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
    printf("Error, usage: ./kalang [path]\n");
    return 1;
  }
  return 0;
}
