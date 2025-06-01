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
#include "function_list.h"
#include "lexer.h"

#define TEMP_BUF_SIZE (MAX_TOKEN_LEN * 4)

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
  for (int i = 0; function_names[i]; i++) {
    if (strcmp(name, function_names[i]) == 0) return true;
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
  if (!match(lexer, ',')) { lexer->pos = start_pos; return make_token(TOK_UNKNOWN, "("); }
  Token imag = lex_number(lexer);
  if (!match(lexer, ')')) { lexer->pos = start_pos; return make_token(TOK_UNKNOWN, "("); }

  char temp_buf[TEMP_BUF_SIZE];
  snprintf(temp_buf, sizeof(temp_buf), "(%s,%s)", real.text, imag.text);

  return make_token(TOK_COMPLEX, temp_buf);
}

Token lex_matrix_file(Lexer* lexer) {
  size_t start_pos = lexer->pos;
  Token row = lex_number(lexer);
  if (!match(lexer, ',')) { lexer->pos = start_pos; return make_token(TOK_UNKNOWN, "["); }
  Token col = lex_number(lexer);
  if (!match(lexer, ',')) { lexer->pos = start_pos; return make_token(TOK_UNKNOWN, "["); }
  Token str = lex_string(lexer);
  if (!match(lexer, ']')) { lexer->pos = start_pos; return make_token(TOK_UNKNOWN, "["); }

  char temp_buf[TEMP_BUF_SIZE];
  snprintf(temp_buf, sizeof(temp_buf), "[%s,%s,\"%s\"]", row.text, col.text, str.text);
  
  return make_token(TOK_MATRIX_FILE, temp_buf);
}

Token lex_matrix_inline_j(Lexer* lexer) {
  size_t start_pos = lexer->pos;
  char buf[TEMP_BUF_SIZE];
  buf[0] = '\0';

  Token rows = lex_number(lexer);
  if (rows.type != TOK_NUMBER) {
    lexer->pos = start_pos;
    return make_token(TOK_UNKNOWN, "[");
  }

  skip_whitespace(lexer);

  Token cols = lex_number(lexer);
  if (cols.type != TOK_NUMBER) {
    lexer->pos = start_pos;
    return make_token(TOK_UNKNOWN, "[");
  }

  skip_whitespace(lexer);

  if (!match(lexer, '$')) {
    lexer->pos = start_pos;
    return make_token(TOK_UNKNOWN, "[");
  }

  skip_whitespace(lexer);

  bool has_real = false;
  bool has_complex = false;
  char temp[TEMP_BUF_SIZE];
  snprintf(buf, sizeof(buf), "%s %s $", rows.text, cols.text);

  while (peek(lexer) != '\0' && peek(lexer) != ']') {
    skip_whitespace(lexer);

    if (peek(lexer) == '(') {
      Token t = lex_complex(lexer);
      has_complex = true;
      snprintf(temp, sizeof(temp), " %s", t.text);
      strncat(buf, temp, sizeof(buf) - strlen(buf) - 1);
    } else if (isdigit(peek(lexer)) || (peek(lexer) == '-' && isdigit(lexer->input[lexer->pos + 1]))) {
      Token t = lex_number(lexer);
      has_real = true;
      snprintf(temp, sizeof(temp), " %s", t.text);
      strncat(buf, temp, sizeof(buf) - strlen(buf) - 1);
    } else {
      break;
    }

    skip_whitespace(lexer);
  }

  if (!match(lexer, ']')) {
    lexer->pos = start_pos;
    return make_token(TOK_UNKNOWN, "[");
  }

  TokenType type;
  if (has_complex && has_real) {
    type = TOK_MATRIX_INLINE_MIXED;
  } else if (has_complex) {
    type = TOK_MATRIX_INLINE_COMPLEX;
  } else {
    type = TOK_MATRIX_INLINE_REAL;
  }

  return make_token(type, buf);
}

Token next_token(Lexer* lexer) {
  skip_whitespace(lexer);
  char c = peek(lexer);
  if (c == '\0') return make_token(TOK_EOF, "<EOF>");

  if (isdigit(c) || (c == '-' && isdigit(lexer->input[lexer->pos + 1]))) return lex_number(lexer);
  if (c == '(') return lex_complex(lexer);

if (c == '[') {
  size_t temp_pos = lexer->pos + 1; // After '['
  skip_whitespace((Lexer*)&(Lexer){.input = lexer->input, .pos = temp_pos});
  if (isdigit(lexer->input[temp_pos]) || lexer->input[temp_pos] == '-') {
    // Look ahead for comma
    while (isdigit(lexer->input[temp_pos]) || lexer->input[temp_pos] == '.' || lexer->input[temp_pos] == '-' || lexer->input[temp_pos] == 'e' || lexer->input[temp_pos] == 'E' || lexer->input[temp_pos] == '+') {
      temp_pos++;
    }
    skip_whitespace((Lexer*)&(Lexer){.input = lexer->input, .pos = temp_pos});
    if (lexer->input[temp_pos] == ',') {
      // Likely a matrix file
      advance(lexer);  // Eat '['
      return lex_matrix_file(lexer);
    }
  }
  advance(lexer);  // Eat '['
  return lex_matrix_inline_j(lexer);
}

  if (isalpha(c) || c == '_') return lex_identifier(lexer);
  if (c == '"') return lex_string(lexer);

// Handle multi-character operators
  if (c == '.' && lexer->input[lexer->pos + 1] == '*') {
    lexer->pos += 2;
    return make_token(TOK_DOT_STAR, ".*");
  }
  if (c == '.' && lexer->input[lexer->pos + 1] == '/') {
    lexer->pos += 2;
    return make_token(TOK_DOT_SLASH, "./");
  }
  if (c == '.' && lexer->input[lexer->pos + 1] == '^') {
    lexer->pos += 2;
    return make_token(TOK_DOT_CARET, ".^");
  }
  
  switch (c) {
  case '+': advance(lexer); return make_token(TOK_PLUS, "+");
  case '-': advance(lexer); return make_token(TOK_MINUS, "-");
  case '*': advance(lexer); return make_token(TOK_STAR, "*");
  case '/': advance(lexer); return make_token(TOK_SLASH, "/");
  case '^': advance(lexer); return make_token(TOK_CARET, "^");
  case '<': advance(lexer); return make_token(TOK_BRA, "<");
  case '>': advance(lexer); return make_token(TOK_KET, ">");
  case '|': advance(lexer); return make_token(TOK_VERTICAL, "|");
  case ':': advance(lexer); return make_token(TOK_COLON, ":");
  case ';': advance(lexer); return make_token(TOK_SEMICOLON, ";");
  case '\'': advance(lexer); return make_token(TOK_FUNCTION, "'");
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
  case TOK_BRA: return "BRA";
  case TOK_KET: return "KET";
  case TOK_IDENTIFIER: return "IDENTIFIER";
  case TOK_FUNCTION: return "FUNCTION";
  case TOK_VERTICAL: return "VERTICAL";
  default: return "UNKNOWN";
  }
}
