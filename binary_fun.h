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

#ifndef BINARY_FUN_H
#define BINARY_FUN_H

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
#include "math_fun.h"
#include "unary_fun.h"

void add_top_two_scalars(Stack* stack);
void add_top_two_matrices(Stack* stack);
void multiply_top_two_scalars(Stack* stack);
void subtract_top_two_scalars(Stack* stack);
void divide_top_two_scalars(Stack* stack);
void subtract_top_two_matrices(Stack* stack);
void multiply_top_two_matrices(Stack* stack);
void add_top_two(Stack* stack);
void sub_top_two(Stack* stack);
void mul_top_two(Stack* stack);
void div_top_two(Stack* stack);
void pow_top_two(Stack* stack);
void join_2_reals(Stack *s);
void kronecker_top_two(Stack* stack);

#endif //BINARY_FUN_H

  
