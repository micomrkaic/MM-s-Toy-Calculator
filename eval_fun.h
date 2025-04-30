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

#ifndef EVAL_FUN_H
#define EVAL_FUN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <complex.h>
#include <time.h>
#include <readline/history.h>
#include <readline/readline.h>
#include "lexer.h"
#include "stack.h"
#include "string_fun.h"
#include "math_fun.h"
#include "binary_fun.h"
#include "unary_fun.h"
#include "eval_fun.h"

int read_complex(const char* input, double complex* z);
void help_menu();
int evaluator(Stack *stack, char * line);

#endif // EVAL_FUN_H
