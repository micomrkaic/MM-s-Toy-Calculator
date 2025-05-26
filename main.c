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

// Define words in this style
//<sq | dup * >
//<qu | sq sq >


/*
  Still to do as of May 26, 2025
  . make .h header files clean
  . stack: nip, tuck, roll, over
  . clean up and consolidate binary_fun.c
  . User defined words and
  . fix eval loop; make sure eval_fun exits nicely and is recursive
  . Full HP-41 style programming with GTO, RTN, XEQ, ISG, DSE, LBL etc. and labels
  . select submatrices; resize matrices and add/remove rows and/or columns
  . matrix norm (frobenius, 2-norm), condest; check if 
  . ignore NANs in a smart way in reduce_ops; 
  . Financial functions: PV,  NPV, IRR, n, etc a la HP-12C; depreciation and amortization
  . statistics functions: sample quantiles, correlation etc.
  . add % and %chg to the basic functions
  . ranges a la Matlab 1:5!
  . sto+, sto-, sto*, sto/, sto ind, rcl ind.
  . promote type correctly like sqrt(-1) switches to complex for all arguments
  . consolidate gsl_complex and double complex
  . Automatic cleanup of matrices with __cleanup__
  . Registers: save and load
  . Integral and zero finding
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
