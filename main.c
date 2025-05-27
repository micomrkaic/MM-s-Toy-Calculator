
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

/*
  Still to do as of May 27, 2025

  . MATH
  . select submatrices; resize matrices and add/remove rows and/or columns
  . ignore NANs in a smart way in reduce_ops;
  . Integral and zero finding
  . fft (nice to have, but not a must).

  . MACROS
  . Financial functions: PV,  NPV, IRR, n, etc a la HP-12C; depreciation and amortization
  . sto+, sto-, sto*, sto/, sto ind, rcl ind.
  . matrix norm (frobenius, 2-norm), condest; check if 
  . load defined words at the startup and have two dictionaries for defined and user-defined words
  . dynamically add tab copletion to the dictionary

  . VM and the system
  . fix buffer overruns in inline_matrix_j
  . run script mode and run program mode -- turn off screen output except when requested
  . make .h header files clean
  . print with paging
  . clean up and consolidate binary_fun.c; cleanup the dispatch table
  . consolidate gsl_complex and double complex
  . Full HP-41 style programming with GTO, RTN, XEQ, ISG, DSE, LBL etc. and labels
  . clean up the interpreter to have only one dispatch table in the VM
  . Automatic cleanup of matrices with __cleanup__

  . OTHER
  . Write documentation
  . (Nice to have) load a CSV file into a dataframe; add dataframe as a stack object
*/

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <gsl/gsl_rng.h>
#include <readline/history.h>
#include <readline/readline.h>
#include "lexer.h"
#include "stack.h"
#include "eval_fun.h" 
#include "registers.h" 
#include "splash_and_help.h" 
#include "tab_completion.h" 
#include "print_fun.h" 

// Globals
gsl_rng * global_rng; // Global random number generator, used throughout the program
Register registers[MAX_REG];  // Global or static array

int main(void) {
  Stack stack;
  Stack old_stack;

  global_rng = gsl_rng_alloc(gsl_rng_mt19937); // Init random number generation
  splash_screen();
  init_stack(&stack);
  init_stack(&old_stack);
  init_registers();

  rl_attempted_completion_function = function_name_completion;

  // The main REPL loop
  while (1) {
    char* line = readline("MM_RPN>> ");
    if (!line || strcmp(line, "q") == 0) {
      free(line);
      break;
    }
    if (*line) add_history(line);
    evaluator(&stack,&old_stack, line);
    print_stack(&stack,NULL);
    free(line);
  }
  free_stack(&old_stack);
  free_stack(&stack);
  free_all_registers();
  return 0;
}
