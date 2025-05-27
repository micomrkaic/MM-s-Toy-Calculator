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

#include "globals.h"

int print_precision = 6;
bool fixed_point = true;
int selected_function = 0;

#include <stdio.h>
#include <complex.h>

int set_print_precision(Stack* stack) {
    if (stack->top < 0) {
        fprintf(stderr, "Error: Stack underflow. Expected number for precision\n");
        return 1;
    }

    StackElement elem = stack->items[stack->top--];

    int precision;

    if (elem.type == TYPE_REAL) {
        precision = (int)elem.real;
    } else if (elem.type == TYPE_COMPLEX) {
        if (cimag(elem.complex_val) != 0.0) {
            fprintf(stderr, "Error: Complex value must be real to set precision\n");
            return 1;
        }
        precision = (int)creal(elem.complex_val);
    } else {
        fprintf(stderr, "Error: Expected number on stack to set print precision\n");
        return 1;
    }

    if (precision < 0 || precision > 20) {
        fprintf(stderr, "Error: Precision value %d is out of valid range (0–20)\n", precision);
        return 1;
    }

    print_precision = precision;
    return 0;
}



void swap_fixed_scientific(void) {
  fixed_point = !fixed_point;
}

/*
  last_x, last_y
  pointers to programs in memory

  loop counters: 8x3 sets of numbers for DSE/ISG style looping

*/
