#include <time.h>
#include <stdio.h>

void whose_place(void) {
  printf("Your place or mine?\n");
  return;
}

void splash_screen(void) {
  time_t now = time(NULL);
  printf("\n");
  printf("Mico's toy Matrix and Scalar RPN Calculator\n");
  printf("Version alpha 0.1, 2025 \n");
  printf("Started on: %s", ctime(&now));
  printf("Enter expressions in RPN (type 'q' or ctrl-d to quit, 'help' for help).\n");
  printf("\n");
}

void help_menu(void) {
  //  printf("\nMico's toy Matrix and Scalar RPN Calculator\n");
  printf("RPN Calculator for real and complex scalars and matrices\n");
  printf("Rules to enter data types\n");
  printf("    All inputs are case sensitive. Enter strings as \"string\".\n");
  printf("    Enter complex numbers as in: (1,3) or (-1.2e-4, 0.7e2).\n");
  printf("    Enter inline matrices as in J language [#rows #cols $ values].\n");
  printf("    Matrix entries can be real or complex.\n");
  printf("    Read matrix from file as [#rows, #cols, \"filename\"].\n");
  printf("Stack functions\n");
  printf("    drop, dup, swap, clst, [nip, tuck, roll, over]\n");
  printf("Matrix functions\n");
  printf("    Get individual matrix elements with get_aij; set them with set_aij.\n");  
  printf("    Special matrices: eye, ones, rand, randn.\n");  
  printf("    Basic matrix statistics: csum, rsum, cmean, rmean, cvar, rvar\n");  
  printf("    Matrix min and max: cmin, rmin, cmax, rmax\n");  
  printf("Math functions\n");
  printf("    +, -, *, /, ^, [%%, %%chg] \n");
  printf("    Basic probability: npdf, ncdf, nquant [quantiles]\n");
  printf("    Other math functions: ln, exp, log, chs, inv [1/x], ^ [power]\n");
  printf("    Trig functions: sin, cos, tan, asin, acos, atan\n");
  printf("    Hyperbolic functions: sinh, cosh, tanh, asinh, acosh, atanh\n");
  printf("    Polynomials: evaluation and zeros\n");
  printf("    Special functions: gamma, ln_gamma, beta, ln-beta\n");
  printf("Comparison and logic functions\n");
  printf("    eq, leq, lt, gt, geq, neq, and,  or, not\n");
  printf("Complex functions\n");
  printf("    re, im, abs, arg, re2c, split_c, j2r {join 2 reals into complex}\n");
  printf("Constant\n");
  printf("    pi, e, gravity\n");
  printf("Registers functions\n");
  printf("    sto, rcl, pr {print registers}, [save, load] \n");
  printf("String functions\n");
  printf("    concat, s2u [to upper], s2l {to lower}, slen, srev {reverse}\n");
  printf("Financial and date functions\n");
  printf("    pv, npv, irr, ddays, date+, \n");
  printf("\n **** Still to be done in [] brackets ****\n");
  printf("Plotting, 1-D integration, save and load registers,... \n");
  printf("Set print precision; readline keystroke shortcuts.\n");
  printf("\n");
 }

