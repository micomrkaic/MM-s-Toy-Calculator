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
#include "math_fun.h"
#include "binary_fun.h"
#include "unary_fun.h"
#include "stat_fun.h"
#include "eval_fun.h"
#include "poly_fun.h"
#include "registers.h"

int read_complex(const char* input, double complex* z) {
  double real, imag;
  if (sscanf(input, " ( %lf , %lf ) ", &real, &imag) == 2) {
    *z = real + imag * I;
    return 1; // success
  }
  return 0; // failure
}

double negate_real(double x) {
  return -x;
}
double complex negate_complex(double complex x) {
  return -x;
}

double one_over_real(double x) {
  return 1.0/x;
}
double complex one_over_complex(double complex x) {
  return 1.0/x;
}


void help_menu() {
  //  printf("\nMico's toy Matrix and Scalar RPN Calculator\n");
  printf("RPN Calculator for real and complex scalars and matrices\n");
  printf("All inputs are case sensitive. Enter strings as \"string\".\n");
  printf("Enter complex numbers as in: (1,3) or (-1.2e-4, 0.7e2)\n");
  printf("Enter inline matrices as in J language [#rows #cols $ values].\n");
  printf("Matrix entries can be real or complex.\n");
  printf("Read matrix from file as [#rows, #cols, \"filename\"]\n");
  printf("Get individual matrix elements with get_aij; set them with set_aij.\n");  
  printf("Special matrices: eye, ones, rand, randn.\n");  
  printf("Basic matrix statistics: csum, rsum, cmean, rmean, cvar, rvar.\n");  
  printf("Basic probability: npdf, ncdf, nquant [quantiles]\n");
  printf("Trig functions: sin, cos, tan, asin, acos, atan\n");
  printf("Hyperbolic functions: sinh, cosh, tanh, asinh, acosh, atanh\n");
  printf("Other math functions: ln, exp, log, chs, inv [1/x], ^ [power]\n");
  printf("Polynomials: evaluation and zeros\n");
  printf("String functions: concat, s2u [to upper], s2l [to lower], slen, srev [reverse]\n");
  printf("Stack functions: drop, dup, swap, clst\n");
  printf("Complex functions: re, im, abs, arg, re2c, split_c, j2r [join 2 reals into complex]\n");
  printf("Constants: pi, e, gravity [9.81]\n");
  printf("Registers to store stack elements: sto, rcl, {save}, {load} \n");
  printf("\n **** Still to be done ****\n");
  printf("Statistics (colmean, rowmean,...); random numbers \n");
  printf("Polynomials: zeros and ealuations\n");
  printf("Set print precision; readline keystroke shortcuts.\n");
  printf("\n");
 }

int evaluator(Stack *stack, char* line) {
  Lexer lexer = {line, 0};
  Token tok;

  // The lexer loop
  do {
    tok = next_token(&lexer);
#ifdef PRINT_TOKEN_FOR_DEBUGGING
    printf("%-20s %s\n", token_type_str(tok.type), tok.text);
#endif    
    switch (tok.type) {
    case TOK_EOF:
      break;
    case TOK_NUMBER:
      push_real(stack, atof(tok.text));
      break;
    case TOK_COMPLEX:
      double complex z;
      if (read_complex(tok.text, &z)) push_complex(stack,z);
      break;
    case TOK_STRING:
      push_string(stack, tok.text);
      break;
    case TOK_MATRIX_FILE:
      int rows, cols;
      char filename[1024];
      if (sscanf(tok.text, "[%d,%d,\"%255[^\"]\"]", &rows, &cols, filename) == 3) {
	printf("rows = %d\n", rows);
	printf("cols = %d\n", cols);
	printf("filename = %s\n", filename);
      } else {
	fprintf(stderr, "Failed to parse input string\n");
      }
      push_matrix_real(stack,load_matrix_from_file(rows, cols, filename));
      break;
    case TOK_MATRIX_INLINE_REAL:
      push_matrix_real(stack,parse_matrix_literal(tok.text));
      break;
    case TOK_MATRIX_INLINE_COMPLEX:
      push_matrix_complex(stack,parse_complex_matrix_literal(tok.text));
      break;
    case TOK_MATRIX_INLINE_MIXED:
      push_matrix_complex(stack,parse_complex_matrix_literal(tok.text));
      break;
    case TOK_PLUS:
      add_top_two(stack);
      break;
    case TOK_MINUS:
      sub_top_two(stack);
      break;
    case TOK_STAR:
      mul_top_two(stack);
      break;
    case TOK_SLASH:
      div_top_two(stack);
      break;
    case TOK_CARET:
      pow_top_two(stack);
      break;
    case TOK_LPAREN:
      printf("< \n");
      break;
    case TOK_RPAREN:
      printf("> \n");
      break;
    case TOK_IDENTIFIER:
      // Constants
      if (!strcmp("gravity",tok.text)) push_real(stack,9.81);
      if (!strcmp("pi",tok.text)) push_real(stack,3.141592654);
      if (!strcmp("e",tok.text)) push_real(stack,exp(1.0));
      // Stack functions
      if (!strcmp("drop",tok.text)) pop_and_free(stack);
      if (!strcmp("clst",tok.text)) free_stack(stack);
      if (!strcmp("swap",tok.text)) swap(stack);
      if (!strcmp("dup",tok.text)) dup(stack);
      // Utility functions
      if (!strcmp("pm",tok.text)) {
	StackElement a = check_top(stack);
	if (a.type == TYPE_MATRIX_REAL) print_real_matrix(a.matrix_real);
	if (a.type == TYPE_MATRIX_COMPLEX) print_complex_matrix(a.matrix_complex);
      }
      // Misc
      if (!strcmp("fuck",tok.text)) printf("Your place or mine?\n");
      if (!strcmp("help",tok.text)) help_menu("Your place or mine?\n");

      // Matrix functions
      if (!strcmp("minv",tok.text)) matrix_inverse(stack);
      if (!strcmp("det",tok.text)) matrix_determinant(stack);
      if (!strcmp("eig",tok.text)) matrix_eigen_decompose(stack);
      if (!strcmp("tran",tok.text)) matrix_transpose(stack);
      if (!strcmp("reshape",tok.text)) reshape_matrix(stack);
      if (!strcmp("get_aij",tok.text)) select_matrix_element(stack);
      if (!strcmp("set_aij",tok.text)) {
	printf("Setting matrix element!\n");
	set_matrix_element(stack);
      }
      if (!strcmp("kron",tok.text)) kronecker_top_two(stack);
      if (!strcmp("diag",tok.text)) matrix_extract_diagonal(stack);    
      if (!strcmp("chol",tok.text)) matrix_cholesky(stack);
      if (!strcmp("svd",tok.text)) matrix_svd(stack);
      if (!strcmp("dim",tok.text)) matrix_dimensions(stack);
      if (!strcmp("eye",tok.text)) make_unit_matrix(stack);
      if (!strcmp("ones",tok.text)) make_matrix_of_ones(stack);
      if (!strcmp("zeroes",tok.text)) make_matrix_of_zeroes(stack);
      if (!strcmp("rand",tok.text)) make_random_matrix(stack);
      if (!strcmp("randn",tok.text)) make_gaussian_random_matrix(stack);
      // Stats on matrices
      if (!strcmp("cmean",tok.text)) matrix_reduce(stack,"col","mean");
      if (!strcmp("rmean",tok.text)) matrix_reduce(stack,"row","mean");
      if (!strcmp("csum",tok.text)) matrix_reduce(stack,"col","sum");
      if (!strcmp("rsum",tok.text)) matrix_reduce(stack,"row","sum");
      if (!strcmp("cvar",tok.text)) matrix_reduce(stack,"col","var");
      if (!strcmp("rvar",tok.text)) matrix_reduce(stack,"row","var");     

      // Polynomial functions
      if (!strcmp("roots",tok.text)) poly_roots(stack);     
      if (!strcmp("pval",tok.text)) poly_eval(stack);     
      
      // Register functions
      if (!strcmp("rcl",tok.text)) recall_from_register(stack);
      if (!strcmp("sto",tok.text)) store_to_register(stack);
      if (!strcmp("pr",tok.text)) show_registers_status();     

      break;
    case TOK_FUNCTION:
      ValueType top_type=stack_top_type(stack);
      if (top_type == TYPE_STRING) {
	if (!strcmp("scon",tok.text)) concatenate(stack);
	if (!strcmp("s2l",tok.text)) to_lower(stack);
	if (!strcmp("s2u",tok.text)) to_upper(stack);
	if (!strcmp("slen",tok.text)) string_length(stack);
	if (!strcmp("srev",tok.text)) string_reverse(stack);	  
      }
      if (top_type == TYPE_REAL) {
	if (!strcmp("sin",tok.text)) apply_real_unary(stack,sin);
	if (!strcmp("cos",tok.text)) apply_real_unary(stack,cos);
	if (!strcmp("tan",tok.text)) apply_real_unary(stack,tan);
	if (!strcmp("asin",tok.text)) apply_real_unary(stack,asin);
	if (!strcmp("acos",tok.text)) apply_real_unary(stack,acos);
	if (!strcmp("atan",tok.text)) apply_real_unary(stack,atan);
	if (!strcmp("sinh",tok.text)) apply_real_unary(stack,sinh);
	if (!strcmp("cosh",tok.text)) apply_real_unary(stack,cosh);
	if (!strcmp("tanh",tok.text)) apply_real_unary(stack,tanh);
	if (!strcmp("asinh",tok.text)) apply_real_unary(stack,asinh);
	if (!strcmp("acosh",tok.text)) apply_real_unary(stack,acosh);
	if (!strcmp("atanh",tok.text)) apply_real_unary(stack,atanh);
	if (!strcmp("exp",tok.text))  apply_real_unary(stack,exp);
	if (!strcmp("abs",tok.text))  apply_real_unary(stack,fabs);
	if (!strcmp("npdf", tok.text)) apply_real_unary(stack, standard_normal_pdf);
	if (!strcmp("ncdf", tok.text)) apply_real_unary(stack, standard_normal_cdf);
	if (!strcmp("nquant", tok.text)) apply_real_unary(stack, standard_normal_quantile);
	if (!strcmp("re2c", tok.text)) real2complex(stack);
	if (!strcmp("j2r", tok.text)) join_2_reals(stack);
	if (!strcmp("chs",tok.text)) {
	  StackElement a=pop(stack);	    
	  push_real(stack,-(a.real));
	}
	if (!strcmp("inv",tok.text)) {
	  StackElement a=pop(stack);	    
	  if (a.real != 0.0) push_real(stack,1.0/a.real); else {
	    printf("Division by zero not allowed!\n");
	    //	    push_real(stack,a.real);
	  }
	}
	if (!strcmp("ln",tok.text)) {
	  StackElement a=pop(stack);	    
	  if (a.real >= 0.0) push_real(stack,log(a.real)); else push_complex(stack,clog(a.real));
	}
	if (!strcmp("log",tok.text)) {
	  StackElement a=pop(stack);	    
	  if (a.real >= 0.0) push_real(stack,log10(a.real)); else push_complex(stack,clog(a.real)/log(10.0));
	}
	if (!strcmp("sqrt",tok.text)) {
	  StackElement a=pop(stack);	    
	  if (a.real >= 0.0) push_real(stack,sqrt(a.real)); else push_complex(stack,csqrt(a.real));
	}
      }
      if (top_type == TYPE_COMPLEX) {
	if (!strcmp("sin",tok.text)) apply_complex_unary(stack,csin);
	if (!strcmp("cos",tok.text)) apply_complex_unary(stack,ccos);
	if (!strcmp("tan",tok.text)) apply_complex_unary(stack,ctan);
	if (!strcmp("asin",tok.text)) apply_complex_unary(stack,casin);
	if (!strcmp("acos",tok.text)) apply_complex_unary(stack,cacos);
	if (!strcmp("atan",tok.text)) apply_complex_unary(stack,catan);
	if (!strcmp("sinh",tok.text)) apply_complex_unary(stack,csinh);
	if (!strcmp("cosh",tok.text)) apply_complex_unary(stack,ccosh);
	if (!strcmp("tanh",tok.text)) apply_complex_unary(stack,ctanh);
	if (!strcmp("asinh",tok.text)) apply_complex_unary(stack,casinh);
	if (!strcmp("acosh",tok.text)) apply_complex_unary(stack,cacosh);
	if (!strcmp("atanh",tok.text)) apply_complex_unary(stack,catanh);
	if (!strcmp("ln",tok.text)) apply_complex_unary(stack,clog);
	if (!strcmp("log",tok.text)) apply_complex_unary(stack,log10_complex);
	if (!strcmp("exp",tok.text)) apply_complex_unary(stack,cexp);
	if (!strcmp("sqrt",tok.text)) apply_complex_unary(stack,csqrt);
	if (!strcmp("conj",tok.text)) apply_complex_unary(stack,conj);
	if (!strcmp("split_c",tok.text)) split_complex(stack);
	if (!strcmp("chs",tok.text)) {
	  StackElement a=pop(stack);	    
	  push_complex(stack,-(a.complex_val));
	}
	if (!strcmp("inv",tok.text)) {
	  StackElement a=pop(stack);	    
	  if (!is_zero_complex(a.complex_val)) push_complex(stack,1.0/a.complex_val); else {
	    printf("Division by zero not allowed!\n");
	    //	    push_real(stack,a.complex_val);
	  }
	}

	if (!strcmp("abs",tok.text)) {
	  StackElement a = pop(stack);
	  if (a.type == TYPE_REAL) push_real(stack, fabs(a.real));
	  else if (a.type == TYPE_COMPLEX) push_real(stack, cabs(a.complex_val));
	  else printf("abs: unsupported type\n");
	}
	if (!strcmp("re",tok.text)) {
	  StackElement a = pop(stack);
	  if (a.type == TYPE_REAL) push_real(stack, a.real);
	  else if (a.type == TYPE_COMPLEX) push_real(stack, creal(a.complex_val));
	  else printf("re: unsupported type\n");
	}
	if (!strcmp("im",tok.text)) {
	  StackElement a = pop(stack);
	  if (a.type == TYPE_REAL) push_real(stack, a.real);
	  else if (a.type == TYPE_COMPLEX) push_real(stack, cimag(a.complex_val));
	  else printf("im: unsupported type\n");
	}
	if (!strcmp("arg",tok.text)) {
	  StackElement a = pop(stack);
	  if (a.type == TYPE_REAL) push_real(stack, 0.0);
	  else if (a.type == TYPE_COMPLEX) push_real(stack, carg(a.complex_val));
	  else printf("abs: unsupported type\n");
	}	  
      }
      if (top_type == TYPE_MATRIX_COMPLEX) {
	if (!strcmp("sin",tok.text)) apply_complex_matrix_unary_inplace(stack, csin);
	if (!strcmp("cos",tok.text)) apply_complex_matrix_unary_inplace(stack, ccos);
	if (!strcmp("tan",tok.text)) apply_complex_matrix_unary_inplace(stack, ctan);
	if (!strcmp("asin",tok.text)) apply_complex_matrix_unary_inplace(stack, casin);
	if (!strcmp("acos",tok.text)) apply_complex_matrix_unary_inplace(stack, cacos);
	if (!strcmp("atan",tok.text)) apply_complex_matrix_unary_inplace(stack, catan);
	if (!strcmp("sinh",tok.text)) apply_complex_matrix_unary_inplace(stack, csinh);
	if (!strcmp("cosh",tok.text)) apply_complex_matrix_unary_inplace(stack, ccosh);
	if (!strcmp("tanh",tok.text)) apply_complex_matrix_unary_inplace(stack, ctanh);
	if (!strcmp("asinh",tok.text)) apply_complex_matrix_unary_inplace(stack, casinh);
	if (!strcmp("acosh",tok.text)) apply_complex_matrix_unary_inplace(stack, cacosh);
	if (!strcmp("atanh",tok.text)) apply_complex_matrix_unary_inplace(stack, catanh);
	if (!strcmp("ln",tok.text)) apply_complex_matrix_unary_inplace(stack, clog);
	if (!strcmp("log",tok.text)) apply_complex_matrix_unary_inplace(stack, log10_complex);
	if (!strcmp("exp",tok.text)) apply_complex_matrix_unary_inplace(stack, cexp);
	if (!strcmp("sqrt",tok.text)) apply_complex_matrix_unary_inplace(stack, csqrt);
	if (!strcmp("chs",tok.text)) apply_complex_matrix_unary_inplace(stack, negate_complex);
	if (!strcmp("inv",tok.text)) apply_complex_matrix_unary_inplace(stack, one_over_complex);
	if (!strcmp("re",tok.text)) complex_matrix_real_part(stack);
	if (!strcmp("im",tok.text)) complex_matrix_imag_part(stack);
	if (!strcmp("split_c",tok.text)) split_complex(stack);
	if (!strcmp("abs",tok.text)) complex_matrix_abs_by_element(stack);
      }
      
      if (top_type == TYPE_MATRIX_REAL) {
	if (!strcmp("sin",tok.text)) apply_real_matrix_unary_inplace(stack, sin);
	if (!strcmp("cos",tok.text)) apply_real_matrix_unary_inplace(stack, cos);
	if (!strcmp("tan",tok.text)) apply_real_matrix_unary_inplace(stack, tan);
	if (!strcmp("asin",tok.text)) apply_real_matrix_unary_inplace(stack, asin);
	if (!strcmp("acos",tok.text)) apply_real_matrix_unary_inplace(stack, acos);
	if (!strcmp("atan",tok.text)) apply_real_matrix_unary_inplace(stack, atan);
	if (!strcmp("sinh",tok.text)) apply_real_matrix_unary_inplace(stack, sinh);
	if (!strcmp("cosh",tok.text)) apply_real_matrix_unary_inplace(stack, cosh);
	if (!strcmp("tanh",tok.text)) apply_real_matrix_unary_inplace(stack, tanh);
	if (!strcmp("asinh",tok.text)) apply_real_matrix_unary_inplace(stack, asinh);
	if (!strcmp("acosh",tok.text)) apply_real_matrix_unary_inplace(stack, acosh);
	if (!strcmp("atanh",tok.text)) apply_real_matrix_unary_inplace(stack, atanh);
	if (!strcmp("ln",tok.text)) apply_real_matrix_unary_inplace(stack, log);
	if (!strcmp("log",tok.text)) apply_real_matrix_unary_inplace(stack, log10);
	if (!strcmp("exp",tok.text)) apply_real_matrix_unary_inplace(stack, exp);
	if (!strcmp("sqrt",tok.text)) apply_real_matrix_unary_inplace(stack, sqrt);
	if (!strcmp("npdf", tok.text)) apply_real_matrix_unary_inplace(stack, standard_normal_pdf);
	if (!strcmp("ncdf", tok.text)) apply_real_matrix_unary_inplace(stack, standard_normal_cdf);
	if (!strcmp("nquant", tok.text)) apply_real_matrix_unary_inplace(stack, standard_normal_quantile);
	if (!strcmp("abs", tok.text)) apply_real_matrix_unary_inplace(stack, fabs);
	if (!strcmp("chs",tok.text)) apply_real_matrix_unary_inplace(stack, negate_real);
	if (!strcmp("inv",tok.text)) apply_real_matrix_unary_inplace(stack, one_over_real);
	if (!strcmp("re2c", tok.text)) real2complex(stack);
	if (!strcmp("j2r", tok.text)) join_2_reals(stack);
      }
      break;
    case TOK_COMMA:
      printf("| \n");
      break;
    case TOK_UNKNOWN:
      printf("Unknown token.\n");
      break;
    default:
      printf("Unhandled token type.\n");
      break;
    }
  } while (tok.type != TOK_EOF);
return 0;
}
