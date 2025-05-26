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
#include <math.h>
#include <complex.h>
#include <time.h>
#include <readline/history.h>
#include <readline/readline.h>
#include "lexer.h"
#include "stack.h"
#include "string_fun.h"
#include "linear_algebra.h"
#include "matrix_fun.h"
#include "math_parsers.h"
#include "math_helpers.h"
#include "binary_fun.h"
#include "unary_fun.h"
#include "splash_and_help.h"
#include "print_fun.h"
#include "my_date_fun.h"
#include "matrix_fun.h"
#include "poly_fun.h"
#include "stat_fun.h"
#include "registers.h"
#include "compare_fun.h"
#include "eval_fun.h"

typedef void (*UnaryFunc)(Stack *stack);

typedef struct {
  const char* name;
  UnaryFunc func;
} ImmutableUnaryOp;

static const ImmutableUnaryOp immutable_unary_ops[] = {
  {"sin",   sin_wrapper},
  {"cos",   cos_wrapper},
  {"tan",   tan_wrapper},
  {"asin",  asin_wrapper},
  {"acos",  acos_wrapper},
  {"atan",  atan_wrapper},
  {"sinh",  sinh_wrapper},
  {"cosh",  cosh_wrapper},
  {"tanh",  tanh_wrapper},
  {"asinh", asinh_wrapper},
  {"acosh", acosh_wrapper},
  {"atanh", atanh_wrapper},
  {"exp",   exp_wrapper},
  {"chs",   chs_wrapper},
  {"inv",   inv_wrapper},
  {NULL,    NULL}
};

typedef struct {
  const char* name;
  const char* axis;
  const char* operation;
} MatrixReduceOp;

static const MatrixReduceOp reduce_ops[] = {
  {"cmean", "col", "mean"},
  {"rmean", "row", "mean"},
  {"csum",  "col", "sum"},
  {"rsum",  "row", "sum"},
  {"cvar",  "col", "var"},
  {"rvar",  "row", "var"},
  {"cmin", "col", "min"},
  {"rmin", "row", "min"},
  {"cmax",  "col", "max"},
  {"rmax",  "row", "max"},
  {NULL, NULL, NULL}
};

typedef int (*MatrixFunc)(Stack*);

typedef struct {
  const char* name;
  MatrixFunc func;
} MatrixOp;

static const MatrixOp matrix_ops[] = {
  {"minv",    matrix_inverse},
  {"det",     matrix_determinant},
  {"eig",     matrix_eigen_decompose},
  {"tran",    matrix_transpose},
  {"reshape", reshape_matrix},
  {"get_aij", select_matrix_element},
  {"set_aij", set_matrix_element},
  {"kron",    kronecker_top_two},
  {"diag",    matrix_extract_diagonal},
  {"chol",    matrix_cholesky},
  {"svd",     matrix_svd},
  {"dim",     matrix_dimensions},
  {"eye",     make_unit_matrix},
  {"ones",    make_matrix_of_ones},
  {"zeroes",  make_matrix_of_zeroes},
  {"rand",    make_random_matrix},
  {"randn",   make_gaussian_random_matrix},
  {NULL,      NULL}
};

// **************** The main loop in this file ****************
void evaluator(Stack *stack, Stack *old_stack, char* line) {
  Lexer lexer = {line, 0};
  Token tok;

  if (!strcmp(line, "undo")) // Restore the old stack
    { copy_stack(stack, old_stack);
      return;
    }
  
  copy_stack(old_stack, stack);  // Preserve the stack

  // The lexer loop
  do {
    tok = next_token(&lexer);
    evaluate_one_token(stack, tok);    
  } while (tok.type != TOK_EOF);
}

// **************** Process one token ****************
void evaluate_one_token(Stack *stack, Token tok) {
  switch (tok.type) {
  case TOK_EOF:
    return;
  case TOK_NUMBER:
    push_real(stack, atof(tok.text));
    return;
  case TOK_COMPLEX: {
    double complex z;
    if (read_complex(tok.text, &z)) push_complex(stack,z);
    return;}
  case TOK_STRING:
    push_string(stack, tok.text);
    return;
  case TOK_MATRIX_FILE:
    read_matrix_from_file(stack, tok.text);
    return;
  case TOK_MATRIX_INLINE_REAL:
    push_matrix_real(stack,parse_matrix_literal(tok.text));
    return;
  case TOK_MATRIX_INLINE_COMPLEX:
    push_matrix_complex(stack,parse_complex_matrix_literal(tok.text));
    return;
  case TOK_MATRIX_INLINE_MIXED:
    push_matrix_complex(stack,parse_complex_matrix_literal(tok.text));
    return;
  case TOK_PLUS:
    add_top_two(stack);
    return;
  case TOK_MINUS:
    sub_top_two(stack);
    return;
  case TOK_STAR:
    mul_top_two(stack);
    return;
  case TOK_SLASH:
    div_top_two(stack);
    return;
  case TOK_CARET:
    pow_top_two(stack);
    return;
  case TOK_DOT_STAR:
    dot_mult_top_two(stack);
    return;
  case TOK_DOT_SLASH:
    dot_div_top_two(stack);
    return;
  case TOK_DOT_CARET:
    dot_pow_top_two(stack);
    return;
  case TOK_BRA:
    printf("< \n");
    return;
  case TOK_KET:
    printf("> \n");
    return;
  case TOK_IDENTIFIER:
    printf("Unknown identifier!\n");
    return;
  case TOK_FUNCTION: {

    // ++++++++++++++++ Nullary functions ++++++++++++++++
    // Constants
    if (!strcmp("gravity",tok.text)) {push_real(stack,9.81); return; }
    if (!strcmp("pi",tok.text)) { push_real(stack,M_PI); return; }
    if (!strcmp("e",tok.text)) { push_real(stack,exp(1.0)); return; }

    // Misc
    if (!strcmp("help",tok.text)) { help_menu(); return; }
    if (!strcmp("fuck",tok.text)) { whose_place(); return; }
    
    // Utility functions
    if (!strcmp("pm",tok.text)) {print_matrix(stack); return;}
    if (!strcmp("setprec",tok.text)) {set_print_precision(stack); return;}
    if (!strcmp("sfs",tok.text)) {swap_fixed_scientific(); return;}
    
    // Date and time functions
    if (!strcmp("ddays",tok.text)) { delta_days_strings(stack); return; }
    if (!strcmp("today",tok.text)) { push_today_date(stack); return; }
    if (!strcmp("dow",tok.text)) { push_weekday_name_from_date_string(stack); return; }
    if (!strcmp("dateplus",tok.text)) { date_plus_days(stack); return; }
    if (!strcmp("edmy",tok.text)) {  extract_day_month_year(stack); return; }
 
    // Stack functions
    if (!strcmp("drop",tok.text)) { pop_and_free(stack); return; }
    if (!strcmp("clst",tok.text)) { free_stack(stack); return; }
    if (!strcmp("swap",tok.text)) { swap(stack); return; }
    if (!strcmp("dup",tok.text)) { dup(stack); return; }

    // Polynomial functions
    if (!strcmp("roots",tok.text)) { poly_roots(stack); return;}
    if (!strcmp("pval",tok.text)) { poly_eval(stack); return; }

     // Comparison and logic functions
    if (!strcmp("eq",tok.text)) { dot_cmp_top_two(stack, CMP_EQ); return; }
    if (!strcmp("neq",tok.text)) { dot_cmp_top_two(stack, CMP_NE); return; }
    if (!strcmp("lt",tok.text)) { dot_cmp_top_two(stack, CMP_LT); return; }
    if (!strcmp("leq",tok.text)) { dot_cmp_top_two(stack, CMP_LE); return; }
    if (!strcmp("gt",tok.text)) { dot_cmp_top_two(stack, CMP_GT); return; }
    if (!strcmp("geq",tok.text)) { dot_cmp_top_two(stack, CMP_GE); return; } 
    if (!strcmp("and",tok.text)) { dot_cmp_top_two(stack, CMP_AND); return; } 
    if (!strcmp("not",tok.text)) { logical_not_wrapper(stack); return; } 
    
    // Special functions
    if (!strcmp("npdf",tok.text)) { npdf_wrapper(stack); return; }
    if (!strcmp("ncdf",tok.text)) { ncdf_wrapper(stack); return; }
    if (!strcmp("nquant",tok.text)) { nquant_wrapper(stack); return; }
    if (!strcmp("gamma",tok.text)) { gamma_wrapper(stack); return; }
    if (!strcmp("ln_gamma",tok.text)) { ln_gamma_wrapper(stack); return; }
    if (!strcmp("beta",tok.text)) { beta_wrapper(stack); return; }
    if (!strcmp("ln_beta",tok.text)) { ln_beta_wrapper(stack); return; }
    
    // Register functions
    if (!strcmp("rcl",tok.text)) { recall_from_register(stack); return; }
    if (!strcmp("sto",tok.text)) { store_to_register(stack); return; }
    if (!strcmp("pr",tok.text)) { show_registers_status(); return; }

    // **************** String functions ****************
    if (!strcmp("scon",tok.text)) {concatenate(stack); return;}
    if (!strcmp("s2l",tok.text)) { to_lower(stack); return; }
    if (!strcmp("s2u",tok.text)) { to_upper(stack); return; }
    if (!strcmp("slen",tok.text)) { string_length(stack); return; }
    if (!strcmp("srev",tok.text)) { string_reverse(stack); return; }       
    
    // **************** Matrix operations ****************
    for (int i = 0; matrix_ops[i].name != NULL; ++i) {
      if (!strcmp(tok.text, matrix_ops[i].name)) {
	matrix_ops[i].func(stack);
	return;
      }
    }
    // **************** Immutable unary functions ****************      
    for (int i = 0; immutable_unary_ops[i].name != NULL; ++i) {
      if (!strcmp(tok.text, immutable_unary_ops[i].name)) {
	immutable_unary_ops[i].func(stack);
	return;
      }
    }
    // **************** Mutable unary operations ****************     
    if (!strcmp(tok.text, "split_c")) { split_complex(stack); return; }    
    if (!strcmp(tok.text, "abs")) { abs_wrapper(stack); return; }
    if (!strcmp(tok.text, "re")) { re_wrapper(stack); return; }
    if (!strcmp(tok.text, "im")) { im_wrapper(stack); return; }
    if (!strcmp(tok.text, "arg")) { arg_wrapper(stack); return; }
    if (!strcmp(tok.text, "re2c")) { real2complex(stack); return; }
    if (!strcmp(tok.text, "j2r")) {  join_2_reals(stack); return; }
    if (!strcmp(tok.text, "ln")) { ln_wrapper(stack); return; }
    if (!strcmp(tok.text, "log")) { log_wrapper(stack); return; }
    if (!strcmp(tok.text, "sqrt")) { sqrt_wrapper(stack); return; }   
    
    // Matrix reduction functions
    for (int i = 0; reduce_ops[i].name != NULL; ++i) {
      if (!strcmp(tok.text, reduce_ops[i].name)) {
        matrix_reduce(stack, reduce_ops[i].axis, reduce_ops[i].operation);
        return;
      }
    }
    return;  
  case TOK_VERTICAL:
    printf("| \n");
    return;
  case TOK_UNKNOWN:
    printf("Illegal token.\n");
    return;
  default:
    printf("Unhandled token type.\n");
    return;
    }
  }
}

