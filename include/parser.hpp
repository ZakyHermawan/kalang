#include <string>
#include <fstream>

enum Token
{
  tok_start,
  tok_eof,
  tok_def,
  tok_extern,
  tok_identifier,
  tok_number,
  tok_binop,
};

class Parser
{
private:
  std::string m_identifierStr;
  int m_numVal;
  char m_lastChar;
  std::ifstream m_file;
  bool m_eof_reached;
  char m_next_char;
  Token m_next_token;
public:
  Parser(std::string& filename);
  ~Parser() = default;
  Token gettok();
};
