#include "parser.hpp"
#include <iostream>


int main(int argc, char** argv) {
  if(argc != 2) {
    fprintf(stderr, "Usage: %s filename\n", argv[0]);
    return 1;
  }
  
  std::string filename = argv[1];
  Parser parser(filename);
  return 0;
}
