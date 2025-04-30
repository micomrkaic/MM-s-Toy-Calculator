/*
 * This file is part of Mico's toy RPN Calculator
 *
 * Mico's toy RPN Calculator is free software: 
 * you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Mico's toy RPN Calculator is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Mico's toy RPN Calculator. If not, see <https://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <math.h>
#include <readline/history.h>
#include <readline/readline.h>

#define MAX_TOKEN_LEN 256
#define MAX_INPUT_LEN 4096
#define MAX_SUBTOKEN_LEN 100

typedef enum {
  TOK_EOF,
  TOK_NUMBER,
  TOK_COMPLEX,
  TOK_STRING,
  TOK_MATRIX_FILE,
  TOK_MATRIX_INLINE_REAL,
  TOK_MATRIX_INLINE_COMPLEX,
  TOK_MATRIX_INLINE_MIXED,
  TOK_PLUS,
  TOK_MINUS,
  TOK_STAR,
  TOK_SLASH,
  TOK_CARET,
  TOK_LPAREN,
  TOK_RPAREN,
  TOK_IDENTIFIER,
  TOK_FUNCTION,
  TOK_COMMA,
  TOK_UNKNOWN
} TokenType;

typedef struct {
  TokenType type;
  char text[MAX_TOKEN_LEN];
} Token;

typedef struct {
  const char* input;
  size_t pos;
} Lexer;

void skip_whitespace(Lexer* lexer) {
  while (isspace(lexer->input[lexer->pos])) lexer->pos++;
}

char peek(Lexer* lexer) {
  return lexer->input[lexer->pos];
}

char advance(Lexer* lexer) {
  return lexer->input[lexer->pos++];
}

bool match(Lexer* lexer, char expected) {
  if (peek(lexer) == expected) {
    lexer->pos++;
    return true;
  }
  return false;
}

Token make_token(TokenType type, const char* text) {
  Token token;
  strncpy(token.text, text, MAX_TOKEN_LEN - 1);
  token.text[MAX_TOKEN_LEN - 1] = '\0';
  token.type = type;
  return token;
}

bool is_function_name(const char* name) {
  const char* functions[] = {"sin", "cos", "tan","asin","acos","atan","sinh", "cosh", "tanh","asinh","acosh","atanh",
    "ln", "log", "exp", "sqrt", "pow","re","im", "abs","arg",
    "conj","scon","s2l","s2u","slen","srev","npdf","ncdf","nquant","re2c","split_c","j2r","chs","inv",NULL};
  for (int i = 0; functions[i]; i++) {
    if (strcmp(name, functions[i]) == 0) return true;
  }
  return false;
}

Token lex_number(Lexer* lexer) {
  size_t start = lexer->pos;
  if (peek(lexer) == '-') advance(lexer);

  while (isdigit(peek(lexer))) advance(lexer);

  if (peek(lexer) == '.') {
    advance(lexer);
    while (isdigit(peek(lexer))) advance(lexer);
  }

  if (peek(lexer) == 'e' || peek(lexer) == 'E') {
    advance(lexer);
    if (peek(lexer) == '+' || peek(lexer) == '-') advance(lexer);
    while (isdigit(peek(lexer))) advance(lexer);
  }

  size_t len = lexer->pos - start;
  char buf[MAX_TOKEN_LEN];
  strncpy(buf, &lexer->input[start], len);
  buf[len] = '\0';
  return make_token(TOK_NUMBER, buf);
}

Token lex_identifier(Lexer* lexer) {
  size_t start = lexer->pos;
  while (isalnum(peek(lexer)) || peek(lexer) == '_') advance(lexer);
  size_t len = lexer->pos - start;
  char buf[MAX_TOKEN_LEN];
  strncpy(buf, &lexer->input[start], len);
  buf[len] = '\0';
  return make_token(is_function_name(buf) ? TOK_FUNCTION : TOK_IDENTIFIER, buf);
}

Token lex_string(Lexer* lexer) {
  advance(lexer);
  size_t start = lexer->pos;
  while (peek(lexer) != '"' && peek(lexer) != '\0') advance(lexer);
  size_t len = lexer->pos - start;
  char buf[MAX_TOKEN_LEN];
  strncpy(buf, &lexer->input[start], len);
  buf[len] = '\0';
  match(lexer, '"');
  return make_token(TOK_STRING, buf);
}

Token lex_complex(Lexer* lexer) {
  size_t start_pos = lexer->pos;
  if (!match(lexer, '(')) return make_token(TOK_UNKNOWN, "(");
  Token real = lex_number(lexer);
  if (!match(lexer, ',')) { lexer->pos = start_pos + 1; return make_token(TOK_UNKNOWN, "("); }
  Token imag = lex_number(lexer);
  if (!match(lexer, ')')) { lexer->pos = start_pos + 1; return make_token(TOK_UNKNOWN, "("); }

  char buf[MAX_TOKEN_LEN];
  snprintf(buf, MAX_TOKEN_LEN, "(%.*s,%.*s)", MAX_SUBTOKEN_LEN, real.text, MAX_SUBTOKEN_LEN, imag.text);
  return make_token(TOK_COMPLEX, buf);
}

Token lex_matrix_file(Lexer* lexer) {
  size_t start_pos = lexer->pos;
  Token row = lex_number(lexer);
  if (!match(lexer, ',')) { lexer->pos = start_pos + 1; return make_token(TOK_UNKNOWN, "["); }
  Token col = lex_number(lexer);
  if (!match(lexer, ',')) { lexer->pos = start_pos + 1; return make_token(TOK_UNKNOWN, "["); }
  Token str = lex_string(lexer);
  if (!match(lexer, ']')) { lexer->pos = start_pos + 1; return make_token(TOK_UNKNOWN, "["); }

  char buf[MAX_TOKEN_LEN];
  //    snprintf(buf, MAX_TOKEN_LEN, "[%.*s,%.*s,\"%.*s\"]", MAX_SUBTOKEN_LEN, row.text, MAX_SUBTOKEN_LEN, col.text, MAX_SUBTOKEN_LEN, str.text);
  snprintf(buf, MAX_TOKEN_LEN, "[%.*s,%.*s,\"%.*s\"]", 
	   (int)(MAX_TOKEN_LEN - 4 - strlen(col.text) - strlen(row.text) - strlen(str.text)),
	   row.text, 
	   (int)(MAX_TOKEN_LEN - 4 - strlen(col.text) - strlen(row.text) - strlen(str.text)),
	   col.text, 
	   (int)(MAX_TOKEN_LEN - 4 - strlen(col.text) - strlen(row.text) - strlen(str.text)),
	   str.text);
  return make_token(TOK_MATRIX_FILE, buf);
}

Token lex_matrix_inline(Lexer* lexer) {
  char buf[MAX_TOKEN_LEN] = "[";
  int bracket_depth = 1;
  bool has_complex = false;
  bool has_real = false;

  while (peek(lexer) != '\0' && bracket_depth > 0) {
    char c = peek(lexer);
    size_t len = strlen(buf);

    if (c == '[') {
      snprintf(buf + len, MAX_TOKEN_LEN - len, "[");
      advance(lexer);
      bracket_depth++;
    } else if (c == ']') {
      snprintf(buf + len, MAX_TOKEN_LEN - len, "]");
      advance(lexer);
      bracket_depth--;
      if (bracket_depth == 0) break;
    } else if (c == '(') {
      Token t = lex_complex(lexer);
      has_complex = true;
      len = strlen(buf);
      snprintf(buf + len, MAX_TOKEN_LEN - len, "%s", t.text);
    } else if (isdigit(c) || c == '-') {
      Token t = lex_number(lexer);
      has_real = true;
      len = strlen(buf);
      snprintf(buf + len, MAX_TOKEN_LEN - len, "%s", t.text);
    } else if (isalpha(c)) {
      Token t = lex_identifier(lexer);
      len = strlen(buf);
      snprintf(buf + len, MAX_TOKEN_LEN - len, "%s", t.text);
    } else {
      char ch[2] = { advance(lexer), '\0' };
      len = strlen(buf);
      snprintf(buf + len, MAX_TOKEN_LEN - len, "%s", ch);
    }
  }

  TokenType type = has_complex ? (has_real ? TOK_MATRIX_INLINE_MIXED : TOK_MATRIX_INLINE_COMPLEX) : TOK_MATRIX_INLINE_REAL;
  return make_token(type, buf);
}


Token next_token(Lexer* lexer) {
  skip_whitespace(lexer);
  char c = peek(lexer);
  if (c == '\0') return make_token(TOK_EOF, "<EOF>");

  if (isdigit(c) || (c == '-' && isdigit(lexer->input[lexer->pos + 1]))) return lex_number(lexer);
  if (c == '(') return lex_complex(lexer);
  if (c == '[') {
    size_t temp_pos = lexer->pos + 1;
    while (lexer->input[temp_pos] != ']' && lexer->input[temp_pos] != '\0') {
      if (lexer->input[temp_pos] == '"') return (advance(lexer), lex_matrix_file(lexer));
      temp_pos++;
    }
    advance(lexer);
    return lex_matrix_inline(lexer);
  }
  if (isalpha(c) || c == '_') return lex_identifier(lexer);
  if (c == '"') return lex_string(lexer);

  switch (c) {
  case '+': advance(lexer); return make_token(TOK_PLUS, "+");
  case '-': advance(lexer); return make_token(TOK_MINUS, "-");
  case '*': advance(lexer); return make_token(TOK_STAR, "*");
  case '/': advance(lexer); return make_token(TOK_SLASH, "/");
  case '^': advance(lexer); return make_token(TOK_CARET, "^");
  case '<': advance(lexer); return make_token(TOK_LPAREN, "<");
  case '>': advance(lexer); return make_token(TOK_RPAREN, ">");
  case '|': advance(lexer); return make_token(TOK_COMMA, "|");
  default: {
    char unknown_char = advance(lexer);
    char unk[2] = { unknown_char, '\0' };
    return make_token(TOK_UNKNOWN, unk);
  }
  }
}

const char* token_type_str(TokenType type) {
  switch (type) {
  case TOK_EOF: return "EOF";
  case TOK_NUMBER: return "NUMBER";
  case TOK_COMPLEX: return "COMPLEX";
  case TOK_STRING: return "STRING";
  case TOK_MATRIX_FILE: return "MATRIX_FILE";
  case TOK_MATRIX_INLINE_REAL: return "MATRIX_INLINE_REAL";
  case TOK_MATRIX_INLINE_COMPLEX: return "MATRIX_INLINE_COMPLEX";
  case TOK_MATRIX_INLINE_MIXED: return "MATRIX_INLINE_MIXED";
  case TOK_PLUS: return "PLUS";
  case TOK_MINUS: return "MINUS";
  case TOK_STAR: return "STAR";
  case TOK_SLASH: return "SLASH";
  case TOK_CARET: return "CARET";
  case TOK_LPAREN: return "LPAREN";
  case TOK_RPAREN: return "RPAREN";
  case TOK_IDENTIFIER: return "IDENTIFIER";
  case TOK_FUNCTION: return "FUNCTION";
  case TOK_COMMA: return "COMMA";
  case TOK_UNKNOWN: return "UNKNOWN";
  default: return "<invalid>";
  }
}

