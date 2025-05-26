#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <complex.h>
#include <stdbool.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_matrix_complex.h>
#include <gsl/gsl_complex_math.h>
#include "stack"


StackElement peek(Stack* stack) {
  return stack->items[stack->top];
}

// === Smart Real Wrappers ===
void apply_real_sqrt(Stack* stack) {
  StackElement a = pop(stack);
  if (a.real >= 0.0) push_real(stack, sqrt(a.real));
  else push_complex(stack, csqrt(a.real));
}

void apply_real_log(Stack* stack) {
  StackElement a = pop(stack);
  if (a.real > 0.0) push_real(stack, log(a.real));
  else push_complex(stack, clog(a.real));
}

void apply_real_log10(Stack* stack) {
  StackElement a = pop(stack);
  if (a.real > 0.0) push_real(stack, log10(a.real));
  else push_complex(stack, clog(a.real) / log(10.0));
}

// === Matrix Promotion Example ===
void apply_real_matrix_sqrt(Stack* stack) {
  StackElement a = pop(stack);
  gsl_matrix* m = a.matrix_real;
  size_t rows = m->size1, cols = m->size2;
  bool promote = false;

  for (size_t i = 0; i < rows && !promote; ++i)
    for (size_t j = 0; j < cols; ++j)
      if (gsl_matrix_get(m, i, j) < 0) {
        promote = true; break;
      }

  if (promote) {
    gsl_matrix_complex* out = gsl_matrix_complex_alloc(rows, cols);
    for (size_t i = 0; i < rows; ++i)
      for (size_t j = 0; j < cols; ++j) {
        double v = gsl_matrix_get(m, i, j);
        gsl_complex z = gsl_complex_sqrt(gsl_complex_rect(v, 0));
        gsl_matrix_complex_set(out, i, j, z);
      }
    push_matrix_complex(stack, out);
  } else {
    gsl_matrix* out = gsl_matrix_alloc(rows, cols);
    for (size_t i = 0; i < rows; ++i)
      for (size_t j = 0; j < cols; ++j)
        gsl_matrix_set(out, i, j, sqrt(gsl_matrix_get(m, i, j)));
    push_matrix_real(stack, out);
  }
}

// === Generic Unary Application ===
void apply_real_unary(Stack* stack, double (*f)(double)) {
  StackElement a = pop(stack);
  push_real(stack, f(a.real));
}

void apply_complex_unary(Stack* stack, double complex (*f)(double complex)) {
  StackElement a = pop(stack);
  push_complex(stack, f(a.complex_val));
}

void apply_real_matrix_unary_inplace(Stack* stack, double (*f)(double)) {
  // Implement as needed
}

void apply_complex_matrix_unary_inplace(Stack* stack, double complex (*f)(double complex)) {
  // Implement as needed
}

// === Dispatch Table ===
typedef struct {
  const char* name;
  void (*real_fn)(Stack*);
  void (*complex_fn)(Stack*);
  void (*matrix_real_fn)(Stack*);
  void (*matrix_complex_fn)(Stack*);
} UnaryFuncDispatcher;

void sin_real_wrapper(Stack* s) { apply_real_unary(s, sin); }
void sin_complex_wrapper(Stack* s) { apply_complex_unary(s, csin); }
// ... more wrappers ...

UnaryFuncDispatcher unary_dispatch_table[] = {
  {"sin", sin_real_wrapper, sin_complex_wrapper, NULL, NULL},
  {"sqrt", apply_real_sqrt, apply_complex_unary, apply_real_matrix_sqrt, apply_complex_matrix_unary_inplace},
  {"ln", apply_real_log, apply_complex_unary, NULL, NULL},
  {"log", apply_real_log10, apply_complex_unary, NULL, NULL},
  {NULL, NULL, NULL, NULL, NULL}  // sentinel
};

// === Dispatcher ===
void dispatch_unary_function(const char* name, Stack* stack) {
  ValueType type = peek(stack).type;
  for (int i = 0; unary_dispatch_table[i].name; ++i) {
    if (strcmp(name, unary_dispatch_table[i].name) == 0) {
      switch (type) {
        case TYPE_REAL:
          if (unary_dispatch_table[i].real_fn) unary_dispatch_table[i].real_fn(stack);
          else fprintf(stderr, "%s not defined for real\n", name);
          return;
        case TYPE_COMPLEX:
          if (unary_dispatch_table[i].complex_fn) unary_dispatch_table[i].complex_fn(stack);
          else fprintf(stderr, "%s not defined for complex\n", name);
          return;
        case TYPE_MATRIX_REAL:
          if (unary_dispatch_table[i].matrix_real_fn) unary_dispatch_table[i].matrix_real_fn(stack);
          else fprintf(stderr, "%s not defined for real matrix\n", name);
          return;
        case TYPE_MATRIX_COMPLEX:
          if (unary_dispatch_table[i].matrix_complex_fn) unary_dispatch_table[i].matrix_complex_fn(stack);
          else fprintf(stderr, "%s not defined for complex matrix\n", name);
          return;
        default:
          fprintf(stderr, "Unsupported type for %s\n", name);
          return;
      }
    }
  }
  fprintf(stderr, "Unknown function: %s\n", name);
}
