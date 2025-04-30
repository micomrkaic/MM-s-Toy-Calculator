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

#ifndef MATH_FUN_H
#define MATH_FUN_H

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
#include "stack.h"
#include "string_fun.h"
#include "binary_fun.h"

void matrix_subtract(Stack* stack);
void matrix_multiply(Stack* stack);
void matrix_inverse(Stack* stack);
void matrix_determinant(Stack* stack);
void solve_linear_system(Stack* stack);
void matrix_eigen_decompose(Stack* stack);
void matrix_transpose(Stack* stack);
gsl_matrix* parse_matrix_literal(const char* input);
gsl_matrix_complex* parse_complex_matrix_literal(const char* input);
int select_matrix_element(Stack *s);
int set_matrix_element(Stack *s);
void matrix_extract_diagonal(Stack* stack);
void matrix_cholesky(Stack* stack);
void matrix_svd(Stack* stack);
void make_unit_matrix(Stack* stack);
void make_matrix_of_ones(Stack* stack);
void make_matrix_of_zeroes(Stack* stack);
void make_random_matrix(Stack* stack);
void make_gaussian_random_matrix(Stack* stack);
void matrix_dimensions(Stack* stack);
void reshape_matrix(Stack* stack);
#endif // MATH_FUN_H
