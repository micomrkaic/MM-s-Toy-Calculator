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
  Still to do
  1. Registers: save and load
  2. Polynomials: zeros and evaluations
  3. Interpret strings as functions
  4. Readline keystroke shortcuts.
  5. Combine utility functions in one file; combine all initializations in one function.
  6. Iterations
  7. Numerical integration of 1-D functions
  8. Write documentation
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <complex.h>
#include <time.h>
#include <readline/history.h>
#include <readline/readline.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include "lexer.h"
#include "stack.h"
#include "string_fun.h"
#include "math_fun.h"
#include "binary_fun.h"
#include "unary_fun.h"
#include "eval_fun.h"
#include "registers.h"
#include "poly_fun.h"

gsl_rng * global_rng; // Global random number generator, used throughout the program
Register registers[MAX_REG];  // Global or static array

void splash_screen() {
  time_t now = time(NULL);
  printf("\n");
  printf("Mico's toy Matrix and Scalar RPN Calculator\n");
  printf("Version alpha 0.1, 2025 \n");
  printf("Current time and date: %s", ctime(&now));
  printf("Enter expressions in RPN (type 'exit' to quit, 'help' for help).\n");
  printf("\n");
}

int main() {
  Stack stack;

  global_rng = gsl_rng_alloc(gsl_rng_mt19937); // Init random number generation
  splash_screen();
  init_stack(&stack);
  init_registers();
  
  // The main REPL loop
  while (1) {
    char* line = readline("MM_RPN>> ");
    if (!line || strcmp(line, "exit") == 0) {
      free(line);
      break;
    }
    if (*line) add_history(line);
    evaluator(&stack,line);
    free(line);
    print_stack(&stack);	  
  }
  free_stack(&stack);
  free_all_registers();
  return 0;
}
