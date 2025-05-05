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
#include <gsl/gsl_vector_complex.h>      // for gsl_vector_complex
#include <gsl/gsl_eigen.h>        // for eigen decomposition functions
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include "stack.h"
#include "math_fun.h"
#include "binary_fun.h"
#include "unary_fun.h"

static inline gsl_complex to_gsl_complex(double complex z) {
    return gsl_complex_rect(creal(z), cimag(z));
}

static inline double complex to_double_complex(gsl_complex z) {
    return GSL_REAL(z) + GSL_IMAG(z) * I;
}


// Parse real matrices: rows cols $ list of real numbers
gsl_matrix* parse_matrix_literal(const char* input) {
  const char* p = input;
  while (isspace(*p)) p++;

  char* endptr;
  size_t rows = strtoul(p, &endptr, 10);
  if (p == endptr) {
    fprintf(stderr, "Expected number of rows\n");
    return NULL;
  }
  p = endptr;

  while (isspace(*p)) p++;

  size_t cols = strtoul(p, &endptr, 10);
  if (p == endptr) {
    fprintf(stderr, "Expected number of columns\n");
    return NULL;
  }
  p = endptr;

  while (isspace(*p)) p++;

  if (*p != '$') {
    fprintf(stderr, "Expected '$' after rows and columns\n");
    return NULL;
  }
  p++; // skip '$'

  double* data = malloc(rows * cols * sizeof(double));
  if (!data) {
    fprintf(stderr, "Memory allocation failed\n");
    return NULL;
  }

  size_t count = 0;
  while (count < rows * cols && *p) {
    while (isspace(*p)) p++;

    double val = strtod(p, &endptr);
    if (p == endptr) {
      fprintf(stderr, "Invalid real number at entry %zu\n", count);
      free(data);
      return NULL;
    }
    p = endptr;
    data[count++] = val;
  }

  if (count != rows * cols) {
    fprintf(stderr, "Matrix element count mismatch: expected %zu, got %zu\n", rows * cols, count);
    free(data);
    return NULL;
  }

  gsl_matrix* m = gsl_matrix_alloc(rows, cols);
  if (!m) {
    fprintf(stderr, "Matrix allocation failed\n");
    free(data);
    return NULL;
  }

  for (size_t i = 0; i < rows; ++i) {
    for (size_t j = 0; j < cols; ++j) {
      gsl_matrix_set(m, i, j, data[i * cols + j]);
    }
  }

  free(data);
  return m;
}

// Parse mixed real/complex matrices: rows cols $ list of real and complex numbers
gsl_matrix_complex* parse_complex_matrix_literal(const char* input) {
  const char* p = input;
  while (isspace(*p)) p++;

  char* endptr;
  size_t rows = strtoul(p, &endptr, 10);
  if (p == endptr) {
    fprintf(stderr, "Expected number of rows\n");
    return NULL;
  }
  p = endptr;

  while (isspace(*p)) p++;

  size_t cols = strtoul(p, &endptr, 10);
  if (p == endptr) {
    fprintf(stderr, "Expected number of columns\n");
    return NULL;
  }
  p = endptr;

  while (isspace(*p)) p++;

  if (*p != '$') {
    fprintf(stderr, "Expected '$' after rows and columns\n");
    return NULL;
  }
  p++; // skip '$'

  gsl_complex* data = malloc(rows * cols * sizeof(gsl_complex));
  if (!data) {
    fprintf(stderr, "Memory allocation failed\n");
    return NULL;
  }

  size_t count = 0;
  while (count < rows * cols && *p) {
    while (isspace(*p)) p++;

    if (*p == '(') {
      p++; // skip '('
      double real = strtod(p, &endptr);
      if (p == endptr) {
	fprintf(stderr, "Invalid real part at entry %zu\n", count);
	free(data);
	return NULL;
      }
      p = endptr;

      while (isspace(*p)) p++;

      if (*p != ',') {
	fprintf(stderr, "Expected ',' between real and imaginary at entry %zu\n", count);
	free(data);
	return NULL;
      }
      p++; // skip ','

      double imag = strtod(p, &endptr);
      if (p == endptr) {
	fprintf(stderr, "Invalid imaginary part at entry %zu\n", count);
	free(data);
	return NULL;
      }
      p = endptr;

      while (isspace(*p)) p++;

      if (*p != ')') {
	fprintf(stderr, "Expected ')' after complex number at entry %zu\n", count);
	free(data);
	return NULL;
      }
      p++; // skip ')'

      data[count++] = gsl_complex_rect(real, imag);
    } else if (isdigit(*p) || (*p == '-' && isdigit(*(p + 1)))) {
      double real = strtod(p, &endptr);
      if (p == endptr) {
	fprintf(stderr, "Invalid real number at entry %zu\n", count);
	free(data);
	return NULL;
      }
      p = endptr;
      data[count++] = gsl_complex_rect(real, 0.0);
    } else {
      fprintf(stderr, "Unexpected character at entry %zu: '%c'\n", count, *p);
      free(data);
      return NULL;
    }
  }

  if (count != rows * cols) {
    fprintf(stderr, "Matrix element count mismatch: expected %zu, got %zu\n", rows * cols, count);
    free(data);
    return NULL;
  }

  gsl_matrix_complex* m = gsl_matrix_complex_alloc(rows, cols);
  if (!m) {
    fprintf(stderr, "Matrix allocation failed\n");
    free(data);
    return NULL;
  }

  for (size_t i = 0; i < rows; ++i) {
    for (size_t j = 0; j < cols; ++j) {
      gsl_matrix_complex_set(m, i, j, data[i * cols + j]);
    }
  }

  free(data);
  return m;
}

void matrix_inverse(Stack* stack) {
  if (stack->top < 0) {
    printf("No matrix to invert\n");
    return;
  }

  StackElement m = pop(stack);

  if (m.type == TYPE_MATRIX_REAL) {
    size_t n = m.matrix_real->size1;
    if (n != m.matrix_real->size2) {
      printf("Matrix is not square\n");
      return;
    }

    gsl_matrix* inv = gsl_matrix_alloc(n, n);
    gsl_matrix* tmp = gsl_matrix_alloc(n, n);
    gsl_matrix_memcpy(tmp, m.matrix_real);
    gsl_permutation* p = gsl_permutation_alloc(n);
    int signum;

    // LU decomposition
    gsl_linalg_LU_decomp(tmp, p, &signum);

    // Check determinant
    double det = gsl_linalg_LU_det(tmp, signum);
    if (det == 0.0) {
      printf("Matrix is singular, cannot invert\n");
      gsl_matrix_free(inv);
      gsl_matrix_free(tmp);
      gsl_permutation_free(p);
      gsl_matrix_free(m.matrix_real); // free popped matrix
      return;
    }

    // Now safe to invert
    if (gsl_linalg_LU_invert(tmp, p, inv) != 0) {
      printf("Matrix inversion failed\n");
      gsl_matrix_free(inv);
      gsl_matrix_free(tmp);
      gsl_permutation_free(p);
      gsl_matrix_free(m.matrix_real);
      return;
    }

    gsl_matrix_free(tmp);
    gsl_permutation_free(p);
    gsl_matrix_free(m.matrix_real);
    push_matrix_real(stack, inv);

  } else if (m.type == TYPE_MATRIX_COMPLEX) {
    size_t n = m.matrix_complex->size1;
    if (n != m.matrix_complex->size2) {
      printf("Complex matrix is not square\n");
      return;
    }

    gsl_matrix_complex* inv = gsl_matrix_complex_alloc(n, n);
    gsl_matrix_complex* tmp = gsl_matrix_complex_alloc(n, n);
    gsl_matrix_complex_memcpy(tmp, m.matrix_complex);
    gsl_permutation* p = gsl_permutation_alloc(n);
    int signum;

    // LU decomposition
    gsl_linalg_complex_LU_decomp(tmp, p, &signum);

    // Check complex determinant
    gsl_complex det = gsl_linalg_complex_LU_det(tmp, signum);
    if (GSL_REAL(det) == 0.0 && GSL_IMAG(det) == 0.0) {
      printf("Complex matrix is singular, cannot invert\n");
      gsl_matrix_complex_free(inv);
      gsl_matrix_complex_free(tmp);
      gsl_permutation_free(p);
      gsl_matrix_complex_free(m.matrix_complex);
      return;
    }

    // Safe to invert
    if (gsl_linalg_complex_LU_invert(tmp, p, inv) != 0) {
      printf("Complex matrix inversion failed\n");
      gsl_matrix_complex_free(inv);
      gsl_matrix_complex_free(tmp);
      gsl_permutation_free(p);
      gsl_matrix_complex_free(m.matrix_complex);
      return;
    }

    gsl_matrix_complex_free(tmp);
    gsl_permutation_free(p);
    gsl_matrix_complex_free(m.matrix_complex);
    push_matrix_complex(stack, inv);

  } else {
    printf("Only real or complex matrix inversion is supported\n");
  }
}


void matrix_determinant(Stack* stack) {
  if (stack->top < 0) {
    printf("No matrix to compute determinant\n");
    return;
  }

  StackElement m = pop(stack);
  if (m.type == TYPE_MATRIX_REAL) {
    size_t n = m.matrix_real->size1;
    if (n != m.matrix_real->size2) {
      printf("Matrix is not square\n");
      return;
    }

    gsl_matrix* tmp = gsl_matrix_alloc(n, n);
    gsl_matrix_memcpy(tmp, m.matrix_real);
    gsl_permutation* p = gsl_permutation_alloc(n);
    int signum;

    if (gsl_linalg_LU_decomp(tmp, p, &signum) != 0) {
      printf("Matrix decomposition failed\n");
      gsl_matrix_free(tmp);
      gsl_permutation_free(p);
      return;
    }

    double det = gsl_linalg_LU_det(tmp, signum);

    gsl_matrix_free(tmp);
    gsl_permutation_free(p);

    push_real(stack, det);

  } else if (m.type == TYPE_MATRIX_COMPLEX) {
    size_t n = m.matrix_complex->size1;
    if (n != m.matrix_complex->size2) {
      printf("Complex matrix is not square\n");
      return;
    }

    gsl_matrix_complex* tmp = gsl_matrix_complex_alloc(n, n);
    gsl_matrix_complex_memcpy(tmp, m.matrix_complex);
    gsl_permutation* p = gsl_permutation_alloc(n);
    int signum;

    if (gsl_linalg_complex_LU_decomp(tmp, p, &signum) != 0) {
      printf("Complex matrix decomposition failed\n");
      gsl_matrix_complex_free(tmp);
      gsl_permutation_free(p);
      return;
    }

    gsl_complex det = gsl_linalg_complex_LU_det(tmp, signum);

    gsl_matrix_complex_free(tmp);
    gsl_permutation_free(p);

    push_complex(stack, to_double_complex(det));

  } else {
    printf("Only real or complex matrix determinant is supported\n");
  }
}


void solve_linear_system(Stack* stack) {
  if (stack->top < 1) {
    printf("Need coefficient matrix and RHS vector\n");
    return;
  }

  StackElement b = pop(stack); // RHS vector (as matrix)
  StackElement a = pop(stack); // Coefficient matrix

  if (a.type == TYPE_MATRIX_REAL && b.type == TYPE_MATRIX_REAL) {
    if (a.matrix_real->size1 != a.matrix_real->size2 ||
	b.matrix_real->size2 != 1 || a.matrix_real->size1 != b.matrix_real->size1) {
      printf("Dimension mismatch or matrix not square\n");
      return;
    }

    size_t n = a.matrix_real->size1;
    gsl_matrix* A = gsl_matrix_alloc(n, n);
    gsl_matrix_memcpy(A, a.matrix_real);
    gsl_vector* B = gsl_vector_alloc(n);
    for (size_t i = 0; i < n; ++i)
      gsl_vector_set(B, i, gsl_matrix_get(b.matrix_real, i, 0));

    gsl_vector* X = gsl_vector_alloc(n);
    gsl_permutation* p = gsl_permutation_alloc(n);
    int signum;

    gsl_linalg_LU_decomp(A, p, &signum);
    gsl_linalg_LU_solve(A, p, B, X);

    gsl_matrix* result = gsl_matrix_alloc(n, 1);
    for (size_t i = 0; i < n; ++i)
      gsl_matrix_set(result, i, 0, gsl_vector_get(X, i));

    push_matrix_real(stack, result);
    gsl_matrix_free(A);
    gsl_vector_free(B);
    gsl_vector_free(X);
    gsl_permutation_free(p);
  } else {
    printf("Unsupported types for linear system solving\n");
  }
}

void matrix_eigen_decompose(Stack* stack) {
  if (stack->top < 0) {
    printf("No matrix on stack for eigendecomposition\n");
    return;
  }

  StackElement m = pop(stack);
  if (m.type == TYPE_MATRIX_REAL) {
    size_t n = m.matrix_real->size1;
    if (n != m.matrix_real->size2) {
      printf("Matrix is not square\n");
      return;
    }

    // Allocate workspace and result containers
    gsl_matrix* tmp = gsl_matrix_alloc(n, n);
    gsl_matrix_memcpy(tmp, m.matrix_real);
    gsl_vector_complex* eval = gsl_vector_complex_alloc(n);
    gsl_matrix_complex* evec = gsl_matrix_complex_alloc(n, n);
    gsl_eigen_nonsymmv_workspace* w = gsl_eigen_nonsymmv_alloc(n);

    if (gsl_eigen_nonsymmv(tmp, eval, evec, w) != 0) {
      printf("Eigen decomposition failed\n");
      gsl_matrix_free(tmp);
      gsl_vector_complex_free(eval);
      gsl_matrix_complex_free(evec);
      gsl_eigen_nonsymmv_free(w);
      return;
    }

    gsl_eigen_nonsymmv_free(w);
    gsl_matrix_free(tmp);

    // Push results to the stack
    push_matrix_complex(stack, evec);

    // Convert eigenvalues (vector) to a diagonal matrix
    gsl_matrix_complex* eval_matrix = gsl_matrix_complex_calloc(n, n);
    for (size_t i = 0; i < n; ++i) {
      gsl_complex z = gsl_vector_complex_get(eval, i);
      gsl_matrix_complex_set(eval_matrix, i, i, z);
    }

    gsl_vector_complex_free(eval);
    push_matrix_complex(stack, eval_matrix);

  } else {
    printf("Only real matrix eigendecomposition is supported\n");
  }
}


void matrix_transpose(Stack* stack) {
  if (stack->top < 0) {
    printf("No matrix to transpose\n");
    return;
  }

  StackElement m = pop(stack);

  if (m.type == TYPE_MATRIX_REAL) {
    size_t rows = m.matrix_real->size1;
    size_t cols = m.matrix_real->size2;

    gsl_matrix* transposed = gsl_matrix_alloc(cols, rows);
    if (!transposed) {
      printf("Memory allocation failed for transposed matrix\n");
      return;
    }

    for (size_t i = 0; i < rows; ++i) {
      for (size_t j = 0; j < cols; ++j) {
	double val = gsl_matrix_get(m.matrix_real, i, j);
	gsl_matrix_set(transposed, j, i, val);
      }
    }

    push_matrix_real(stack, transposed);
    gsl_matrix_free(m.matrix_real);
  }
  else if (m.type == TYPE_MATRIX_COMPLEX) {
    size_t rows = m.matrix_complex->size1;
    size_t cols = m.matrix_complex->size2;

    gsl_matrix_complex* transposed = gsl_matrix_complex_alloc(cols, rows);
    if (!transposed) {
      printf("Memory allocation failed for transposed complex matrix\n");
      return;
    }

    for (size_t i = 0; i < rows; ++i) {
      for (size_t j = 0; j < cols; ++j) {
	gsl_complex val = gsl_matrix_complex_get(m.matrix_complex, i, j);
	gsl_matrix_complex_set(transposed, j, i, val);
      }
    }

    push_matrix_complex(stack, transposed);
    gsl_matrix_complex_free(m.matrix_complex);
  }
  else {
    printf("Only real or complex matrices can be transposed\n");
  }
}


int select_matrix_element(Stack *s) {
  if (s->top < 1) {
    fprintf(stderr, "Error: insufficient elements on stack (need at least two: row, col).\n");
    return -1;  // Indicate error
  }

  // Pop row and column indices
  StackElement *col_elem = &s->items[s->top];
  StackElement *row_elem = &s->items[s->top - 1];
  s->top -= 2;

  if (row_elem->type != TYPE_REAL || col_elem->type != TYPE_REAL) {
    fprintf(stderr, "Error: row and column must be real scalars.\n");
    return -1;  // Indicate error
  }

  // Extract row and column numbers
  size_t row = (size_t)row_elem->real;
  size_t col = (size_t)col_elem->real;

  // Now look at the element on top of the stack (matrix)
  StackElement *matrix_elem = &s->items[s->top];
  if (matrix_elem->type == TYPE_MATRIX_REAL) {
    gsl_matrix *matrix = matrix_elem->matrix_real;
    if (!matrix || row >= matrix->size1 || col >= matrix->size2) {
      fprintf(stderr, "Error: invalid row or column index.\n");
      return -1;  // Indicate error
    }

    // Access the element in the real matrix
    double value = gsl_matrix_get(matrix, row, col);

    // Push the scalar onto the stack
    s->top++;
    StackElement *result_elem = &s->items[s->top];
    result_elem->type = TYPE_REAL;
    result_elem->real = value;
  } else if (matrix_elem->type == TYPE_MATRIX_COMPLEX) {
    gsl_matrix_complex *matrix = matrix_elem->matrix_complex;
    if (!matrix || row >= matrix->size1 || col >= matrix->size2) {
      fprintf(stderr, "Error: invalid row or column index.\n");
      return -1;  // Indicate error
    }

    // Access the element in the complex matrix
    gsl_complex value = gsl_matrix_complex_get(matrix, row, col);

    // Push the complex scalar onto the stack
    s->top++;
    StackElement *result_elem = &s->items[s->top];
    result_elem->type = TYPE_COMPLEX;
    result_elem->complex_val = GSL_REAL(value) + I*GSL_IMAG(value);
  } else {
    fprintf(stderr, "Error: top element is not a matrix.\n");
    return -1;  // Indicate error
  }

  return 0;  // Success
}

int set_matrix_element(Stack *s) {
  if (s->top < 3) {
    fprintf(stderr, "Error: insufficient elements on stack (need value, row, col, and matrix).\n");
    return -1;
  }

  // Top stack layout (high to low):
  StackElement *val_elem = &s->items[s->top - 3];   // [top - 3] -> value (scalar)
  StackElement *col_elem = &s->items[s->top - 2];   // [top - 2] -> col (real scalar)
  StackElement *row_elem = &s->items[s->top - 1];   // [top - 1] -> row (real scalar)
  StackElement *matrix_elem = &s->items[s->top];    // [top - 0] -> matrix (real or complex)

  if (row_elem->type != TYPE_REAL || col_elem->type != TYPE_REAL) {
    fprintf(stderr, "Error: row and column must be real scalars.\n");
    return -1;
  }

  size_t row = (size_t)row_elem->real;
  size_t col = (size_t)col_elem->real;

  if (matrix_elem->type == TYPE_MATRIX_REAL) {
    gsl_matrix *matrix = matrix_elem->matrix_real;
    if (!matrix || row >= matrix->size1 || col >= matrix->size2) {
      fprintf(stderr, "Error: invalid row or column index for real matrix.\n");
      return -1;
    }

    if (val_elem->type != TYPE_REAL) {
      fprintf(stderr, "Error: cannot set real matrix with non-real value.\n");
      return -1;
    }

    printf("rows %zu, cols %zu, element %g\n",row, col, val_elem->real);   
    gsl_matrix_set(matrix, row, col, val_elem->real);

  } else if (matrix_elem->type == TYPE_MATRIX_COMPLEX) {
    gsl_matrix_complex *matrix = matrix_elem->matrix_complex;
    if (!matrix || row >= matrix->size1 || col >= matrix->size2) {
      fprintf(stderr, "Error: invalid row or column index for complex matrix.\n");
      return -1;
    }

    gsl_complex value;
    if (val_elem->type == TYPE_COMPLEX) {
      value = to_gsl_complex(val_elem->complex_val);
    } else if (val_elem->type == TYPE_REAL) {
      GSL_SET_COMPLEX(&value, val_elem->real, 0.0);
    } else {
      fprintf(stderr, "Error: invalid value type for complex matrix.\n");
      return -1;
    }

    gsl_matrix_complex_set(matrix, row, col, value);

  } else {
    fprintf(stderr, "Error: top-3 element is not a matrix.\n");
    return -1;
  }
  return 0;
}

void matrix_extract_diagonal(Stack* stack) {
  if (stack->top < 0) {
    printf("No matrix on stack to extract diagonal from\n");
    return;
  }

  StackElement m = pop(stack);

  if (m.type == TYPE_MATRIX_REAL) {
    size_t n = (m.matrix_real->size1 < m.matrix_real->size2) ? m.matrix_real->size1 : m.matrix_real->size2;

    gsl_matrix* diag = gsl_matrix_calloc(1, n);
    for (size_t i = 0; i < n; ++i) {
      double val = gsl_matrix_get(m.matrix_real, i, i);
      gsl_matrix_set(diag, 0, i, val);
    }

    gsl_matrix_free(m.matrix_real);
    push_matrix_real(stack, diag);

  } else if (m.type == TYPE_MATRIX_COMPLEX) {
    size_t n = (m.matrix_complex->size1 < m.matrix_complex->size2) ? m.matrix_complex->size1 : m.matrix_complex->size2;

    gsl_matrix_complex* diag = gsl_matrix_complex_calloc(1, n);
    for (size_t i = 0; i < n; ++i) {
      gsl_complex z = gsl_matrix_complex_get(m.matrix_complex, i, i);
      gsl_matrix_complex_set(diag, 0, i, z);
    }

    gsl_matrix_complex_free(m.matrix_complex);
    push_matrix_complex(stack, diag);

  } else {
    printf("Only real or complex matrices are supported for diagonal extraction\n");
  }
}

void matrix_cholesky(Stack* stack) {
  if (stack->top < 0) {
    printf("No matrix on stack for Cholesky decomposition\n");
    return;
  }

  StackElement m = pop(stack);

  if (m.type == TYPE_MATRIX_COMPLEX) {
    printf("Cholesky decomposition is only supported for real matrices\n");
    return;
  }

  if (m.type != TYPE_MATRIX_REAL) {
    printf("Only real matrices are supported for Cholesky decomposition\n");
    return;
  }

  size_t n = m.matrix_real->size1;
  if (n != m.matrix_real->size2) {
    printf("Matrix must be square for Cholesky decomposition\n");
    return;
  }

  // Check if the matrix is symmetric
  for (size_t i = 0; i < n; ++i) {
    for (size_t j = i + 1; j < n; ++j) { // Only need to check upper triangle
      double aij = gsl_matrix_get(m.matrix_real, i, j);
      double aji = gsl_matrix_get(m.matrix_real, j, i);
      if (fabs(aij - aji) > 1e-9) { // Tolerance for floating point errors
	printf("Matrix is not symmetric; cannot perform Cholesky decomposition\n");
	return;
      }
    }
  }

  gsl_matrix* tmp = gsl_matrix_alloc(n, n);
  gsl_matrix_memcpy(tmp, m.matrix_real);

  int status = gsl_linalg_cholesky_decomp(tmp);
  if (status != 0) {
    printf("Cholesky decomposition failed (matrix may not be positive definite)\n");
    gsl_matrix_free(tmp);
    return;
  }

  // Zero out the upper triangle to make it clean
  for (size_t i = 0; i < n; ++i) {
    for (size_t j = i + 1; j < n; ++j) {
      gsl_matrix_set(tmp, i, j, 0.0);
    }
  }

  gsl_matrix_free(m.matrix_real); // Free original
  push_matrix_real(stack, tmp);
}

void matrix_svd(Stack* stack) {
  if (stack->top < 0) {
    printf("No matrix on stack for SVD\n");
    return;
  }

  StackElement m = pop(stack);

  if (m.type != TYPE_MATRIX_REAL) {
    printf("SVD is only implemented for real matrices\n");
    return;
  }

  size_t m_rows = m.matrix_real->size1;
  size_t m_cols = m.matrix_real->size2;

  gsl_matrix* A = gsl_matrix_alloc(m_rows, m_cols);
  gsl_matrix_memcpy(A, m.matrix_real);

  size_t min_dim = (m_rows < m_cols) ? m_rows : m_cols;

  gsl_vector* S = gsl_vector_alloc(min_dim);            // Singular values
  gsl_matrix* V = gsl_matrix_alloc(m_cols, m_cols);      // Right singular vectors
  gsl_matrix* U = gsl_matrix_alloc(m_rows, m_rows);      // Left singular vectors
  gsl_vector* work = gsl_vector_alloc(min_dim);          // Workspace

  int status = gsl_linalg_SV_decomp(A, V, S, work);

  if (status != 0) {
    printf("SVD decomposition failed\n");
    gsl_matrix_free(A);
    gsl_matrix_free(V);
    gsl_matrix_free(U);
    gsl_vector_free(S);
    gsl_vector_free(work);
    return;
  }

  gsl_vector_free(work);

  // A now contains U (overwritten)
  gsl_matrix_memcpy(U, A);

  gsl_matrix_free(A);

  // Push results: U, S (as a matrix), V
  push_matrix_real(stack, V); // V goes first
  gsl_matrix* S_mat = gsl_matrix_calloc(m_rows, m_cols);
  for (size_t i = 0; i < min_dim; ++i) {
    gsl_matrix_set(S_mat, i, i, gsl_vector_get(S, i));
  }
  push_matrix_real(stack, S_mat); // Then S (as diagonal matrix)
  push_matrix_real(stack, U);     // Then U

  gsl_vector_free(S);

  // Free original matrix memory
  gsl_matrix_free(m.matrix_real);
}

void make_unit_matrix(Stack* stack) {
  if (stack->top < 0) {
    fprintf(stderr, "Stack underflow: no dimension to create unit matrix.\n");
    return;
  }

  ValueType top_type=stack_top_type(stack);

  if (top_type != TYPE_REAL) {
    fprintf(stderr, "Type error: top stack item must be a real number (dimension).\n");
    return;
  }

  StackElement dim_item = pop(stack);

  int n = (int)(dim_item.real);
  if (n <= 0) {
    fprintf(stderr, "Dimension must be positive, got %d.\n", n);
    return;
  }

  gsl_matrix* m = gsl_matrix_calloc(n, n); // allocates and zeroes the matrix
  if (!m) {
    fprintf(stderr, "Failed to allocate matrix.\n");
    return;
  }

  for (int i = 0; i < n; ++i) {
    gsl_matrix_set(m, i, i, 1.0);  // Set diagonal to 1
  }

  push_matrix_real(stack, m);
}

void make_matrix_of_ones(Stack* stack) {
  if (stack->top < 1) {
    fprintf(stderr, "Stack underflow: need two dimensions to create the matrix.\n");
    return;
  }

  ValueType rows_top_type=stack_top_type(stack);
  ValueType cols_top_type=stack_top_type(stack);
    
  if ((rows_top_type != TYPE_REAL) || (cols_top_type != TYPE_REAL)) {
    fprintf(stderr, "Type error: top stack item must be a real number (dimension).\n");
    return;
  }

  StackElement cols = pop(stack);
  StackElement rows = pop(stack);
    
  int n = (int)(rows.real);
  if (n <= 0) {
    fprintf(stderr, "Dimension must be positive, got %d.\n", n);
    return;
  }

  int m = (int)(cols.real);
  if (n <= 0) {
    fprintf(stderr, "Dimension must be positive, got %d.\n", m);
    return;
  }

  gsl_matrix* mat = gsl_matrix_calloc(n, m); // allocates and zeroes the matrix
  if (!m) {
    fprintf(stderr, "Failed to allocate matrix.\n");
    return;
  }

  gsl_matrix_set_all(mat, 1.0);
    
  push_matrix_real(stack, mat);
}


void make_matrix_of_zeroes(Stack* stack) {
  if (stack->top < 1) {
    fprintf(stderr, "Stack underflow: need two dimensions to create the matrix.\n");
    return;
  }

  ValueType rows_top_type=stack_top_type(stack);
  ValueType cols_top_type=stack_top_type(stack);
    
  if ((rows_top_type != TYPE_REAL) || (cols_top_type != TYPE_REAL)) {
    fprintf(stderr, "Type error: top stack item must be a real number (dimension).\n");
    return;
  }

  StackElement cols = pop(stack);
  StackElement rows = pop(stack);
    
  int n = (int)(rows.real);
  if (n <= 0) {
    fprintf(stderr, "Dimension must be positive, got %d.\n", n);
    return;
  }

  int m = (int)(cols.real);
  if (n <= 0) {
    fprintf(stderr, "Dimension must be positive, got %d.\n", m);
    return;
  }

  gsl_matrix* mat = gsl_matrix_calloc(n, m); // allocates and zeroes the matrix
  if (!m) {
    fprintf(stderr, "Failed to allocate matrix.\n");
    return;
  }
  push_matrix_real(stack, mat);
}

extern gsl_rng* global_rng;  // Assume you initialize this elsewhere

void make_random_matrix(Stack* stack) {
  if (stack->top < 1) {
    fprintf(stderr, "Stack underflow: need two dimensions to create the matrix.\n");
    return;
  }

  ValueType cols_top_type = stack_top_type(stack);
  ValueType rows_top_type = stack_top_type(stack);

  if ((rows_top_type != TYPE_REAL) || (cols_top_type != TYPE_REAL)) {
    fprintf(stderr, "Type error: top stack items must be real numbers (dimensions).\n");
    return;
  }

  StackElement cols = pop(stack);
  StackElement rows = pop(stack);

  int n = (int)(rows.real);
  int m = (int)(cols.real);

  if (n <= 0 || m <= 0) {
    fprintf(stderr, "Dimensions must be positive, got %d x %d.\n", n, m);
    return;
  }

  //    gsl_rng * rng = gsl_rng_alloc(gsl_rng_mt19937);
    
  gsl_matrix* mat = gsl_matrix_alloc(n, m);  // Allocate uninitialized matrix
  if (!mat) {
    fprintf(stderr, "Failed to allocate matrix.\n");
    return;
  }

  for (int i = 0; i < n; ++i) {
    for (int j = 0; j < m; ++j) {
      double u = gsl_rng_uniform(global_rng); // uniform [0,1)
      gsl_matrix_set(mat, i, j, u);
    }
  }

  push_matrix_real(stack, mat);
}

void make_gaussian_random_matrix(Stack* stack) {
  if (stack->top < 1) {
    fprintf(stderr, "Stack underflow: need two dimensions to create the matrix.\n");
    return;
  }

  ValueType cols_top_type = stack_top_type(stack);
  ValueType rows_top_type = stack_top_type(stack);

  if ((rows_top_type != TYPE_REAL) || (cols_top_type != TYPE_REAL)) {
    fprintf(stderr, "Type error: top stack items must be real numbers (dimensions).\n");
    return;
  }

  StackElement cols = pop(stack);
  StackElement rows = pop(stack);

  int n = (int)(rows.real);
  int m = (int)(cols.real);

  if (n <= 0 || m <= 0) {
    fprintf(stderr, "Dimensions must be positive, got %d x %d.\n", n, m);
    return;
  }
  gsl_matrix* mat = gsl_matrix_alloc(n, m);  // Allocate uninitialized matrix
  if (!mat) {
    fprintf(stderr, "Failed to allocate matrix.\n");
    return;
  }

  for (int i = 0; i < n; ++i) {
    for (int j = 0; j < m; ++j) {
      double u = gsl_ran_gaussian(global_rng,1.0); // uniform [0,1)
      gsl_matrix_set(mat, i, j, u);
    }
  }
  push_matrix_real(stack, mat);
}

void matrix_dimensions(Stack* stack) {
    if (stack->top < 0) {
        fprintf(stderr, "Stack underflow: need a matrix to get dimensions.\n");
        return;
    }

    StackElement* top_elem = &stack->items[stack->top];

    size_t rows = 0;
    size_t cols = 0;

    if (top_elem->type == TYPE_MATRIX_REAL) {
        if (!top_elem->matrix_real) {
            fprintf(stderr, "Real matrix pointer is NULL.\n");
            return;
        }
        rows = top_elem->matrix_real->size1;
        cols = top_elem->matrix_real->size2;
    } else if (top_elem->type == TYPE_MATRIX_COMPLEX) {
        if (!top_elem->matrix_complex) {
            fprintf(stderr, "Complex matrix pointer is NULL.\n");
            return;
        }
        rows = top_elem->matrix_complex->size1;
        cols = top_elem->matrix_complex->size2;
    } else {
        fprintf(stderr, "Type error: top stack item is not a matrix.\n");
        return;
    }

    push_real(stack, (double)rows);
    push_real(stack, (double)cols);
}

void reshape_matrix(Stack* stack) {
    if (stack->top < 2) {
        fprintf(stderr, "Stack underflow: need matrix and two dimensions.\n");
        return;
    }

    // Get new cols and rows (top two elements)
    StackElement cols_elem = stack->items[stack->top--];
    StackElement rows_elem = stack->items[stack->top--];

    if (cols_elem.type != TYPE_REAL || rows_elem.type != TYPE_REAL) {
        fprintf(stderr, "Type error: expected real numbers for new dimensions.\n");
        stack->top += 2; // Restore stack
        return;
    }

    int new_rows = (int)rows_elem.real;
    int new_cols = (int)cols_elem.real;

    if (new_rows <= 0 || new_cols <= 0) {
        fprintf(stderr, "Invalid reshape dimensions: must be positive integers.\n");
        return;
    }

    StackElement* mat_elem = &stack->items[stack->top];

    if (mat_elem->type == TYPE_MATRIX_REAL) {
        gsl_matrix* original = mat_elem->matrix_real;
        if (!original) {
            fprintf(stderr, "Null real matrix.\n");
            return;
        }

        size_t original_elements = original->size1 * original->size2;
        if ((size_t)(new_rows * new_cols) != original_elements) {
            fprintf(stderr, "Reshape error: size mismatch (%zu != %d).\n",
                    original_elements, new_rows * new_cols);
            return;
        }

        gsl_matrix* reshaped = gsl_matrix_alloc(new_rows, new_cols);
        if (!reshaped) {
            fprintf(stderr, "Allocation failed for reshaped matrix.\n");
            return;
        }

        // Copy data in row-major order
        size_t k = 0;
        for (size_t i = 0; i < original->size1; ++i) {
            for (size_t j = 0; j < original->size2; ++j) {
                size_t ri = k / new_cols;
                size_t rj = k % new_cols;
                gsl_matrix_set(reshaped, ri, rj, gsl_matrix_get(original, i, j));
                k++;
            }
        }

        // Replace original matrix
        gsl_matrix_free(original);
        mat_elem->matrix_real = reshaped;

    } else if (mat_elem->type == TYPE_MATRIX_COMPLEX) {
        gsl_matrix_complex* original = mat_elem->matrix_complex;
        if (!original) {
            fprintf(stderr, "Null complex matrix.\n");
            return;
        }

        size_t original_elements = original->size1 * original->size2;
        if ((size_t)(new_rows * new_cols) != original_elements) {
            fprintf(stderr, "Reshape error: size mismatch (%zu != %d).\n",
                    original_elements, new_rows * new_cols);
            return;
        }

        gsl_matrix_complex* reshaped = gsl_matrix_complex_alloc(new_rows, new_cols);
        if (!reshaped) {
            fprintf(stderr, "Allocation failed for reshaped complex matrix.\n");
            return;
        }

        size_t k = 0;
        for (size_t i = 0; i < original->size1; ++i) {
            for (size_t j = 0; j < original->size2; ++j) {
                size_t ri = k / new_cols;
                size_t rj = k % new_cols;
                gsl_matrix_complex_set(reshaped, ri, rj, gsl_matrix_complex_get(original, i, j));
                k++;
            }
        }

        gsl_matrix_complex_free(original);
        mat_elem->matrix_complex = reshaped;

    } else {
        fprintf(stderr, "Type error: expected a real or complex matrix below dimensions.\n");
        stack->top += 2; // Restore top
        return;
    }

    // Top of stack now holds reshaped matrix; two items (dims) are popped
}
