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

/* **** Still to do as of June 4, 2025 ****
   . silent mode to skip error warnings
   . Overlay for registers -- store varibles; recall values with <= 
   . load program, list program, run program -> separate instructions
   . implement loop counters and easier comparison registers for iterations;
   . fully implement counters and tests
   . fix buffer overruns in load_program
   . clean up and consolidate binary_fun.c; cleanup the dispatch table
   . Test full HP-41 style programming with GTO, RTN, XEQ, ISG, DSE, LBL etc. and labels
   . clean up the interpreter to have only one dispatch table in the VM
   . select submatrices; resize matrices and add/remove rows and/or columns
   . ignore NANs in a smart way in reduce_ops;
   . Write documentation
   . check if name is already defined and reject the definition if it is
   . sto ind, rcl ind.
   
   . WOULD BE NICE
   . Automatic cleanup of matrices with __cleanup__ 
   . fft (nice to have, but not a must).
   . add loading of data frames; turn on the PMS mode 
   . (Nice to have) load a CSV file into a dataframe; add dataframe as a stack object
   
*/

#define _POSIX_C_SOURCE 200809L
#define HISTORY_FILE ".rpn_history"
#define CONFIG_FILE "../data/config.txt"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/wait.h> // for WIFEXITED, WEXITSTATUS, etc.
#include <gsl/gsl_rng.h>
#include <gsl/gsl_errno.h>
#include <readline/history.h>
#include <readline/readline.h>
#include "lexer.h"
#include "stack.h"
#include "eval_fun.h" 
#include "registers.h" 
#include "splash_and_help.h" 
#include "tab_completion.h" 
#include "print_fun.h" 
#include "words.h" 
#include "run_machine.h"

// Globals
gsl_rng * global_rng; // Global random number generator, used throughout the program
Register registers[MAX_REG];  // Global or static array

void my_error_handler(const char *reason, const char *file, int line, int gsl_errno) {
  fprintf(stderr, "GSL ERROR: %s (%s:%d) [code=%d]\n", reason, file, line, gsl_errno);
  // Do NOT call abort(); this allows graceful recovery
}


int repl(void) {

  Stack stack;
  Stack old_stack;

  // Initialize everything needed
  splash_screen();
  init_stack(&stack);
  init_stack(&old_stack);
  init_registers();
  load_macros_from_file();
  if (verbose_mode) list_macros();
  load_config(CONFIG_FILE);
  read_history(HISTORY_FILE);
  stifle_history(1000);  // Keep only the last 1000 commands 
  rl_attempted_completion_function = function_name_completion;

  // The main REPL loop
  while (1) {
    char* line = readline("MM_RPN>> ");
    if (!line || strcmp(line, "q") == 0) {
      free(line);
      break;
    }
    if (*line) add_history(line);

    if (line[0] == '!') {
      int ret = system(line + 1);
      if (ret == -1) {
        perror("system");
      } else if (WIFEXITED(ret) && WEXITSTATUS(ret) != 0) {
        fprintf(stderr, "Command exited with status %d\n", WEXITSTATUS(ret));
      }
      free(line);
      continue;
    }
   
    if (!strcmp(line, "undo")) // Restore the old stack
      { copy_stack(&stack, &old_stack);
      } else {
      copy_stack(&old_stack, &stack);  // Preserve the stack
      evaluate_line(&stack, line);
    }
    if (completed_batch)
      completed_batch = false;
    else
      if (!skip_stack_printing) print_stack(&stack,NULL);
    skip_stack_printing = false;
    free(line);
  }

  // Save config, history, and cleanup
  save_config("../data/config.txt");
  write_history(HISTORY_FILE);
  free_stack(&old_stack);
  free_stack(&stack);
  free_all_registers();
  return 0;
}

int main(void) {
  global_rng = gsl_rng_alloc(gsl_rng_mt19937); // Init random number generation
  gsl_set_error_handler(&my_error_handler);
    
  repl();
  return 0;
}
