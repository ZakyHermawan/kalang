#pragma once
enum Token
{
  tok_start, // 0
  tok_eof,
  tok_def,
  tok_decl,
  tok_extern,
  tok_identifier,
  tok_number,
  tok_binop,
  tok_notdef,
  tok_semicolon,
  tok_plus,
  tok_min,
  tok_mult,
  tok_open_paren,
  tok_close_paren,
  tok_comma,
  tok_less,
};
