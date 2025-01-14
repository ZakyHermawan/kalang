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
  tok_colon,
  tok_plus,
  tok_min,
  tok_mult,
  tok_assignment,
  tok_open_paren,
  tok_close_paren,
  tok_comma,
  tok_less,
  tok_if,
  tok_for,
  tok_in,
  tok_then,
  tok_else,
  tok_var
};
