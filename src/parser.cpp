#include "parser.hpp"
#include <iostream>
#include <cmath>

Parser::Parser(std::string& filename)
  : m_eof_reached(0), m_next_token(tok_start)
{
  m_file.open(filename, std::fstream::in);
  if(m_file.fail()) {
    fprintf(stderr, "failed to open file\n");
  }
  m_file.get(m_next_char);
  if(m_file.eof()) {
    m_eof_reached = 1;
  }
  while(m_eof_reached == 0 and m_next_token != tok_eof) {
    if(m_file.eof()) {
      break;
    }
    m_next_token = gettok();
  }
}

Token Parser::gettok()
{
  while(1) {
    if(m_file.eof()) {
      m_eof_reached = 1;
      return tok_eof;
    }
    else if(m_next_char == '+' or m_next_char == '-' or m_next_char == '*') {
      printf("%c\n", m_next_char);
      m_file.get(m_next_char);
      return tok_binop;
    }
    else if(isdigit(m_next_char)) {
      m_numVal = 0;
      do {
        m_numVal = (m_next_char - '0') + m_numVal * 10;
        m_file.get(m_next_char);
      } while(std::isdigit(m_next_char) and !m_file.eof());
      printf("%d\n", m_numVal);
      return tok_number;
    }
    else if(isalpha(m_next_char)) {
      m_identifierStr = "";
      do {
        m_identifierStr.push_back(m_next_char);
        m_file.get(m_next_char);
      } while(isalnum(m_next_char) and !m_file.eof());
      if(m_identifierStr == "def") {
        printf("%s\n", m_identifierStr.c_str());
        return tok_def;
      }
      else if(m_identifierStr == "extern") {
        printf("%s\n", m_identifierStr.c_str());
        return tok_extern;
      }
      printf("%s\n", m_identifierStr.c_str());
      return tok_identifier;
    }
    else if(m_next_char == '#') {
      do {
        m_file.get(m_next_char);
      } while(!m_file.eof() and m_next_char != '\n');
    }
    m_file.get(m_next_char);
  }
  return tok_eof;
}
