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

#ifndef STACK_H
#define STACK_H

#define _POSIX_C_SOURCE 200809L

#include <string.h> 
#include <complex.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_complex_math.h>

#define STACK_SIZE 100

#define GUARANTEE_STACK(stack, n)					\
  do {									\
    if ((stack)->top + 1 < (n)) {					\
      fprintf(stderr, "Error: need at least %d items on stack.\n", (n)); \
      return 0;								\
    }									\
  } while (0)

typedef enum {
  TYPE_REAL,
  TYPE_COMPLEX,
  TYPE_STRING,
  TYPE_MATRIX_REAL,
  TYPE_MATRIX_COMPLEX
} ValueType;

typedef struct {
  ValueType type;
  union {
    double real;
    gsl_complex complex_val;
    char* string;
    gsl_matrix* matrix_real;
    gsl_matrix_complex* matrix_complex;
  };
} StackElement;

typedef struct {
  StackElement items[STACK_SIZE];
  int top;
} Stack;

void init_stack(Stack* stack);
int stack_size(const Stack* stack);
void push_real(Stack* stack, double value);
void push_complex(Stack* stack, gsl_complex value);
void push_string(Stack* stack, const char* str);
void push_matrix_real(Stack* stack, gsl_matrix* matrix);
void push_matrix_complex(Stack* stack, gsl_matrix_complex* matrix);
StackElement pop(Stack* stack);
int dup(Stack* stack);
void swap(Stack* stack);
StackElement check_top(Stack* stack);
StackElement pop_and_free(Stack* stack);
StackElement* view_top(Stack* stack);
void free_stack(Stack* stack);
gsl_matrix* load_matrix_from_file(int rows, int cols, const char* filename);
ValueType stack_top_type(const Stack* stack);
ValueType stack_next2_top_type(const Stack* stack);
int save_stack_to_file(Stack* stack, const char* filename);
int load_stack_from_file(Stack* stack, const char* filename);
int copy_stack(Stack* dest, const Stack* src);

#endif // STACK_H

// Stack manipulation functions
void stack_nip(Stack* stack);   // Drop second item from top
void stack_tuck(Stack* stack);  // Duplicate second item and push it on top
void stack_over(Stack* stack);  // Copy second item to top
void stack_roll(Stack* stack, int n); // Roll nth item to top (0 = top)
