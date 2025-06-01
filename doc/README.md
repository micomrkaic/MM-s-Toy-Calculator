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
- C compiler (I use GCC with C11 standard, but CLANG should work without issues)
- GNU make
- GNU readline (libreadline-dev)
- Gnu Scientific Library (libgsl-dev)

## Would be nice to add
- Numerical integration
- Estimation: OLS, GLS, GMM, ML, etc.

# Annex I: Full Function List
## Mathematics
### Basic functionality
- .*
- ./
- .^
- ln
- log
- exp
- sqrt
- pow
- frac
- intg
- chs
- inv
### Complex numbers
- re
- im
- abs
- arg
- conj
- re2c
- split_c
- j2r
### Trigonometry
- sin
- cos
- tan
- asin
- acos
- atan
### Hyperbolic functions
- sinh
- cosh
- tanh
- asinh
- acosh
- atanh
## Matrices and linear algebra
- minv
- pinv
- det
- eig
- tran
- reshape
- get_aij
- set_aij
- split_mat
- kron
- diag
- to_diag
- chol
- svd
- dim
- eye
- join_v
- join_h
- cumsum_r
- cumsum_c
- ones
- zeroes
- rand
- randn
- rrange
- cmean
- rmean
- csum
- rsum
- cvar
- rvar
- cmin
- cmax
- rmin
- rmax
## Polynomials
- roots
- pval
### Special functions
- npdf
- ncdf
- nquant
- gamma
- ln_gamma
- beta
- ln_beta
## Constants
- gravity
- pi
- e
- inf
- nan
## Stack operations
- drop
- clst
- swap
- dup
- nip
- tuck
- roll
- over
## String functions
- scon
- s2l
- s2u
- slen
- srev
- int2str
## Register operations
- rcl
- sto
- pr
- saveregs
- loadregs
- clregs
- ffr
## Printing and format control
- print
- pm
- ps
- setprec
- sfs
## Tests and logic
- eq
- leq
- lt
- gt
- geq
- neq
- and
- or
- not
# Date functions
- ddays
- today
- dateplus
- dow
- edmy
## User defined words and predefined macros
- listwords
- loadwords
- savewords
- delword
- selword
- clrwords
- listmacros
## Programming
- top_eq0?
- top_ge0?
- top_gt0?
- top_le0?
- top_lt0?
- top_eg?
- top_ge?
- top_gt?
- top_le?
- top_lt?
- ctr_eq0?
- ctr_ge0?
- ctr_gt0?
- ctr_le0?
- ctr_lt0?
- set_ctr
- clr_ctr
- ctr_inc
- ctr_dec
- goto
- xeq
- rtn
- end
- lbl
- eval
- batch
- run
## Misc
- fuck
- help
- listfcns
- undo
- clrhist
