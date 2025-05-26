#ifndef MATRIX_FUN_H
#define MATRIX_FUN_H

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
#include "math_parsers.h"
#include "math_helpers.h"
#include "binary_fun.h"
#include "unary_fun.h"

int select_matrix_element(Stack *s);
int set_matrix_element(Stack *s);
int matrix_extract_diagonal(Stack* stack);
int make_unit_matrix(Stack* stack);
int make_matrix_of_ones(Stack* stack);
int make_matrix_of_zeroes(Stack* stack);
extern gsl_rng* global_rng;  // Assume you initialize this elsewhere
int make_random_matrix(Stack* stack);
int make_gaussian_random_matrix(Stack* stack);
int matrix_dimensions(Stack* stack);
int reshape_matrix(Stack* stack);
//void matrix_reduce_minmax(Stack* stack, const char* axis, const char* op, bool ignore_nan);

#endif // MATRIX_FUN_H
