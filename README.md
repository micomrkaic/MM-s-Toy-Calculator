# Mico's toy Matrix and Scalar RPN Calculator
RPN Calculator for real and complex scalars and matrices.
Usage:
All inputs are case sensitive.
Enter complex numbers as in: (1,3) or (-1.2e-4, 0.7e2)
Enter matrices as [#rows #cols $ matrix_elements]. Entries can be complex; #rows and #cols are integers.
Read matrix from file as [#rows, #cols, "filename"] where #cols and #rows are integers and "filename" is the name of the file (must be in quotes).
Statistics (colmean, rowmean,...); regression; normpdf, normcdf, rand, nrand
Trig functions: sin, cos, tan, asin, acos, atan
Other math functions: ln, exp, log, ^
String functions: concat, s2u [to upper], s2l [to lower], slen, srev [reverse]
Stack functions: drop, dup, swap, clst
Constants: pi, e, gravity

# RPN Calculator

A stack-based Reverse Polish Notation (RPN) calculator written in C. Supports real and complex numbers, strings, and matrices (real and complex).

## Features

- ✅ Real and complex number support
- ✅ String operations
- ✅ Real and complex matrices using [GSL](https://www.gnu.org/software/gsl/)
- ✅ Stack-based computation with visitor traversal
- ✅ User-defined functions and variables
- ✅ Scientific functions: trigonometric, exponential, logarithmic
- ✅ Matrix algebra: addition, multiplication, inversion, division
- ✅ Matrix statistics: means, sums, and variances by rows or columns
- ✅ Special matrices: identity, constant, random, Gaussian random
- ✅ GNU Readline support for command history and editing
- ✅ Registers for storage
- ✅ Normal pdf, cdf, quantiles
- ✅ Polynomials: zeros and evaluations

## Build Instructions

bash
- git clone https://github.com/yourusername/rpn-calculator.git
- cd rpn-calculator
- make

## Requirements
- C compiler (I use GCC with C17 standard, but CLANG should work with a little bit of effort)
- GNU make
- GNU readline (libreadline-dev)
- Gnu Scientific Library (libgsl-dev)

## Would be nice to add
- User-defined functions
- Numerical integration
- Estimation: OLS, GLS, GMM, ML, etc.
