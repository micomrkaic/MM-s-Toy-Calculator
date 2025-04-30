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


// An implementation of a heterogenous stack and some operations for an RPN calculator
// Mico Mrkaic, mrkaic.mico@gmail.com
// Latest version: 0.7
// Date: April 24, 2025

#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <complex.h>
#include <ctype.h>
#include <stdbool.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_complex_math.h>
#include <gsl/gsl_blas.h>         // For gsl_blas_dgemm, gsl_blas_zgemm
#include <gsl/gsl_linalg.h>       // For LU decomposition/inversion
#include <gsl/gsl_permutation.h>  // For gsl_permutation and related
#include "stack.h"
#include "string_fun.h"
#include "math_fun.h"

void init_stack(Stack* stack) {
  stack->top = -1;
}

void push_real(Stack* stack, double value) {
  if (stack->top >= STACK_SIZE - 1) {
    printf("Stack overflow\n");
    return;
  }
  stack->top++;
  stack->items[stack->top].type = TYPE_REAL;
  stack->items[stack->top].real = value;
}

void push_complex(Stack* stack, double complex value) {
  if (stack->top >= STACK_SIZE - 1) {
    printf("Stack overflow\n");
    return;
  }
  stack->top++;
  stack->items[stack->top].type = TYPE_COMPLEX;
  stack->items[stack->top].complex_val = value;
}

void push_string(Stack* stack, const char* str) {
  if (stack->top >= STACK_SIZE - 1) {
    printf("Stack overflow\n");
    return;
  }
  stack->top++;
  stack->items[stack->top].type = TYPE_STRING;
  stack->items[stack->top].string = strdup(str);
  if (!stack->items[stack->top].string) {
    printf("Memory allocation failed\n");
    stack->top--;
  }
}

void push_matrix_real(Stack* stack, gsl_matrix* matrix) {
  if (stack->top >= STACK_SIZE - 1) {
    printf("Stack overflow\n");
    return;
  }
  if (NULL == matrix) {
    printf("NULL pointer to matrix, exiting!\n");
    return;
  }
  stack->top++;
  stack->items[stack->top].type = TYPE_MATRIX_REAL;
  stack->items[stack->top].matrix_real = matrix;
}

void push_matrix_complex(Stack* stack, gsl_matrix_complex* matrix) {
  if (stack->top >= STACK_SIZE - 1) {
    printf("Stack overflow\n");
    return;
  }
  stack->top++;
  stack->items[stack->top].type = TYPE_MATRIX_COMPLEX;
  stack->items[stack->top].matrix_complex = matrix;
}

StackElement pop(Stack* stack) {
  StackElement popped;
  if (stack->top < 0) {
    printf("Stack underflow\n");
    popped.type = TYPE_REAL;
    popped.real = 0.0;
    return popped;
  }
  popped = stack->items[stack->top];
  stack->top--;
  return popped;
}

void swap(Stack* stack) {
  if (stack->top < 1) {
    printf("Too few elements on stack!\n");
  } else {
    StackElement temp_top = stack->items[stack->top];
    StackElement temp_second = stack->items[stack->top-1];
    stack->items[stack->top] = temp_second;
    stack->items[stack->top-1] = temp_top;
  }
}

int dup(Stack* stack) {
  if (stack->top < 0) {
    printf("Stack is empty! Cannot duplicate.\n");
    return -1; // Error
  }
  if (stack->top + 1 >= STACK_SIZE) {
    printf("Stack overflow! Cannot duplicate.\n");
    return -1; // Error
  }
  if ((stack->items[stack->top].type == TYPE_REAL) || (stack->items[stack->top].type == TYPE_COMPLEX)) {
    stack->items[stack->top + 1] = stack->items[stack->top];
    stack->top++;
  }
  if (stack->items[stack->top].type == TYPE_MATRIX_REAL) {
    stack->items[stack->top + 1].type = TYPE_MATRIX_REAL;  
    size_t rows = stack->items[stack->top].matrix_real->size1;
    size_t cols = stack->items[stack->top].matrix_real->size2;
    stack->items[stack->top + 1].matrix_real = gsl_matrix_alloc(rows, cols);
    gsl_matrix_memcpy(stack->items[stack->top + 1].matrix_real,stack->items[stack->top].matrix_real);
    stack->top++;
  }
  if (stack->items[stack->top].type == TYPE_MATRIX_COMPLEX) {
    stack->items[stack->top + 1].type = TYPE_MATRIX_COMPLEX;
    size_t rows = stack->items[stack->top].matrix_complex->size1;
    size_t cols = stack->items[stack->top].matrix_complex->size2;
    stack->items[stack->top + 1].matrix_complex = gsl_matrix_complex_alloc(rows, cols);
    gsl_matrix_complex_memcpy(stack->items[stack->top + 1].matrix_complex,stack->items[stack->top].matrix_complex);
    stack->top++;
  }
  if (stack->items[stack->top].type == TYPE_STRING) {
    stack->items[stack->top + 1].type = TYPE_STRING;
    stack->items[stack->top + 1].string = strdup(stack->items[stack->top].string);
    stack->top++;
  }
  return 0; // Success
}

StackElement check_top(Stack* stack) {
  StackElement top;
  if (stack->top < 0) {
    printf("Stack underflow\n");
    top.type = TYPE_REAL;
    top.real = 0.0;
    return top;
  }
  top = stack->items[stack->top];
  return top;
}

StackElement pop_and_free(Stack* stack) {
  StackElement popped;
  if (stack->top < 0) {
    printf("Stack underflow\n");
    popped.type = TYPE_REAL;
    popped.real = 0.0;
    return popped;
  }
  popped = stack->items[stack->top];
  stack->top--;
  return popped;
}

StackElement* view_top(Stack* stack) {
  if (stack->top < 0) {
    printf("Stack is empty\n");
    return NULL;
  }
  return &stack->items[stack->top];
}

void free_stack(Stack* stack) {
  while (stack->top >= 0) {
    switch (stack->items[stack->top].type) {
    case TYPE_STRING:
      free(stack->items[stack->top].string);
      break;
    case TYPE_MATRIX_REAL:
      gsl_matrix_free(stack->items[stack->top].matrix_real);
      break;
    case TYPE_MATRIX_COMPLEX:
      gsl_matrix_complex_free(stack->items[stack->top].matrix_complex);
      break;
    default:
      break;
    }
    stack->top--;
  }
}

void print_stack(const Stack* stack) {
  if (stack->top == -1) {
    printf("{}\n");
    return;
  }
  printf("\n");
  for (int i = 0; i <= stack->top; i++) {
    switch (stack->items[i].type) {
    case TYPE_REAL:
      printf("[%d] ℝ : %g\n", i, stack->items[i].real);
      //    printf("Set of real numbers: \u211D\n");    // Unicode escape (if compiler supports it)
      break;
    case TYPE_COMPLEX:
      printf("[%d] ℂ : (%g, %gi)\n", i, creal(stack->items[i].complex_val), cimag(stack->items[i].complex_val));
      break;
    case TYPE_STRING:
      printf("[%d] 𝒮 : \"%s\"\n", i, stack->items[i].string);
      break;
    case TYPE_MATRIX_REAL:
      printf("[%d] Mℝ: %zu x %zu matrix\n", i, stack->items[i].matrix_real->size1, stack->items[i].matrix_real->size2);
      break;
    case TYPE_MATRIX_COMPLEX:
      printf("[%d] Mℂ: %zu x %zu matrix\n", i, stack->items[i].matrix_complex->size1, stack->items[i].matrix_complex->size2);
      break;
    }
  }
}

void print_real_matrix(const gsl_matrix* m) {
    size_t rows = m->size1;
    size_t cols = m->size2;
    int col_width[cols];

    // Step 1: find max width per column
    for (size_t j = 0; j < cols; ++j) {
        col_width[j] = 0;
        for (size_t i = 0; i < rows; ++i) {
            char buf[64];
            double val = gsl_matrix_get(m, i, j);
            if (fabs(val) > 1e5 || (fabs(val) > 0 && fabs(val) < 1e-4)) {
                snprintf(buf, sizeof(buf), "%.4e", val);
            } else {
                snprintf(buf, sizeof(buf), "%.4f", val);
            }
            int len = (int)strlen(buf);
            if (len > col_width[j]) col_width[j] = len;
        }
    }

    // Step 2: print each row
    for (size_t i = 0; i < rows; ++i) {
        printf("| ");
        for (size_t j = 0; j < cols; ++j) {
            double val = gsl_matrix_get(m, i, j);
            if (fabs(val) > 1e5 || (fabs(val) > 0 && fabs(val) < 1e-4)) {
                printf("%*.*e ", col_width[j], 4, val);
            } else {
                printf("%*.*f ", col_width[j], 4, val);
            }
        }
        printf("|\n");
    }
}

void print_complex_matrix(const gsl_matrix_complex* m) {
    size_t rows = m->size1;
    size_t cols = m->size2;
    int col_width[cols];

    // Step 1: find max width per column
    for (size_t j = 0; j < cols; ++j) {
        col_width[j] = 0;
        for (size_t i = 0; i < rows; ++i) {
            char buf[128];
            gsl_complex z = gsl_matrix_complex_get(m, i, j);
            double re = GSL_REAL(z);
            double im = GSL_IMAG(z);

            if ((fabs(re) > 1e5 || (fabs(re) > 0 && fabs(re) < 1e-4)) ||
                (fabs(im) > 1e5 || (fabs(im) > 0 && fabs(im) < 1e-4))) {
                snprintf(buf, sizeof(buf), "(%.4e,%.4e)", re, im);
            } else {
                snprintf(buf, sizeof(buf), "(%.4f,%.4f)", re, im);
            }
            int len = (int)strlen(buf);
            if (len > col_width[j]) col_width[j] = len;
        }
    }

    // Step 2: print each row
    for (size_t i = 0; i < rows; ++i) {
        printf("| ");
        for (size_t j = 0; j < cols; ++j) {
            gsl_complex z = gsl_matrix_complex_get(m, i, j);
            double re = GSL_REAL(z);
            double im = GSL_IMAG(z);

            if ((fabs(re) > 1e5 || (fabs(re) > 0 && fabs(re) < 1e-4)) ||
                (fabs(im) > 1e5 || (fabs(im) > 0 && fabs(im) < 1e-4))) {
                printf("%*s ", col_width[j], "");
                printf("(%.4e,%.4e)", re, im);
            } else {
                printf("%*s ", col_width[j], "");
                printf("(%.4f,%.4f)", re, im);
            }
        }
        printf("|\n");
    }
}


gsl_matrix* load_matrix_from_file(int rows, int cols, const char* filename) {
  FILE* f = fopen(filename, "r");
  if (!f) {
    perror("Failed to open matrix file");
    return NULL;
  }

  gsl_matrix* m = gsl_matrix_alloc(rows, cols);
  if (!m) {
    fclose(f);
    fprintf(stderr, "Failed to allocate matrix.\n");
    return NULL;
  }

  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < cols; ++j) {
      double val;
      if (fscanf(f, "%lf", &val) != 1) {
	fprintf(stderr, "Failed to read value at [%d, %d] from file '%s'\n", i, j, filename);
	gsl_matrix_free(m);
	fclose(f);
	return NULL;
      }
      gsl_matrix_set(m, i, j, val);
    }
  }

  fclose(f);
  return m;
}

#include <stdio.h>

ValueType stack_top_type(const Stack* stack) {
  if (stack->top < 0) {
    fprintf(stderr, "Stack is empty\n");
    return -1;  // Or some sentinel value you define
  }
  return stack->items[stack->top].type;
}
ValueType stack_next2_top_type(const Stack* stack) {
  if (stack->top < 0) {
    fprintf(stderr, "Stack is empty\n");
    return -1;  // Or some sentinel value you define
  }
  return stack->items[stack->top-1].type;
}

int save_stack_to_file(Stack* stack, const char* filename) {
  FILE* file = fopen(filename, "wb");
  if (!file) {
    perror("fopen");
    return -1;
  }

  // Save the number of elements
  if (fwrite(&stack->top, sizeof(int), 1, file) != 1) {
    perror("fwrite top");
    fclose(file);
    return -1;
  }

  for (int i = 0; i <= stack->top; ++i) {
    StackElement* elem = &stack->items[i];

    // Save the type first
    if (fwrite(&elem->type, sizeof(ValueType), 1, file) != 1) {
      perror("fwrite type");
      fclose(file);
      return -1;
    }

    switch (elem->type) {
    case TYPE_REAL:
      if (fwrite(&elem->real, sizeof(double), 1, file) != 1) {
	perror("fwrite real");
	fclose(file);
	return -1;
      }
      break;

    case TYPE_COMPLEX:
      if (fwrite(&elem->complex_val, sizeof(double complex), 1, file) != 1) {
	perror("fwrite complex");
	fclose(file);
	return -1;
      }
      break;

    case TYPE_STRING: {
      size_t len = strlen(elem->string) + 1; // Include null terminator
      if (fwrite(&len, sizeof(size_t), 1, file) != 1 ||
	  fwrite(elem->string, sizeof(char), len, file) != len) {
	perror("fwrite string");
	fclose(file);
	return -1;
      }
      break;
    }

    case TYPE_MATRIX_REAL: {
      size_t rows = elem->matrix_real->size1;
      size_t cols = elem->matrix_real->size2;
      if (fwrite(&rows, sizeof(size_t), 1, file) != 1 ||
	  fwrite(&cols, sizeof(size_t), 1, file) != 1 ||
	  fwrite(elem->matrix_real->data, sizeof(double), rows * cols, file) != rows * cols) {
	perror("fwrite matrix_real");
	fclose(file);
	return -1;
      }
      break;
    }

    case TYPE_MATRIX_COMPLEX: {
      size_t rows = elem->matrix_complex->size1;
      size_t cols = elem->matrix_complex->size2;
      if (fwrite(&rows, sizeof(size_t), 1, file) != 1 ||
	  fwrite(&cols, sizeof(size_t), 1, file) != 1 ||
	  fwrite(elem->matrix_complex->data, sizeof(double), 2 * rows * cols, file) != 2 * rows * cols) {
	perror("fwrite matrix_complex");
	fclose(file);
	return -1;
      }
      break;
    }

    default:
      fprintf(stderr, "Unknown type: %d\n", elem->type);
      fclose(file);
      return -1;
    }
  }

  fclose(file);
  return 0;
}

int load_stack_from_file(Stack* stack, const char* filename) {
  FILE* file = fopen(filename, "rb");
  if (!file) {
    perror("fopen");
    return -1;
  }

  // Read the number of elements (top index)
  if (fread(&stack->top, sizeof(int), 1, file) != 1) {
    perror("fread top");
    fclose(file);
    return -1;
  }

  for (int i = 0; i <= stack->top; ++i) {
    StackElement* elem = &stack->items[i];

    // Read type
    if (fread(&elem->type, sizeof(ValueType), 1, file) != 1) {
      perror("fread type");
      fclose(file);
      return -1;
    }

    switch (elem->type) {
    case TYPE_REAL:
      if (fread(&elem->real, sizeof(double), 1, file) != 1) {
	perror("fread real");
	fclose(file);
	return -1;
      }
      break;

    case TYPE_COMPLEX:
      if (fread(&elem->complex_val, sizeof(double complex), 1, file) != 1) {
	perror("fread complex");
	fclose(file);
	return -1;
      }
      break;

    case TYPE_STRING: {
      size_t len;
      if (fread(&len, sizeof(size_t), 1, file) != 1) {
	perror("fread string length");
	fclose(file);
	return -1;
      }

      elem->string = malloc(len);
      if (!elem->string) {
	perror("malloc string");
	fclose(file);
	return -1;
      }

      if (fread(elem->string, sizeof(char), len, file) != len) {
	perror("fread string");
	free(elem->string);
	fclose(file);
	return -1;
      }
      break;
    }

    case TYPE_MATRIX_REAL: {
      size_t rows, cols;
      if (fread(&rows, sizeof(size_t), 1, file) != 1 ||
	  fread(&cols, sizeof(size_t), 1, file) != 1) {
	perror("fread matrix_real size");
	fclose(file);
	return -1;
      }

      elem->matrix_real = gsl_matrix_alloc(rows, cols);
      if (!elem->matrix_real) {
	fprintf(stderr, "Failed to allocate matrix_real\n");
	fclose(file);
	return -1;
      }

      if (fread(elem->matrix_real->data, sizeof(double), rows * cols, file) != rows * cols) {
	perror("fread matrix_real data");
	gsl_matrix_free(elem->matrix_real);
	fclose(file);
	return -1;
      }
      break;
    }

    case TYPE_MATRIX_COMPLEX: {
      size_t rows, cols;
      if (fread(&rows, sizeof(size_t), 1, file) != 1 ||
	  fread(&cols, sizeof(size_t), 1, file) != 1) {
	perror("fread matrix_complex size");
	fclose(file);
	return -1;
      }

      elem->matrix_complex = gsl_matrix_complex_alloc(rows, cols);
      if (!elem->matrix_complex) {
	fprintf(stderr, "Failed to allocate matrix_complex\n");
	fclose(file);
	return -1;
      }

      if (fread(elem->matrix_complex->data, sizeof(double), 2 * rows * cols, file) != 2 * rows * cols) {
	perror("fread matrix_complex data");
	gsl_matrix_complex_free(elem->matrix_complex);
	fclose(file);
	return -1;
      }
      break;
    }

    default:
      fprintf(stderr, "Unknown type: %d\n", elem->type);
      fclose(file);
      return -1;
    }
  }

  fclose(file);
  return 0;
}
